/*********************                                                        */
/*! \file theory_sets_rels.h
 ** \verbatim
 ** Original author: Paul Meng
 ** Major contributors: none
 ** Minor contributors (to current version): none
 ** This file is part of the CVC4 project.
 ** Copyright (c) 2009-2014  New York University and The University of Iowa
 ** See the file COPYING in the top-level source directory for licensing
 ** information.\endverbatim
 **
 ** \brief Sets theory implementation.
 **
 ** Extension to Sets theory.
 **/

#ifndef SRC_THEORY_SETS_THEORY_SETS_RELS_H_
#define SRC_THEORY_SETS_THEORY_SETS_RELS_H_

#include "theory/theory.h"
#include "theory/uf/equality_engine.h"
#include "context/cdhashset.h"
#include "context/cdchunk_list.h"
#include "theory/sets/rels_utils.h"

namespace CVC4 {
namespace theory {
namespace sets {

class TheorySets;


class TupleTrie {
public:
  /** the data */
  std::map< Node, TupleTrie > d_data;
public:
  std::vector<Node> findTerms( std::vector< Node >& reps, int argIndex = 0 );
  std::vector<Node> findSuccessors( std::vector< Node >& reps, int argIndex = 0 );
  Node existsTerm( std::vector< Node >& reps, int argIndex = 0 );
  bool addTerm( Node n, std::vector< Node >& reps, int argIndex = 0 );
  void debugPrint( const char * c, Node n, unsigned depth = 0 );
  void clear() { d_data.clear(); }
};/* class TupleTrie */

class TheorySetsRels {

  typedef context::CDChunkList< Node >                            NodeList;
  typedef context::CDHashSet< Node, NodeHashFunction >            NodeSet;
  typedef context::CDHashMap< Node, Node, NodeHashFunction >      NodeMap;

public:
  TheorySetsRels(context::Context* c,
                 context::UserContext* u,
                 eq::EqualityEngine*,
                 context::CDO<bool>*,
                 TheorySets&);

  ~TheorySetsRels();
  void check(Theory::Effort);
  void doPendingLemmas();

  bool isRelationKind( Kind k );
private:
  /** equivalence class info
   * d_mem tuples that are members of this equivalence class
   * d_not_mem tuples that are not members of this equivalence class
   * d_tp is a node of kind TRANSPOSE (if any) in this equivalence class,
   * d_pt is a node of kind PRODUCT (if any) in this equivalence class,
   * d_tc is a node of kind TCLOSURE (if any) in this equivalence class,
   */
  class EqcInfo
  {
  public:
    EqcInfo( context::Context* c );
    ~EqcInfo(){}
    NodeSet                     d_mem;
    NodeMap                     d_mem_exp;
    context::CDO< Node >        d_tp;
    context::CDO< Node >        d_pt;
    context::CDO< Node >        d_tc;
    context::CDO< Node >        d_rel_tc;
  };

private:

  /** has eqc info */
  bool hasEqcInfo( TNode n ) { return d_eqc_info.find( n )!=d_eqc_info.end(); }

  eq::EqualityEngine            *d_eqEngine;
  context::CDO<bool>            *d_conflict;
  TheorySets&                   d_sets_theory;

  /** True and false constant nodes */
  Node                          d_trueNode;
  Node                          d_falseNode;

  /** Facts and lemmas to be sent to EE */
  NodeList                      d_pending_merge;
  NodeSet                       d_lemmas_produced;
  NodeSet                       d_shared_terms;
  std::vector< Node >           d_lemmas_out;
  std::map< Node, Node >        d_pending_facts;


  std::hash_set< Node, NodeHashFunction >       d_rel_nodes;
  std::map< Node, std::vector<Node> >           d_tuple_reps;
  std::map< Node, TupleTrie >                   d_membership_trie;

  /** Symbolic tuple variables that has been reduced to concrete ones */
  std::hash_set< Node, NodeHashFunction >       d_symbolic_tuples;

  /** Mapping between relation and its member representatives */
  std::map< Node, std::vector< Node > >           d_rReps_memberReps_cache;

  /** Mapping between relation and its member representatives explanation */
  std::map< Node, std::vector< Node > >           d_rReps_memberReps_exp_cache;

  /** Mapping between a relation representative and its equivalent relations involving relational operators */
  std::map< Node, std::map<kind::Kind_t, std::vector<Node> > >                  d_terms_cache;

  /** Mapping between transitive closure relation TC(r) and its TC graph constructed based on the members of r*/
  std::map< Node, std::map< Node, std::hash_set<Node, NodeHashFunction> > >     d_rRep_tcGraph;
  std::map< Node, std::map< Node, std::hash_set<Node, NodeHashFunction> > >     d_tcr_tcGraph;
  std::map< Node, std::map< Node, Node > > d_tcr_tcGraph_exps;
  std::map< Node, std::vector< Node > > d_tc_lemmas_last;

  std::map< Node, EqcInfo* > d_eqc_info;

public:
  /** Standard effort notifications */
  void eqNotifyNewClass(Node t);
  void eqNotifyPostMerge(Node t1, Node t2);

private:

  /** Methods used in standard effort */
  void doPendingMerge();
  void sendInferProduct(Node member, Node pt_rel, Node exp);
  void sendInferTranspose(Node t1, Node t2, Node exp );
  void sendInferTClosure( Node mem_rep, EqcInfo* ei );
  void sendMergeInfer( Node fact, Node reason, const char * c );
  EqcInfo* getOrMakeEqcInfo( Node n, bool doMake = false );

  /** Methods used in full effort */
  void check();
  void collectRelsInfo();
  void applyTransposeRule( std::vector<Node> tp_terms );
  void applyTransposeRule( Node rel, Node rel_rep, Node exp );
  void applyProductRule( Node rel, Node rel_rep, Node exp );
  void applyJoinRule( Node rel, Node rel_rep, Node exp);
  void applyJoinImageRule( Node mem_rep, Node rel_rep, Node exp);
  void applyIdenRule( Node mem_rep, Node rel_rep, Node exp);
  void applyTCRule( Node mem, Node rel, Node rel_rep, Node exp);
  void buildTCGraphForRel( Node tc_rel );
  void doTCInference();
  void doTCInference( std::map< Node, std::hash_set<Node, NodeHashFunction> > rel_tc_graph, std::map< Node, Node > rel_tc_graph_exps, Node tc_rel );
  void doTCInference(Node tc_rel, std::vector< Node > reasons, std::map< Node, std::hash_set< Node, NodeHashFunction > >& tc_graph,
                       std::map< Node, Node >& rel_tc_graph_exps, Node start_node_rep, Node cur_node_rep, std::hash_set< Node, NodeHashFunction >& seen );

  void composeMembersForRels( Node );
  void computeMembersForBinOpRel( Node );
  void computeMembersForIdenTerm( Node );
  void computeMembersForUnaryOpRel( Node );
  void computeMembersForJoinImageTerm( Node );

  bool isTCReachable( Node mem_rep, Node tc_rel );
  void isTCReachable( Node start, Node dest, std::hash_set<Node, NodeHashFunction>& hasSeen,
                    std::map< Node, std::hash_set< Node, NodeHashFunction > >& tc_graph, bool& isReachable );


  void addSharedTerm( TNode n );
  void sendInfer( Node fact, Node exp, const char * c );
  void sendLemma( Node fact, Node reason, const char * c );
  void doTCLemmas();

  /** Helper functions */
  bool holds( Node );
  bool hasTerm( Node a );
  void makeSharedTerm( Node );
  void reduceTupleVar( Node );
  bool hasMember( Node, Node );
  void computeTupleReps( Node );
  bool areEqual( Node a, Node b );
  Node getRepresentative( Node t );
  bool exists( std::vector<Node>&, Node );
  Node mkAnd( std::vector< TNode >& assumptions );
  inline void addToMembershipDB( Node, Node, Node  );
  void printNodeMap(char* fst, char* snd, NodeMap map);
  inline Node constructPair(Node tc_rep, Node a, Node b);
  void addToMap( std::map< Node, std::vector<Node> >&, Node, Node );
  bool safelyAddToMap( std::map< Node, std::vector<Node> >&, Node, Node );
  bool isRel( Node n ) {return n.getType().isSet() && n.getType().getSetElementType().isTuple();}
};


}/* CVC4::theory::sets namespace */
}/* CVC4::theory namespace */
}/* CVC4 namespace */



#endif /* SRC_THEORY_SETS_THEORY_SETS_RELS_H_ */
