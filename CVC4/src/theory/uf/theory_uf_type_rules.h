/*********************                                                        */
/*! \file theory_uf_type_rules.h
 ** \verbatim
 ** Top contributors (to current version):
 **   Andrew Reynolds, Morgan Deters, Dejan Jovanovic
 ** This file is part of the CVC4 project.
 ** Copyright (c) 2009-2016 by the authors listed in the file AUTHORS
 ** in the top-level source directory) and their institutional affiliations.
 ** All rights reserved.  See the file COPYING in the top-level source
 ** directory for licensing information.\endverbatim
 **
 ** \brief [[ Add brief comments here ]]
 **
 ** [[ Add file-specific comments here ]]
 **/

#include "cvc4_private.h"

#ifndef __CVC4__THEORY__UF__THEORY_UF_TYPE_RULES_H
#define __CVC4__THEORY__UF__THEORY_UF_TYPE_RULES_H

namespace CVC4 {
namespace theory {
namespace uf {

class UfTypeRule {
 public:
  inline static TypeNode computeType(NodeManager* nodeManager, TNode n,
                                     bool check) {
    TNode f = n.getOperator();
    TypeNode fType = f.getType(check);
    if (!fType.isFunction()) {
      throw TypeCheckingExceptionPrivate(
          n, "operator does not have function type");
    }
    if (check) {
      if (n.getNumChildren() != fType.getNumChildren() - 1) {
        throw TypeCheckingExceptionPrivate(
            n, "number of arguments does not match the function type");
      }
      TNode::iterator argument_it = n.begin();
      TNode::iterator argument_it_end = n.end();
      TypeNode::iterator argument_type_it = fType.begin();
      for (; argument_it != argument_it_end;
           ++argument_it, ++argument_type_it) {
        TypeNode currentArgument = (*argument_it).getType();
        TypeNode currentArgumentType = *argument_type_it;
        if (!currentArgument.isComparableTo(currentArgumentType)) {
          std::stringstream ss;
          ss << "argument type is not a subtype of the function's argument "
             << "type:\n"
             << "argument:  " << *argument_it << "\n"
             << "has type:  " << (*argument_it).getType() << "\n"
             << "not subtype: " << *argument_type_it;
          throw TypeCheckingExceptionPrivate(n, ss.str());
        }
      }
    }
    return fType.getRangeType();
  }
}; /* class UfTypeRule */

class CardinalityConstraintTypeRule {
 public:
  inline static TypeNode computeType(NodeManager* nodeManager, TNode n,
                                     bool check) {
    if (check) {
      // don't care what it is, but it should be well-typed
      n[0].getType(check);

      TypeNode valType = n[1].getType(check);
      if (valType != nodeManager->integerType()) {
        throw TypeCheckingExceptionPrivate(
            n, "cardinality constraint must be integer");
      }
      if (n[1].getKind() != kind::CONST_RATIONAL) {
        throw TypeCheckingExceptionPrivate(
            n, "cardinality constraint must be a constant");
      }
      CVC4::Rational r(INT_MAX);
      if (n[1].getConst<Rational>() > r) {
        throw TypeCheckingExceptionPrivate(
            n, "Exceeded INT_MAX in cardinality constraint");
      }
      if (n[1].getConst<Rational>().getNumerator().sgn() != 1) {
        throw TypeCheckingExceptionPrivate(
            n, "cardinality constraint must be positive");
      }
    }
    return nodeManager->booleanType();
  }
}; /* class CardinalityConstraintTypeRule */

class CombinedCardinalityConstraintTypeRule {
 public:
  inline static TypeNode computeType(NodeManager* nodeManager, TNode n,
                                     bool check) {
    if (check) {
      TypeNode valType = n[0].getType(check);
      if (valType != nodeManager->integerType()) {
        throw TypeCheckingExceptionPrivate(
            n, "combined cardinality constraint must be integer");
      }
      if (n[0].getKind() != kind::CONST_RATIONAL) {
        throw TypeCheckingExceptionPrivate(
            n, "combined cardinality constraint must be a constant");
      }
      CVC4::Rational r(INT_MAX);
      if (n[0].getConst<Rational>() > r) {
        throw TypeCheckingExceptionPrivate(
            n, "Exceeded INT_MAX in combined cardinality constraint");
      }
      if (n[0].getConst<Rational>().getNumerator().sgn() == -1) {
        throw TypeCheckingExceptionPrivate(
            n, "combined cardinality constraint must be non-negative");
      }
    }
    return nodeManager->booleanType();
  }
}; /* class CardinalityConstraintTypeRule */

class PartialTypeRule {
 public:
  inline static TypeNode computeType(NodeManager* nodeManager, TNode n,
                                     bool check) {
    return n.getOperator().getType().getRangeType();
  }
}; /* class PartialTypeRule */

class CardinalityValueTypeRule {
 public:
  inline static TypeNode computeType(NodeManager* nodeManager, TNode n,
                                     bool check) {
    if (check) {
      n[0].getType(check);
    }
    return nodeManager->integerType();
  }
}; /* class CardinalityValueTypeRule */

} /* CVC4::theory::uf namespace */
} /* CVC4::theory namespace */
} /* CVC4 namespace */

#endif /* __CVC4__THEORY__UF__THEORY_UF_TYPE_RULES_H */
