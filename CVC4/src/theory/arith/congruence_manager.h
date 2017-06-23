/*********************                                                        */
/*! \file congruence_manager.h
 ** \verbatim
 ** Top contributors (to current version):
 **   Tim King, Morgan Deters, Dejan Jovanovic, Andrew Reynolds
 ** This file is part of the CVC4 project.
 ** Copyright (c) 2009-2016 by the authors listed in the file AUTHORS
 ** in the top-level source directory) and their institutional affiliations.
 ** All rights reserved.  See the file COPYING in the top-level source
 ** directory for licensing information.\endverbatim
 **
 ** \brief [[ Add one-line brief description here ]]
 **
 ** [[ Add lengthier description here ]]
 ** \todo document this file
 **/

#include "cvc4_private.h"

#pragma once

#include "context/cdlist.h"
#include "context/cdmaybe.h"
#include "context/cdo.h"
#include "context/cdtrail_queue.h"
#include "context/context.h"
#include "theory/arith/arithvar.h"
#include "theory/arith/constraint_forward.h"
#include "theory/arith/partial_model.h"
#include "theory/uf/equality_engine.h"
#include "util/dense_map.h"
#include "util/statistics_registry.h"

namespace CVC4 {
namespace theory {

namespace quantifiers {
class EqualityInference;
}

namespace arith {

class ArithCongruenceManager {
private:
  context::CDRaised d_inConflict;
  RaiseEqualityEngineConflict d_raiseConflict;

  /**
   * The set of ArithVars equivalent to a pair of terms.
   * If this is 0 or cannot be 0, this can be signalled.
   * The pair of terms for the congruence is stored in watched equalities.
   */
  DenseSet d_watchedVariables;
  /** d_watchedVariables |-> (= x y) */
  ArithVarToNodeMap d_watchedEqualities;


  class ArithCongruenceNotify : public eq::EqualityEngineNotify {
  private:
    ArithCongruenceManager& d_acm;
  public:
    ArithCongruenceNotify(ArithCongruenceManager& acm);

    bool eqNotifyTriggerEquality(TNode equality, bool value);

    bool eqNotifyTriggerPredicate(TNode predicate, bool value);

    bool eqNotifyTriggerTermEquality(TheoryId tag, TNode t1, TNode t2, bool value);

    void eqNotifyConstantTermMerge(TNode t1, TNode t2);
    void eqNotifyNewClass(TNode t);
    void eqNotifyPreMerge(TNode t1, TNode t2);
    void eqNotifyPostMerge(TNode t1, TNode t2);
    void eqNotifyDisequal(TNode t1, TNode t2, TNode reason);
  };
  ArithCongruenceNotify d_notify;
  
  /** module for shostak normalization, d_eqi_counter is how many pending merges in d_eq_infer we have processed */
  quantifiers::EqualityInference * d_eq_infer;
  context::CDO< unsigned > d_eqi_counter;
  Node d_true;

  context::CDList<Node> d_keepAlive;

  /** Store the propagations. */
  context::CDTrailQueue<Node> d_propagatations;

  /* This maps the node a theory engine will request on an explain call to
   * to its corresponding PropUnit.
   * This is node is potentially both the propagation or
   * Rewriter::rewrite(propagation).
   */
  typedef context::CDHashMap<Node, size_t, NodeHashFunction> ExplainMap;
  ExplainMap d_explanationMap;

  ConstraintDatabase& d_constraintDatabase;
  SetupLiteralCallBack d_setupLiteral;

  const ArithVariables& d_avariables;

  eq::EqualityEngine d_ee;

  void raiseConflict(Node conflict);
public:

  bool inConflict() const;

  bool hasMorePropagations() const;

  const Node getNextPropagation();

  bool canExplain(TNode n) const;

  void setMasterEqualityEngine(eq::EqualityEngine* eq);

private:
  Node externalToInternal(TNode n) const;

  void pushBack(TNode n);

  void pushBack(TNode n, TNode r);

  void pushBack(TNode n, TNode r, TNode w);

  bool propagate(TNode x);
  void explain(TNode literal, std::vector<TNode>& assumptions);


  /** This sends a shared term to the uninterpreted equality engine. */
  void assertionToEqualityEngine(bool eq, ArithVar s, TNode reason);

  /** Dequeues the delay queue and asserts these equalities.*/
  void enableSharedTerms();
  void dequeueLiterals();

  void enqueueIntoNB(const std::set<TNode> all, NodeBuilder<>& nb);

  Node explainInternal(TNode internal);

  void eqNotifyNewClass(TNode t);
  void eqNotifyPostMerge(TNode t1, TNode t2);
public:

  ArithCongruenceManager(context::Context* satContext, ConstraintDatabase&, SetupLiteralCallBack, const ArithVariables&, RaiseEqualityEngineConflict raiseConflict);
  ~ArithCongruenceManager();

  Node explain(TNode literal);
  void explain(TNode lit, NodeBuilder<>& out);

  void addWatchedPair(ArithVar s, TNode x, TNode y);

  inline bool isWatchedVariable(ArithVar s) const {
    return d_watchedVariables.isMember(s);
  }

  /** Assert an equality. */
  void watchedVariableIsZero(ConstraintCP eq);

  /** Assert a conjunction from lb and ub. */
  void watchedVariableIsZero(ConstraintCP lb, ConstraintCP ub);

  /** Assert that the value cannot be zero. */
  void watchedVariableCannotBeZero(ConstraintCP c);

  /** Assert that the value cannot be zero. */
  void watchedVariableCannotBeZero(ConstraintCP c, ConstraintCP d);


  /** Assert that the value is congruent to a constant. */
  void equalsConstant(ConstraintCP eq);
  void equalsConstant(ConstraintCP lb, ConstraintCP ub);


  void addSharedTerm(Node x);

  /** process inferred equalities based on Shostak normalization */
  bool fixpointInfer();
  
  eq::EqualityEngine * getEqualityEngine() { return &d_ee; }

private:
  class Statistics {
  public:
    IntStat d_watchedVariables;
    IntStat d_watchedVariableIsZero;
    IntStat d_watchedVariableIsNotZero;

    IntStat d_equalsConstantCalls;

    IntStat d_propagations;
    IntStat d_propagateConstraints;
    IntStat d_conflicts;

    Statistics();
    ~Statistics();
  } d_statistics;

};/* class ArithCongruenceManager */

}/* CVC4::theory::arith namespace */
}/* CVC4::theory namespace */
}/* CVC4 namespace */
