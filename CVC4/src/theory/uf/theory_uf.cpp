/*********************                                                        */
/*! \file theory_uf.cpp
 ** \verbatim
 ** Top contributors (to current version):
 **   Dejan Jovanovic, Morgan Deters, Andrew Reynolds
 ** This file is part of the CVC4 project.
 ** Copyright (c) 2009-2016 by the authors listed in the file AUTHORS
 ** in the top-level source directory) and their institutional affiliations.
 ** All rights reserved.  See the file COPYING in the top-level source
 ** directory for licensing information.\endverbatim
 **
 ** \brief This is the interface to TheoryUF implementations
 **
 ** This is the interface to TheoryUF implementations.  All
 ** implementations of TheoryUF should inherit from this class.
 **/

#include "theory/uf/theory_uf.h"

#include "options/quantifiers_options.h"
#include "options/smt_options.h"
#include "options/uf_options.h"
#include "proof/proof_manager.h"
#include "proof/theory_proof.h"
#include "proof/uf_proof.h"
#include "theory/theory_model.h"
#include "theory/type_enumerator.h"
#include "theory/uf/theory_uf_strong_solver.h"
#include "theory/quantifiers/term_database.h"
#include "options/theory_options.h"

using namespace std;

namespace CVC4 {
namespace theory {
namespace uf {

/** Constructs a new instance of TheoryUF w.r.t. the provided context.*/
TheoryUF::TheoryUF(context::Context* c, context::UserContext* u,
                   OutputChannel& out, Valuation valuation,
                   const LogicInfo& logicInfo, std::string instanceName)
    : Theory(THEORY_UF, c, u, out, valuation, logicInfo, instanceName),
      d_notify(*this),
      /* The strong theory solver can be notified by EqualityEngine::init(),
       * so make sure it's initialized first. */
      d_thss(NULL),
      d_equalityEngine(d_notify, c, instanceName + "theory::uf::TheoryUF",
                       true),
      d_conflict(c, false),
      d_literalsToPropagate(c),
      d_literalsToPropagateIndex(c, 0),
      d_functionsTerms(c),
      d_symb(u, instanceName)
{
  // The kinds we are treating as function application in congruence
  d_equalityEngine.addFunctionKind(kind::APPLY_UF);
}

TheoryUF::~TheoryUF() {
  delete d_thss;
}

void TheoryUF::setMasterEqualityEngine(eq::EqualityEngine* eq) {
  d_equalityEngine.setMasterEqualityEngine(eq);
}

void TheoryUF::finishInit() {
  // initialize the strong solver
  if (options::finiteModelFind() && options::ufssMode()!=UF_SS_NONE) {
    d_thss = new StrongSolverTheoryUF(getSatContext(), getUserContext(), *d_out, this);
  }
}

static Node mkAnd(const std::vector<TNode>& conjunctions) {
  Assert(conjunctions.size() > 0);

  std::set<TNode> all;
  all.insert(conjunctions.begin(), conjunctions.end());

  if (all.size() == 1) {
    // All the same, or just one
    return conjunctions[0];
  }

  NodeBuilder<> conjunction(kind::AND);
  std::set<TNode>::const_iterator it = all.begin();
  std::set<TNode>::const_iterator it_end = all.end();
  while (it != it_end) {
    conjunction << *it;
    ++ it;
  }

  return conjunction;
}/* mkAnd() */

void TheoryUF::check(Effort level) {
  if (done() && !fullEffort(level)) {
    return;
  }
  getOutputChannel().spendResource(options::theoryCheckStep());
  TimerStat::CodeTimer checkTimer(d_checkTime);

  while (!done() && !d_conflict)
  {
    // Get all the assertions
    Assertion assertion = get();
    TNode fact = assertion.assertion;

    Debug("uf") << "TheoryUF::check(): processing " << fact << std::endl;
    Debug("uf") << "Term's theory: " << theory::Theory::theoryOf(fact.toExpr()) << std::endl;

    if (d_thss != NULL) {
      bool isDecision = d_valuation.isSatLiteral(fact) && d_valuation.isDecision(fact);
      d_thss->assertNode(fact, isDecision);
      if( d_thss->isConflict() ){
        d_conflict = true;
        return;
      }
    }

    // Do the work
    bool polarity = fact.getKind() != kind::NOT;
    TNode atom = polarity ? fact : fact[0];
    if (atom.getKind() == kind::EQUAL) {
      d_equalityEngine.assertEquality(atom, polarity, fact);
    } else if (atom.getKind() == kind::CARDINALITY_CONSTRAINT || atom.getKind() == kind::COMBINED_CARDINALITY_CONSTRAINT) {
      if( d_thss == NULL ){
        std::stringstream ss;
        ss << "Cardinality constraint " << atom << " was asserted, but the logic does not allow it." << std::endl;
        ss << "Try using a logic containing \"UFC\"." << std::endl;
        throw Exception( ss.str() );
      }
      //needed for models
      if( options::produceModels() && ( atom.getKind() == kind::COMBINED_CARDINALITY_CONSTRAINT || options::ufssMode()!=UF_SS_FULL ) ){
        d_equalityEngine.assertPredicate(atom, polarity, fact);
      }
    } else {
      d_equalityEngine.assertPredicate(atom, polarity, fact);
    }
  }

  if(! d_conflict ){
    if (d_thss != NULL) {
      d_thss->check(level);
      if( d_thss->isConflict() ){
        d_conflict = true;
      }
    }
  }
}/* TheoryUF::check() */

void TheoryUF::preRegisterTerm(TNode node) {
  Debug("uf") << "TheoryUF::preRegisterTerm(" << node << ")" << std::endl;

  if (d_thss != NULL) {
    d_thss->preRegisterTerm(node);
  }

  switch (node.getKind()) {
  case kind::EQUAL:
    // Add the trigger for equality
    d_equalityEngine.addTriggerEquality(node);
    break;
  case kind::APPLY_UF:
    // Maybe it's a predicate
    if (node.getType().isBoolean()) {
      // Get triggered for both equal and dis-equal
      d_equalityEngine.addTriggerPredicate(node);
    } else {
      // Function applications/predicates
      d_equalityEngine.addTerm(node);
    }
    // Remember the function and predicate terms
    d_functionsTerms.push_back(node);
    break;
  case kind::CARDINALITY_CONSTRAINT:
  case kind::COMBINED_CARDINALITY_CONSTRAINT:
    //do nothing
    break;
  default:
    // Variables etc
    d_equalityEngine.addTerm(node);
    break;
  }
}/* TheoryUF::preRegisterTerm() */

bool TheoryUF::propagate(TNode literal) {
  Debug("uf::propagate") << "TheoryUF::propagate(" << literal  << ")" << std::endl;
  // If already in conflict, no more propagation
  if (d_conflict) {
    Debug("uf::propagate") << "TheoryUF::propagate(" << literal << "): already in conflict" << std::endl;
    return false;
  }
  // Propagate out
  bool ok = d_out->propagate(literal);
  if (!ok) {
    d_conflict = true;
  }
  return ok;
}/* TheoryUF::propagate(TNode) */

void TheoryUF::propagate(Effort effort) {
  //if (d_thss != NULL) {
  //  return d_thss->propagate(effort);
  //}
}

Node TheoryUF::getNextDecisionRequest( unsigned& priority ){
  if (d_thss != NULL && !d_conflict) {
    return d_thss->getNextDecisionRequest( priority );
  }else{
    return Node::null();
  }
}

void TheoryUF::explain(TNode literal, std::vector<TNode>& assumptions, eq::EqProof* pf) {
  // Do the work
  bool polarity = literal.getKind() != kind::NOT;
  TNode atom = polarity ? literal : literal[0];
  if (atom.getKind() == kind::EQUAL) {
    d_equalityEngine.explainEquality(atom[0], atom[1], polarity, assumptions, pf);
  } else {
    d_equalityEngine.explainPredicate(atom, polarity, assumptions, pf);
  }
  if( pf ){
    Debug("pf::uf") << std::endl;
    pf->debug_print("pf::uf");
  }

  Debug("pf::uf") << "UF: explain( " << literal << " ):" << std::endl << "\t";
  for (unsigned i = 0; i < assumptions.size(); ++i) {
    Debug("pf::uf") << assumptions[i] << " ";
  }
  Debug("pf::uf") << std::endl;
}

Node TheoryUF::explain(TNode literal) {
  return explain(literal, NULL);
}

Node TheoryUF::explain(TNode literal, eq::EqProof* pf) {
  Debug("uf") << "TheoryUF::explain(" << literal << ")" << std::endl;
  std::vector<TNode> assumptions;
  explain(literal, assumptions, pf);
  return mkAnd(assumptions);
}

void TheoryUF::collectModelInfo( TheoryModel* m ){
  set<Node> termSet;

  // Compute terms appearing in assertions and shared terms
  computeRelevantTerms(termSet);

  m->assertEqualityEngine( &d_equalityEngine, &termSet );
  // if( fullModel ){
  //   std::map< TypeNode, TypeEnumerator* > type_enums;
  //   //must choose proper representatives
  //   // for each equivalence class, specify fresh constant as representative
  //   eq::EqClassesIterator eqcs_i = eq::EqClassesIterator( &d_equalityEngine );
  //   while( !eqcs_i.isFinished() ){
  //     Node eqc = (*eqcs_i);
  //     TypeNode tn = eqc.getType();
  //     if( tn.isSort() ){
  //       if( type_enums.find( tn )==type_enums.end() ){
  //         type_enums[tn] = new TypeEnumerator( tn );
  //       }
  //       Node rep = *(*type_enums[tn]);
  //       ++(*type_enums[tn]);
  //       //specify the constant as the representative
  //       m->assertEquality( eqc, rep, true );
  //       m->assertRepresentative( rep );
  //     }
  //     ++eqcs_i;
  //   }
  // }
}

void TheoryUF::presolve() {
  // TimerStat::CodeTimer codeTimer(d_presolveTimer);

  Debug("uf") << "uf: begin presolve()" << endl;
  if(options::ufSymmetryBreaker()) {
    vector<Node> newClauses;
    d_symb.apply(newClauses);
    for(vector<Node>::const_iterator i = newClauses.begin();
        i != newClauses.end();
        ++i) {
      Debug("uf") << "uf: generating a lemma: " << *i << std::endl;
      d_out->lemma(*i);
    }
  }
  if( d_thss ){
    d_thss->presolve();
  }
  Debug("uf") << "uf: end presolve()" << endl;
}

void TheoryUF::ppStaticLearn(TNode n, NodeBuilder<>& learned) {
  //TimerStat::CodeTimer codeTimer(d_staticLearningTimer);

  vector<TNode> workList;
  workList.push_back(n);
  __gnu_cxx::hash_set<TNode, TNodeHashFunction> processed;

  while(!workList.empty()) {
    n = workList.back();

    if(n.getKind() == kind::FORALL || n.getKind() == kind::EXISTS) {
      // unsafe to go under quantifiers; we might pull bound vars out of scope!
      processed.insert(n);
      workList.pop_back();
      continue;
    }

    bool unprocessedChildren = false;
    for(TNode::iterator i = n.begin(), iend = n.end(); i != iend; ++i) {
      if(processed.find(*i) == processed.end()) {
        // unprocessed child
        workList.push_back(*i);
        unprocessedChildren = true;
      }
    }

    if(unprocessedChildren) {
      continue;
    }

    workList.pop_back();
    // has node n been processed in the meantime ?
    if(processed.find(n) != processed.end()) {
      continue;
    }
    processed.insert(n);

    // == DIAMONDS ==

    Debug("diamonds") << "===================== looking at" << endl
                      << n << endl;

    // binary OR of binary ANDs of EQUALities
    if(n.getKind() == kind::OR && n.getNumChildren() == 2 &&
       n[0].getKind() == kind::AND && n[0].getNumChildren() == 2 &&
       n[1].getKind() == kind::AND && n[1].getNumChildren() == 2 &&
       (n[0][0].getKind() == kind::EQUAL) &&
       (n[0][1].getKind() == kind::EQUAL) &&
       (n[1][0].getKind() == kind::EQUAL) &&
       (n[1][1].getKind() == kind::EQUAL)) {
      // now we have (a = b && c = d) || (e = f && g = h)

      Debug("diamonds") << "has form of a diamond!" << endl;

      TNode
        a = n[0][0][0], b = n[0][0][1],
        c = n[0][1][0], d = n[0][1][1],
        e = n[1][0][0], f = n[1][0][1],
        g = n[1][1][0], h = n[1][1][1];

      // test that one of {a, b} = one of {c, d}, and make "b" the
      // shared node (i.e. put in the form (a = b && b = d))
      // note we don't actually care about the shared ones, so the
      // "swaps" below are one-sided, ignoring b and c
      if(a == c) {
        a = b;
      } else if(a == d) {
        a = b;
        d = c;
      } else if(b == c) {
        // nothing to do
      } else if(b == d) {
        d = c;
      } else {
        // condition not satisfied
        Debug("diamonds") << "+ A fails" << endl;
        continue;
      }

      Debug("diamonds") << "+ A holds" << endl;

      // same: one of {e, f} = one of {g, h}, and make "f" the
      // shared node (i.e. put in the form (e = f && f = h))
      if(e == g) {
        e = f;
      } else if(e == h) {
        e = f;
        h = g;
      } else if(f == g) {
        // nothing to do
      } else if(f == h) {
        h = g;
      } else {
        // condition not satisfied
        Debug("diamonds") << "+ B fails" << endl;
        continue;
      }

      Debug("diamonds") << "+ B holds" << endl;

      // now we have (a = b && b = d) || (e = f && f = h)
      // test that {a, d} == {e, h}
      if( (a == e && d == h) ||
          (a == h && d == e) ) {
        // learn: n implies a == d
        Debug("diamonds") << "+ C holds" << endl;
        Node newEquality = a.eqNode(d);
        Debug("diamonds") << "  ==> " << newEquality << endl;
        learned << n.impNode(newEquality);
      } else {
        Debug("diamonds") << "+ C fails" << endl;
      }
    }
  }

  if(options::ufSymmetryBreaker()) {
    d_symb.assertFormula(n);
  }
}/* TheoryUF::ppStaticLearn() */

EqualityStatus TheoryUF::getEqualityStatus(TNode a, TNode b) {

  // Check for equality (simplest)
  if (d_equalityEngine.areEqual(a, b)) {
    // The terms are implied to be equal
    return EQUALITY_TRUE;
  }

  // Check for disequality
  if (d_equalityEngine.areDisequal(a, b, false)) {
    // The terms are implied to be dis-equal
    return EQUALITY_FALSE;
  }

  // All other terms we interpret as dis-equal in the model
  return EQUALITY_FALSE_IN_MODEL;
}

void TheoryUF::addSharedTerm(TNode t) {
  Debug("uf::sharing") << "TheoryUF::addSharedTerm(" << t << ")" << std::endl;
  d_equalityEngine.addTriggerTerm(t, THEORY_UF);
}

bool TheoryUF::areCareDisequal(TNode x, TNode y){
  Assert( d_equalityEngine.hasTerm(x) );
  Assert( d_equalityEngine.hasTerm(y) );
  if( d_equalityEngine.isTriggerTerm(x, THEORY_UF) && d_equalityEngine.isTriggerTerm(y, THEORY_UF) ){
    TNode x_shared = d_equalityEngine.getTriggerTermRepresentative(x, THEORY_UF);
    TNode y_shared = d_equalityEngine.getTriggerTermRepresentative(y, THEORY_UF);
    EqualityStatus eqStatus = d_valuation.getEqualityStatus(x_shared, y_shared);
    if( eqStatus==EQUALITY_FALSE_AND_PROPAGATED || eqStatus==EQUALITY_FALSE || eqStatus==EQUALITY_FALSE_IN_MODEL ){
      return true;
    }
  }
  return false;
}

//TODO: move quantifiers::TermArgTrie to src/theory/
void TheoryUF::addCarePairs( quantifiers::TermArgTrie * t1, quantifiers::TermArgTrie * t2, unsigned arity, unsigned depth ){
  if( depth==arity ){
    if( t2!=NULL ){
      Node f1 = t1->getNodeData();
      Node f2 = t2->getNodeData();
      if( !d_equalityEngine.areEqual( f1, f2 ) ){
        Debug("uf::sharing") << "TheoryUf::computeCareGraph(): checking function " << f1 << " and " << f2 << std::endl;
        vector< pair<TNode, TNode> > currentPairs;
        for (unsigned k = 0; k < f1.getNumChildren(); ++ k) {
          TNode x = f1[k];
          TNode y = f2[k];
          Assert( d_equalityEngine.hasTerm(x) );
          Assert( d_equalityEngine.hasTerm(y) );
          Assert( !d_equalityEngine.areDisequal( x, y, false ) );
          Assert( !areCareDisequal( x, y ) );
          if( !d_equalityEngine.areEqual( x, y ) ){
            if( d_equalityEngine.isTriggerTerm(x, THEORY_UF) && d_equalityEngine.isTriggerTerm(y, THEORY_UF) ){
              TNode x_shared = d_equalityEngine.getTriggerTermRepresentative(x, THEORY_UF);
              TNode y_shared = d_equalityEngine.getTriggerTermRepresentative(y, THEORY_UF);
              currentPairs.push_back(make_pair(x_shared, y_shared));
            }
          }
        }
        for (unsigned c = 0; c < currentPairs.size(); ++ c) {
          Debug("uf::sharing") << "TheoryUf::computeCareGraph(): adding to care-graph" << std::endl;
          addCarePair(currentPairs[c].first, currentPairs[c].second);
        }
      }
    }
  }else{
    if( t2==NULL ){
      if( depth<(arity-1) ){
        //add care pairs internal to each child
        for( std::map< TNode, quantifiers::TermArgTrie >::iterator it = t1->d_data.begin(); it != t1->d_data.end(); ++it ){
          addCarePairs( &it->second, NULL, arity, depth+1 );
        }
      }
      //add care pairs based on each pair of non-disequal arguments
      for( std::map< TNode, quantifiers::TermArgTrie >::iterator it = t1->d_data.begin(); it != t1->d_data.end(); ++it ){
        std::map< TNode, quantifiers::TermArgTrie >::iterator it2 = it;
        ++it2;
        for( ; it2 != t1->d_data.end(); ++it2 ){
          if( !d_equalityEngine.areDisequal(it->first, it2->first, false) ){
            if( !areCareDisequal(it->first, it2->first) ){
              addCarePairs( &it->second, &it2->second, arity, depth+1 );
            }
          }
        }
      }
    }else{
      //add care pairs based on product of indices, non-disequal arguments
      for( std::map< TNode, quantifiers::TermArgTrie >::iterator it = t1->d_data.begin(); it != t1->d_data.end(); ++it ){
        for( std::map< TNode, quantifiers::TermArgTrie >::iterator it2 = t2->d_data.begin(); it2 != t2->d_data.end(); ++it2 ){
          if( !d_equalityEngine.areDisequal(it->first, it2->first, false) ){
            if( !areCareDisequal(it->first, it2->first) ){
              addCarePairs( &it->second, &it2->second, arity, depth+1 );
            }
          }
        }
      }
    }
  }
}

void TheoryUF::computeCareGraph() {

  if (d_sharedTerms.size() > 0) {
    //use term indexing
    Debug("uf::sharing") << "TheoryUf::computeCareGraph(): Build term indices..." << std::endl;
    std::map< Node, quantifiers::TermArgTrie > index;
    std::map< Node, unsigned > arity;
    unsigned functionTerms = d_functionsTerms.size();
    for (unsigned i = 0; i < functionTerms; ++ i) {
      TNode f1 = d_functionsTerms[i];
      Node op = f1.getOperator();
      std::vector< TNode > reps;
      bool has_trigger_arg = false;
      for( unsigned j=0; j<f1.getNumChildren(); j++ ){
        reps.push_back( d_equalityEngine.getRepresentative( f1[j] ) );
        if( d_equalityEngine.isTriggerTerm( f1[j], THEORY_UF ) ){
          has_trigger_arg = true;
        }
      }
      if( has_trigger_arg ){
        index[op].addTerm( f1, reps );
        arity[op] = reps.size();
      }
    }
    //for each index
    for( std::map< Node, quantifiers::TermArgTrie >::iterator itii = index.begin(); itii != index.end(); ++itii ){
      Debug("uf::sharing") << "TheoryUf::computeCareGraph(): Process index " << itii->first << "..." << std::endl;
      addCarePairs( &itii->second, NULL, arity[ itii->first ], 0 );
    }
  }
}/* TheoryUF::computeCareGraph() */

void TheoryUF::conflict(TNode a, TNode b) {
  eq::EqProof* pf = d_proofsEnabled ? new eq::EqProof() : NULL;
  d_conflictNode = explain(a.eqNode(b),pf);
  ProofUF* puf = d_proofsEnabled ? new ProofUF( pf ) : NULL;
  d_out->conflict(d_conflictNode, puf);
  d_conflict = true;
}

void TheoryUF::eqNotifyNewClass(TNode t) {
  if (d_thss != NULL) {
    d_thss->newEqClass(t);
  }
}

void TheoryUF::eqNotifyPreMerge(TNode t1, TNode t2) {
  //if (getLogicInfo().isQuantified()) {
    //getQuantifiersEngine()->getEfficientEMatcher()->merge( t1, t2 );
  //}
}

void TheoryUF::eqNotifyPostMerge(TNode t1, TNode t2) {
  if (d_thss != NULL) {
    d_thss->merge(t1, t2);
  }
}

void TheoryUF::eqNotifyDisequal(TNode t1, TNode t2, TNode reason) {
  if (d_thss != NULL) {
    d_thss->assertDisequal(t1, t2, reason);
  }
}


} /* namespace CVC4::theory::uf */
} /* namespace CVC4::theory */
} /* namespace CVC4 */
