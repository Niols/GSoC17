/*********************                                                        */
/*! \file theory_sets_type_rules.h
 ** \verbatim
 ** Top contributors (to current version):
 **   Kshitij Bansal, Tim King, Morgan Deters
 ** This file is part of the CVC4 project.
 ** Copyright (c) 2009-2016 by the authors listed in the file AUTHORS
 ** in the top-level source directory) and their institutional affiliations.
 ** All rights reserved.  See the file COPYING in the top-level source
 ** directory for licensing information.\endverbatim
 **
 ** \brief Sets theory type rules.
 **
 ** Sets theory type rules.
 **/

#include "cvc4_private.h"

#ifndef __CVC4__THEORY__SETS__THEORY_SETS_TYPE_RULES_H
#define __CVC4__THEORY__SETS__THEORY_SETS_TYPE_RULES_H

#include "theory/sets/normal_form.h"

namespace CVC4 {
namespace theory {
namespace sets {

class SetsTypeRule {
public:

  /**
   * Compute the type for (and optionally typecheck) a term belonging
   * to the theory of sets.
   *
   * @param check if true, the node's type should be checked as well
   * as computed.
   */
  inline static TypeNode computeType(NodeManager* nodeManager, TNode n,
                                     bool check)
    throw (TypeCheckingExceptionPrivate) {

    // TODO: implement me!
    Unimplemented();

  }

};/* class SetsTypeRule */

struct SetsBinaryOperatorTypeRule {
  inline static TypeNode computeType(NodeManager* nodeManager, TNode n, bool check)
    throw (TypeCheckingExceptionPrivate, AssertionException) {
    Assert(n.getKind() == kind::UNION ||
           n.getKind() == kind::INTERSECTION ||
           n.getKind() == kind::SETMINUS);
    TypeNode setType = n[0].getType(check);
    if( check ) {
      if(!setType.isSet()) {
        throw TypeCheckingExceptionPrivate(n, "operator expects a set, first argument is not");
      }
      TypeNode secondSetType = n[1].getType(check);
      if(secondSetType != setType) {
        if( n.getKind() == kind::INTERSECTION ){
          setType = TypeNode::mostCommonTypeNode( secondSetType, setType );
        }else{
          setType = TypeNode::leastCommonTypeNode( secondSetType, setType );
        }
        if( setType.isNull() ){
          throw TypeCheckingExceptionPrivate(n, "operator expects two sets of comparable types");
        }
        
      }
    }
    return setType;
  }

  inline static bool computeIsConst(NodeManager* nodeManager, TNode n) {
    Assert(n.getKind() == kind::UNION ||
           n.getKind() == kind::INTERSECTION ||
           n.getKind() == kind::SETMINUS);
    if(n.getKind() == kind::UNION) {
      return NormalForm::checkNormalConstant(n);
    } else {
      return false;
    }
  }
};/* struct SetUnionTypeRule */

struct SubsetTypeRule {
  inline static TypeNode computeType(NodeManager* nodeManager, TNode n, bool check)
    throw (TypeCheckingExceptionPrivate, AssertionException) {
    Assert(n.getKind() == kind::SUBSET);
    TypeNode setType = n[0].getType(check);
    if( check ) {
      if(!setType.isSet()) {
        throw TypeCheckingExceptionPrivate(n, "set subset operating on non-set");
      }
      TypeNode secondSetType = n[1].getType(check);
      if(secondSetType != setType) {
        if( !setType.isComparableTo( secondSetType ) ){
          throw TypeCheckingExceptionPrivate(n, "set subset operating on sets of different types");
        }
      }
    }
    return nodeManager->booleanType();
  }
};/* struct SetSubsetTypeRule */

struct MemberTypeRule {
  inline static TypeNode computeType(NodeManager* nodeManager, TNode n, bool check)
    throw (TypeCheckingExceptionPrivate, AssertionException) {
    Assert(n.getKind() == kind::MEMBER);
    TypeNode setType = n[1].getType(check);
    if( check ) {
      if(!setType.isSet()) {
        throw TypeCheckingExceptionPrivate(n, "checking for membership in a non-set");
      }
      TypeNode elementType = n[0].getType(check);
      if(!elementType.isComparableTo(setType.getSetElementType())) {
        throw TypeCheckingExceptionPrivate(n, "member operating on sets of different types");
      }
    }
    return nodeManager->booleanType();
  }
};/* struct MemberTypeRule */

struct SingletonTypeRule {
  inline static TypeNode computeType(NodeManager* nodeManager, TNode n, bool check)
    throw (TypeCheckingExceptionPrivate, AssertionException) {
    Assert(n.getKind() == kind::SINGLETON);
    return nodeManager->mkSetType(n[0].getType(check));
  }

  inline static bool computeIsConst(NodeManager* nodeManager, TNode n) {
    Assert(n.getKind() == kind::SINGLETON);
    return n[0].isConst();
  }
};/* struct SingletonTypeRule */

struct EmptySetTypeRule {
  inline static TypeNode computeType(NodeManager* nodeManager, TNode n, bool check)
    throw (TypeCheckingExceptionPrivate, AssertionException) {
    Assert(n.getKind() == kind::EMPTYSET);
    EmptySet emptySet = n.getConst<EmptySet>();
    Type setType = emptySet.getType();
    return TypeNode::fromType(setType);
  }
};/* struct EmptySetTypeRule */

struct CardTypeRule {
  inline static TypeNode computeType(NodeManager* nodeManager, TNode n, bool check)
    throw (TypeCheckingExceptionPrivate, AssertionException) {
    Assert(n.getKind() == kind::CARD);
    TypeNode setType = n[0].getType(check);
    if( check ) {
      if(!setType.isSet()) {
        throw TypeCheckingExceptionPrivate(n, "cardinality operates on a set, non-set object found");
      }
    }
    return nodeManager->integerType();
  }

  inline static bool computeIsConst(NodeManager* nodeManager, TNode n) {
    Assert(n.getKind() == kind::CARD);
    return false;
  }
};/* struct CardTypeRule */

struct ComplementTypeRule {
  inline static TypeNode computeType(NodeManager* nodeManager, TNode n, bool check)
    throw (TypeCheckingExceptionPrivate, AssertionException) {
    Assert(n.getKind() == kind::COMPLEMENT);
    TypeNode setType = n[0].getType(check);
    if( check ) {
      if(!setType.isSet()) {
        throw TypeCheckingExceptionPrivate(n, "COMPLEMENT operates on a set, non-set object found");
      }
    }
    return setType;
  }

  inline static bool computeIsConst(NodeManager* nodeManager, TNode n) {
    Assert(n.getKind() == kind::COMPLEMENT);
    return false;
  }
};/* struct ComplementTypeRule */

struct UniverseSetTypeRule {
  inline static TypeNode computeType(NodeManager* nodeManager, TNode n, bool check)
    throw (TypeCheckingExceptionPrivate, AssertionException) {
    Assert(n.getKind() == kind::UNIVERSE_SET);
    // for nullary operators, we only computeType for check=true, since they are given TypeAttr() on creation
    Assert(check);
    TypeNode setType = n.getType();
    if(!setType.isSet()) {
      throw TypeCheckingExceptionPrivate(n, "Non-set type found for universe set");
    }
    return setType;
  }
};/* struct ComplementTypeRule */

struct InsertTypeRule {
  inline static TypeNode computeType(NodeManager* nodeManager, TNode n, bool check)
    throw (TypeCheckingExceptionPrivate, AssertionException) {
    Assert(n.getKind() == kind::INSERT);
    size_t numChildren = n.getNumChildren();
    Assert( numChildren >= 2 );
    TypeNode setType = n[numChildren-1].getType(check);
    if( check ) {
      if(!setType.isSet()) {
        throw TypeCheckingExceptionPrivate(n, "inserting into a non-set");
      }
      for(size_t i = 0; i < numChildren-1; ++i) {
        TypeNode elementType = n[i].getType(check);
        if(elementType != setType.getSetElementType()) {
          throw TypeCheckingExceptionPrivate
            (n, "type of element should be same as element type of set being inserted into");
        }
      }
    }
    return setType;
  }

  inline static bool computeIsConst(NodeManager* nodeManager, TNode n) {
    Assert(n.getKind() == kind::INSERT);
    return false;
  }
};/* struct InsertTypeRule */

struct RelBinaryOperatorTypeRule {
  inline static TypeNode computeType(NodeManager* nodeManager, TNode n, bool check)
    throw (TypeCheckingExceptionPrivate, AssertionException) {
    Assert(n.getKind() == kind::PRODUCT ||
           n.getKind() == kind::JOIN);

    TypeNode firstRelType = n[0].getType(check);
    TypeNode secondRelType = n[1].getType(check);
    TypeNode resultType = firstRelType;

    if(!firstRelType.isSet() || !secondRelType.isSet()) {
      throw TypeCheckingExceptionPrivate(n, " Relational operator operates on non-sets");
    }
    if(!firstRelType[0].isTuple() || !secondRelType[0].isTuple()) {
      throw TypeCheckingExceptionPrivate(n, " Relational operator operates on non-relations (sets of tuples)");
    }

    std::vector<TypeNode> newTupleTypes;
    std::vector<TypeNode> firstTupleTypes = firstRelType[0].getTupleTypes();
    std::vector<TypeNode> secondTupleTypes = secondRelType[0].getTupleTypes();

    // JOIN is not allowed to apply on two unary sets
    if( n.getKind() == kind::JOIN ) {
      if((firstTupleTypes.size() == 1) && (secondTupleTypes.size() == 1)) {
        throw TypeCheckingExceptionPrivate(n, " Join operates on two unary relations");
      } else if(firstTupleTypes.back() != secondTupleTypes.front()) {
        throw TypeCheckingExceptionPrivate(n, " Join operates on two non-joinable relations");
      }
      newTupleTypes.insert(newTupleTypes.end(), firstTupleTypes.begin(), firstTupleTypes.end()-1);
      newTupleTypes.insert(newTupleTypes.end(), secondTupleTypes.begin()+1, secondTupleTypes.end());
    }else if( n.getKind() == kind::PRODUCT ) {
      newTupleTypes.insert(newTupleTypes.end(), firstTupleTypes.begin(), firstTupleTypes.end());
      newTupleTypes.insert(newTupleTypes.end(), secondTupleTypes.begin(), secondTupleTypes.end());
    }
    resultType = nodeManager->mkSetType(nodeManager->mkTupleType(newTupleTypes));

    return resultType;
  }

  inline static bool computeIsConst(NodeManager* nodeManager, TNode n) {
    Assert(n.getKind() == kind::JOIN ||
           n.getKind() == kind::PRODUCT);
    return false;
  }
};/* struct RelBinaryOperatorTypeRule */

struct RelTransposeTypeRule {
  inline static TypeNode computeType(NodeManager* nodeManager, TNode n, bool check)
    throw (TypeCheckingExceptionPrivate, AssertionException) {
    Assert(n.getKind() == kind::TRANSPOSE);
    TypeNode setType = n[0].getType(check);
    if(check && (!setType.isSet() || !setType.getSetElementType().isTuple())) {
        throw TypeCheckingExceptionPrivate(n, "relation transpose operates on non-relation");
    }
    std::vector<TypeNode> tupleTypes = setType[0].getTupleTypes();
    std::reverse(tupleTypes.begin(), tupleTypes.end());
    return nodeManager->mkSetType(nodeManager->mkTupleType(tupleTypes));
  }

  inline static bool computeIsConst(NodeManager* nodeManager, TNode n) {
      return false;
    }
};/* struct RelTransposeTypeRule */

struct RelTransClosureTypeRule {
  inline static TypeNode computeType(NodeManager* nodeManager, TNode n, bool check)
    throw (TypeCheckingExceptionPrivate, AssertionException) {
    Assert(n.getKind() == kind::TCLOSURE);
    TypeNode setType = n[0].getType(check);
    if(check) {
      if(!setType.isSet() || !setType.getSetElementType().isTuple()) {
        throw TypeCheckingExceptionPrivate(n, " transitive closure operates on non-relation");
      }
      std::vector<TypeNode> tupleTypes = setType[0].getTupleTypes();
      if(tupleTypes.size() != 2) {
        throw TypeCheckingExceptionPrivate(n, " transitive closure operates on non-binary relations");
      }
      if(tupleTypes[0] != tupleTypes[1]) {
        throw TypeCheckingExceptionPrivate(n, " transitive closure operates on non-homogeneous binary relations");
      }
    }
    return setType;
  }

  inline static bool computeIsConst(NodeManager* nodeManager, TNode n) {
      Assert(n.getKind() == kind::TCLOSURE);
      return false;
    }
};/* struct RelTransClosureTypeRule */

struct JoinImageTypeRule {
  inline static TypeNode computeType(NodeManager* nodeManager, TNode n, bool check)
    throw (TypeCheckingExceptionPrivate, AssertionException) {
    Assert(n.getKind() == kind::JOIN_IMAGE);

    TypeNode firstRelType = n[0].getType(check);

    if(!firstRelType.isSet()) {
      throw TypeCheckingExceptionPrivate(n, " JoinImage operator operates on non-relations");
    }
    if(!firstRelType[0].isTuple()) {
      throw TypeCheckingExceptionPrivate(n, " JoinImage operator operates on non-relations (sets of tuples)");
    }

    std::vector<TypeNode> tupleTypes = firstRelType[0].getTupleTypes();
    if(tupleTypes.size() != 2) {
      throw TypeCheckingExceptionPrivate(n, " JoinImage operates on a non-binary relation");
    }
    TypeNode valType = n[1].getType(check);
    if (valType != nodeManager->integerType()) {
      throw TypeCheckingExceptionPrivate(
          n, " JoinImage cardinality constraint must be integer");
    }
    if (n[1].getKind() != kind::CONST_RATIONAL) {
      throw TypeCheckingExceptionPrivate(
          n, " JoinImage cardinality constraint must be a constant");
    }
    CVC4::Rational r(INT_MAX);
    if (n[1].getConst<Rational>() > r) {
      throw TypeCheckingExceptionPrivate(
          n, " JoinImage Exceeded INT_MAX in cardinality constraint");
    }
    if (n[1].getConst<Rational>().getNumerator().getSignedInt() < 0) {
      throw TypeCheckingExceptionPrivate(
          n, " JoinImage cardinality constraint must be non-negative");
    }
    std::vector<TypeNode> newTupleTypes;
    newTupleTypes.push_back(tupleTypes[0]);
    return nodeManager->mkSetType(nodeManager->mkTupleType(newTupleTypes));
  }

  inline static bool computeIsConst(NodeManager* nodeManager, TNode n) {
    Assert(n.getKind() == kind::JOIN_IMAGE);
    return false;
  }
};/* struct JoinImageTypeRule */

struct RelIdenTypeRule {
  inline static TypeNode computeType(NodeManager* nodeManager, TNode n, bool check)
    throw (TypeCheckingExceptionPrivate, AssertionException) {
    Assert(n.getKind() == kind::IDEN);
    TypeNode setType = n[0].getType(check);
    if(check) {
      if(!setType.isSet() && !setType.getSetElementType().isTuple()) {
        throw TypeCheckingExceptionPrivate(n, " Identity operates on non-relation");
      }
      if(setType[0].getTupleTypes().size() != 1) {
        throw TypeCheckingExceptionPrivate(n, " Identity operates on non-unary relations");
      }
    }
    std::vector<TypeNode> tupleTypes = setType[0].getTupleTypes();
    tupleTypes.push_back(tupleTypes[0]);
    return nodeManager->mkSetType(nodeManager->mkTupleType(tupleTypes));
  }

  inline static bool computeIsConst(NodeManager* nodeManager, TNode n) {
      return false;
    }
};/* struct RelIdenTypeRule */

struct SetsProperties {
  inline static Cardinality computeCardinality(TypeNode type) {
    Assert(type.getKind() == kind::SET_TYPE);
    Cardinality elementCard = 2;
    elementCard ^= type[0].getCardinality();
    return elementCard;
  }

  inline static bool isWellFounded(TypeNode type) {
    return type[0].isWellFounded();
  }

  inline static Node mkGroundTerm(TypeNode type) {
    Assert(type.isSet());
    return NodeManager::currentNM()->mkConst(EmptySet(type.toType()));
  }
};/* struct SetsProperties */

}/* CVC4::theory::sets namespace */
}/* CVC4::theory namespace */
}/* CVC4 namespace */

#endif /* __CVC4__THEORY__SETS__THEORY_SETS_TYPE_RULES_H */
