/*********************                                                        */
/*! \file bv_subtheory_inequality.h
 ** \verbatim
 ** Top contributors (to current version):
 **   Liana Hadarean, Andrew Reynolds, Morgan Deters
 ** This file is part of the CVC4 project.
 ** Copyright (c) 2009-2016 by the authors listed in the file AUTHORS
 ** in the top-level source directory) and their institutional affiliations.
 ** All rights reserved.  See the file COPYING in the top-level source
 ** directory for licensing information.\endverbatim
 **
 ** \brief Algebraic solver.
 **
 ** Algebraic solver.
 **/

#include "cvc4_private.h"

#ifndef __CVC4__THEORY__BV__BV_SUBTHEORY__INEQUALITY_H
#define __CVC4__THEORY__BV__BV_SUBTHEORY__INEQUALITY_H

#include "theory/bv/bv_subtheory.h"
#include "theory/bv/bv_inequality_graph.h"
#include "context/cdhashset.h"
#include "expr/attribute.h"

namespace CVC4 {
namespace theory {
namespace bv {

/** Cache for InequalitySolver::isInequalityOnly() */
struct IneqOnlyAttributeId {};
typedef expr::Attribute<IneqOnlyAttributeId, bool> IneqOnlyAttribute;

/** Whether the above has been computed yet or not for an expr */
struct IneqOnlyComputedAttributeId {};
typedef expr::Attribute<IneqOnlyComputedAttributeId, bool> IneqOnlyComputedAttribute;

class InequalitySolver: public SubtheorySolver {
  struct Statistics {
    IntStat d_numCallstoCheck;
    Statistics();
    ~Statistics();
  };

  context::CDHashSet<Node, NodeHashFunction> d_assertionSet;
  InequalityGraph d_inequalityGraph;
  context::CDHashMap<Node, TNode, NodeHashFunction> d_explanations;
  context::CDO<bool> d_isComplete;
  typedef __gnu_cxx::hash_set<Node, NodeHashFunction> NodeSet;
  NodeSet d_ineqTerms;
  bool isInequalityOnly(TNode node);
  bool addInequality(TNode a, TNode b, bool strict, TNode fact);
  Statistics d_statistics;
public:
  InequalitySolver(context::Context* c, context::Context* u, TheoryBV* bv)
    : SubtheorySolver(c, bv),
      d_assertionSet(c),
      d_inequalityGraph(c, u),
      d_explanations(c),
      d_isComplete(c, true),
      d_ineqTerms(),
      d_statistics()
  {}

  bool check(Theory::Effort e);
  void propagate(Theory::Effort e);
  void explain(TNode literal, std::vector<TNode>& assumptions);
  bool isComplete() { return d_isComplete; }
  void collectModelInfo(TheoryModel* m, bool fullModel);
  Node getModelValue(TNode var);
  EqualityStatus getEqualityStatus(TNode a, TNode b);
  void assertFact(TNode fact);
  void preRegister(TNode node);
};

}
}
}

#endif /* __CVC4__THEORY__BV__BV_SUBTHEORY__INEQUALITY_H */
