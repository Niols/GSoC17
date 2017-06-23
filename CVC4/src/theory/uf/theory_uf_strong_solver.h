/*********************                                                        */
/*! \file theory_uf_strong_solver.h
 ** \verbatim
 ** Top contributors (to current version):
 **   Andrew Reynolds, Morgan Deters, Tim King
 ** This file is part of the CVC4 project.
 ** Copyright (c) 2009-2016 by the authors listed in the file AUTHORS
 ** in the top-level source directory) and their institutional affiliations.
 ** All rights reserved.  See the file COPYING in the top-level source
 ** directory for licensing information.\endverbatim
 **
 ** \brief Theory uf strong solver
 **/

#include "cvc4_private.h"

#ifndef __CVC4__THEORY_UF_STRONG_SOLVER_H
#define __CVC4__THEORY_UF_STRONG_SOLVER_H

#include "context/cdchunk_list.h"
#include "context/cdhashmap.h"
#include "context/context.h"
#include "context/context_mm.h"
#include "theory/theory.h"
#include "util/statistics_registry.h"

namespace CVC4 {
class SortInference;
namespace theory {
class SubsortSymmetryBreaker;
namespace uf {
class TheoryUF;
class DisequalityPropagator;
} /* namespace CVC4::theory::uf */
} /* namespace CVC4::theory */
} /* namespace CVC4 */

namespace CVC4 {
namespace theory {
namespace uf {

class StrongSolverTheoryUF{
protected:
  typedef context::CDHashMap<Node, bool, NodeHashFunction> NodeBoolMap;
  typedef context::CDHashMap<Node, int, NodeHashFunction> NodeIntMap;
  typedef context::CDHashMap<Node, Node, NodeHashFunction> NodeNodeMap;
  typedef context::CDHashMap<TypeNode, bool, TypeNodeHashFunction> TypeNodeBoolMap;
public:
  /**
   * Information for incremental conflict/clique finding for a
   * particular sort.
   */
  class SortModel {
  private:
    std::map< Node, std::vector< int > > d_totality_lems;
    std::map< TypeNode, std::map< int, std::vector< Node > > > d_sym_break_terms;
    std::map< Node, int > d_sym_break_index;
  public:

    /**
     * A partition of the current equality graph for which cliques
     * can occur internally.
     */
    class Region {
    public:
      /** information stored about each node in region */
      class RegionNodeInfo {
      public:
        /** disequality list for node */
        class DiseqList {
        public:
          DiseqList( context::Context* c )
            : d_size( c, 0 ), d_disequalities( c ) {}
          ~DiseqList(){}

          void setDisequal( Node n, bool valid ){
            Assert( (!isSet(n)) || getDisequalityValue(n) != valid );
            d_disequalities[ n ] = valid;
            d_size = d_size + ( valid ? 1 : -1 );
          }
          bool isSet(Node n) const {
            return d_disequalities.find(n) != d_disequalities.end();
          }
          bool getDisequalityValue(Node n) const {
            Assert(isSet(n));
            return (*(d_disequalities.find(n))).second;
          }

          int size() const { return d_size; }
          
          typedef NodeBoolMap::iterator iterator;
          iterator begin() { return d_disequalities.begin(); }
          iterator end() { return d_disequalities.end(); }

        private:
          context::CDO< int > d_size;
          NodeBoolMap d_disequalities;
        }; /* class DiseqList */
      public:
        /** constructor */
        RegionNodeInfo( context::Context* c )
          : d_internal(c), d_external(c), d_valid(c, true) {
          d_disequalities[0] = &d_internal;
          d_disequalities[1] = &d_external;
        }
        ~RegionNodeInfo(){}
       
        int getNumDisequalities() const {
          return d_disequalities[0]->size() + d_disequalities[1]->size();
        }
        int getNumExternalDisequalities() const {
          return d_disequalities[0]->size();
        }
        int getNumInternalDisequalities() const {
          return d_disequalities[1]->size();
        }

        bool valid() const { return d_valid; }
        void setValid(bool valid) { d_valid = valid; }

        DiseqList* get(unsigned i) { return d_disequalities[i]; }

      private:
        DiseqList d_internal;
        DiseqList d_external;
        context::CDO< bool > d_valid;
        DiseqList* d_disequalities[2];
      }; /* class RegionNodeInfo */

    private:
      /** conflict find pointer */
      SortModel* d_cf;

      context::CDO< unsigned > d_testCliqueSize;
      context::CDO< unsigned > d_splitsSize;
      //a postulated clique
      NodeBoolMap d_testClique;
      //disequalities needed for this clique to happen
      NodeBoolMap d_splits;
      //number of valid representatives in this region
      context::CDO< unsigned > d_reps_size;
      //total disequality size (external)
      context::CDO< unsigned > d_total_diseq_external;
      //total disequality size (internal)
      context::CDO< unsigned > d_total_diseq_internal;
      /** set rep */
      void setRep( Node n, bool valid );
      //region node infomation
      std::map< Node, RegionNodeInfo* > d_nodes;
      //whether region is valid
      context::CDO< bool > d_valid;

    public:
      //constructor
      Region( SortModel* cf, context::Context* c );
      virtual ~Region();

      typedef std::map< Node, RegionNodeInfo* >::iterator iterator;
      iterator begin() { return d_nodes.begin(); }
      iterator end() { return d_nodes.end(); }

      typedef NodeBoolMap::iterator split_iterator;
      split_iterator begin_splits() { return d_splits.begin(); }
      split_iterator end_splits() { return d_splits.end(); }

      /** Returns a RegionInfo. */
      RegionNodeInfo* getRegionInfo(Node n) {
        Assert(d_nodes.find(n) != d_nodes.end());
        return (* (d_nodes.find(n))).second;
      }

      /** Returns whether or not d_valid is set in current context. */
      bool valid() const { return d_valid; }

      /** Sets d_valid to the value valid in the current context.*/
      void setValid(bool valid) { d_valid = valid; }

      /** add rep */
      void addRep( Node n );
      //take node from region
      void takeNode( Region* r, Node n );
      //merge with other region
      void combine( Region* r );
      /** merge */
      void setEqual( Node a, Node b );
      //set n1 != n2 to value 'valid', type is whether it is internal/external
      void setDisequal( Node n1, Node n2, int type, bool valid );
      //get num reps
      int getNumReps() { return d_reps_size; }
      //get test clique size
      int getTestCliqueSize() { return d_testCliqueSize; }
      // has representative
      bool hasRep( Node n ) {
        return d_nodes.find(n) != d_nodes.end() && d_nodes[n]->valid();
      }
      // is disequal
      bool isDisequal( Node n1, Node n2, int type );
      /** get must merge */
      bool getMustCombine( int cardinality );
      /** has splits */
      bool hasSplits() { return d_splitsSize>0; }
      /** get external disequalities */
      void getNumExternalDisequalities(std::map< Node, int >& num_ext_disequalities );
      /** check for cliques */
      bool check( Theory::Effort level, int cardinality, std::vector< Node >& clique );
      /** get candidate clique */
      bool getCandidateClique( int cardinality, std::vector< Node >& clique );
      //print debug
      void debugPrint( const char* c, bool incClique = false );

      // Returns the first key in d_nodes.
      Node frontKey() const { return d_nodes.begin()->first; }
    }; /* class Region */

  private:
    /** the type this model is for */
    TypeNode d_type;
    /** strong solver pointer */
    StrongSolverTheoryUF* d_thss;
    /** regions used to d_region_index */
    context::CDO< unsigned > d_regions_index;
    /** vector of regions */
    std::vector< Region* > d_regions;
    /** map from Nodes to index of d_regions they exist in, -1 means invalid */
    NodeIntMap d_regions_map;
    /** the score for each node for splitting */
    NodeIntMap d_split_score;
    /** number of valid disequalities in d_disequalities */
    context::CDO< unsigned > d_disequalities_index;
    /** list of all disequalities */
    std::vector< Node > d_disequalities;
    /** number of representatives in all regions */
    context::CDO< unsigned > d_reps;

    /** get number of disequalities from node n to region ri */
    int getNumDisequalitiesToRegion( Node n, int ri );
    /** get number of disequalities from Region r to other regions */
    void getDisequalitiesToRegions( int ri, std::map< int, int >& regions_diseq );
    /** is valid */
    bool isValid( int ri ) {
      return ri>=0 && ri<(int)d_regions_index && d_regions[ ri ]->valid();
    }
    /** set split score */
    void setSplitScore( Node n, int s );
    /** check if we need to combine region ri */
    void checkRegion( int ri, bool checkCombine = true );
    /** force combine region */
    int forceCombineRegion( int ri, bool useDensity = true );
    /** merge regions */
    int combineRegions( int ai, int bi );
    /** move node n to region ri */
    void moveNode( Node n, int ri );
    /** allocate cardinality */
    void allocateCardinality( OutputChannel* out );
    /**
     * Add splits. Returns
     *   0 = no split,
     *  -1 = entailed disequality added, or
     *   1 = split added.
     */
    int addSplit( Region* r, OutputChannel* out );
    /** add clique lemma */
    void addCliqueLemma( std::vector< Node >& clique, OutputChannel* out );
    /** add totality axiom */
    void addTotalityAxiom( Node n, int cardinality, OutputChannel* out );

    class NodeTrie {
    public:
      bool add( std::vector< Node >& n, unsigned i = 0 ){
        Assert( i<n.size() );
        if( i==(n.size()-1) ){
          bool ret = d_children.find( n[i] )==d_children.end();
          d_children[n[i]].d_children.clear();
          return ret;
        }else{
          return d_children[n[i]].add( n, i+1 );
        }
      }
    private:
      std::map< Node, NodeTrie > d_children;
    }; /* class NodeTrie */

    std::map< int, NodeTrie > d_clique_trie;
    void addClique( int c, std::vector< Node >& clique );

    /** Are we in conflict */
    context::CDO<bool> d_conflict;
    /** cardinality */
    context::CDO< int > d_cardinality;
    /** maximum allocated cardinality */
    context::CDO< int > d_aloc_cardinality;
    /** cardinality lemma term */
    Node d_cardinality_term;
    /** cardinality totality terms */
    std::map< int, std::vector< Node > > d_totality_terms;
    /** cardinality literals */
    std::map< int, Node > d_cardinality_literal;
    /** cardinality lemmas */
    std::map< int, Node > d_cardinality_lemma;
    /** whether a positive cardinality constraint has been asserted */
    context::CDO< bool > d_hasCard;
    /** clique lemmas that have been asserted */
    std::map< int, std::vector< std::vector< Node > > > d_cliques;
    /** maximum negatively asserted cardinality */
    context::CDO< int > d_maxNegCard;
    /** list of fresh representatives allocated */
    std::vector< Node > d_fresh_aloc_reps;
    /** whether we are initialized */
    context::CDO< bool > d_initialized;
    /** cache for lemmas */
    NodeBoolMap d_lemma_cache;

    /** apply totality */
    bool applyTotality( int cardinality );
    /** get totality lemma terms */
    Node getTotalityLemmaTerm( int cardinality, int i );
    /** simple check cardinality */
    void simpleCheckCardinality();

    bool doSendLemma( Node lem );

  public:
    SortModel( Node n, context::Context* c, context::UserContext* u,
               StrongSolverTheoryUF* thss );
    virtual ~SortModel();
    /** initialize */
    void initialize( OutputChannel* out );
    /** new node */
    void newEqClass( Node n );
    /** merge */
    void merge( Node a, Node b );
    /** assert terms are disequal */
    void assertDisequal( Node a, Node b, Node reason );
    /** are disequal */
    bool areDisequal( Node a, Node b );
    /** check */
    void check( Theory::Effort level, OutputChannel* out );
    /** presolve */
    void presolve();
    /** propagate */
    void propagate( Theory::Effort level, OutputChannel* out );
    /** get next decision request */
    Node getNextDecisionRequest();
    /** minimize */
    bool minimize( OutputChannel* out, TheoryModel* m );
    /** assert cardinality */
    void assertCardinality( OutputChannel* out, int c, bool val );
    /** is in conflict */
    bool isConflict() { return d_conflict; }
    /** get cardinality */
    int getCardinality() { return d_cardinality; }
    /** has cardinality */
    bool hasCardinalityAsserted() { return d_hasCard; }
    /** get cardinality term */
    Node getCardinalityTerm() { return d_cardinality_term; }
    /** get cardinality literal */
    Node getCardinalityLiteral( int c );
    /** get maximum negative cardinality */
    int getMaximumNegativeCardinality() { return d_maxNegCard.get(); }
    //print debug
    void debugPrint( const char* c );
    /** debug a model */
    bool debugModel( TheoryModel* m );
  public:
    /** get number of regions (for debugging) */
    int getNumRegions();
  }; /** class SortModel */

public:
  StrongSolverTheoryUF(context::Context* c, context::UserContext* u,
                       OutputChannel& out, TheoryUF* th);
  ~StrongSolverTheoryUF();
  /** get theory */
  TheoryUF* getTheory() { return d_th; }
  /** disequality propagator */
  DisequalityPropagator* getDisequalityPropagator() { return d_deq_prop; }
  /** symmetry breaker */
  SubsortSymmetryBreaker* getSymmetryBreaker() { return d_sym_break; }
  /** get sort inference module */
  SortInference* getSortInference();
  /** get default sat context */
  context::Context* getSatContext();
  /** get default output channel */
  OutputChannel& getOutputChannel();
  /** new node */
  void newEqClass( Node n );
  /** merge */
  void merge( Node a, Node b );
  /** assert terms are disequal */
  void assertDisequal( Node a, Node b, Node reason );
  /** assert node */
  void assertNode( Node n, bool isDecision );
  /** are disequal */
  bool areDisequal( Node a, Node b );
  /** check */
  void check( Theory::Effort level );
  /** presolve */
  void presolve();
  /** propagate */
  void propagate( Theory::Effort level );
  /** get next decision request */
  Node getNextDecisionRequest( unsigned& priority );
  /** preregister a term */
  void preRegisterTerm( TNode n );
  /** notify restart */
  void notifyRestart();
  /** identify */
  std::string identify() const { return std::string("StrongSolverTheoryUF"); }
  //print debug
  void debugPrint( const char* c );
  /** debug a model */
  bool debugModel( TheoryModel* m );
  /** get is in conflict */
  bool isConflict() { return d_conflict; }
  /** get cardinality for node */
  int getCardinality( Node n );
  /** get cardinality for type */
  int getCardinality( TypeNode tn );
  /** minimize */
  bool minimize( TheoryModel* m = NULL );
  /** has eqc */
  bool hasEqc(Node a);

  class Statistics {
   public:
    IntStat d_clique_conflicts;
    IntStat d_clique_lemmas;
    IntStat d_split_lemmas;
    IntStat d_disamb_term_lemmas;
    IntStat d_sym_break_lemmas;
    IntStat d_totality_lemmas;
    IntStat d_max_model_size;
    Statistics();
    ~Statistics();
  };
  /** statistics class */
  Statistics d_statistics;

 private:
  /** get sort model */
  SortModel* getSortModel(Node n);
  /** initialize */
  void initializeCombinedCardinality();
  /** allocateCombinedCardinality */
  void allocateCombinedCardinality();
  /** check */
  void checkCombinedCardinality();
  /** ensure eqc */
  void ensureEqc(SortModel* c, Node a);
  /** ensure eqc for all subterms of n */
  void ensureEqcRec(Node n);

  /** The output channel for the strong solver. */
  OutputChannel* d_out;
  /** theory uf pointer */
  TheoryUF* d_th;
  /** Are we in conflict */
  context::CDO<bool> d_conflict;
  /** rep model structure, one for each type */
  std::map<TypeNode, SortModel*> d_rep_model;
  /** allocated combined cardinality */
  context::CDO<int> d_aloc_com_card;
  /** combined cardinality constraints */
  std::map<int, Node> d_com_card_literal;
  /** combined cardinality assertions (indexed by cardinality literals ) */
  NodeBoolMap d_com_card_assertions;
  /** minimum positive combined cardinality */
  context::CDO<int> d_min_pos_com_card;
  /** cardinality literals for which we have added */
  NodeBoolMap d_card_assertions_eqv_lemma;
  /** the master monotone type (if ufssFairnessMonotone enabled) */
  TypeNode d_tn_mono_master;
  std::map<TypeNode, bool> d_tn_mono_slave;
  context::CDO<int> d_min_pos_tn_master_card;
  /** relevant eqc */
  NodeBoolMap d_rel_eqc;
  /** disequality propagator */
  DisequalityPropagator* d_deq_prop;
  /** symmetry breaking techniques */
  SubsortSymmetryBreaker* d_sym_break;
}; /* class StrongSolverTheoryUF */

class DisequalityPropagator {
public:
  DisequalityPropagator(QuantifiersEngine* qe, StrongSolverTheoryUF* ufss);
  /** merge */
  void merge( Node a, Node b );
  /** assert terms are disequal */
  void assertDisequal( Node a, Node b, Node reason );
  /** assert predicate */
  void assertPredicate( Node p, bool polarity );

  class Statistics {
  public:
    IntStat d_propagations;
    Statistics();
    ~Statistics();
  };
  /** statistics class */
  Statistics d_statistics;

private:
  /** quantifiers engine */
  QuantifiersEngine* d_qe;
  /** strong solver */
  StrongSolverTheoryUF* d_ufss;
  /** true,false */
  Node d_true;
  Node d_false;
  /** check term t against equivalence class that t is disequal from */
  void checkEquivalenceClass( Node t, Node eqc );
}; /* class DisequalityPropagator */

}/* CVC4::theory namespace::uf */
}/* CVC4::theory namespace */
}/* CVC4 namespace */

#endif /* __CVC4__THEORY_UF_STRONG_SOLVER_H */
