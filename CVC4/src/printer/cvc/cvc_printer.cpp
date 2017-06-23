/*********************                                                        */
/*! \file cvc_printer.cpp
 ** \verbatim
 ** Top contributors (to current version):
 **   Morgan Deters, Dejan Jovanovic, Andrew Reynolds
 ** This file is part of the CVC4 project.
 ** Copyright (c) 2009-2016 by the authors listed in the file AUTHORS
 ** in the top-level source directory) and their institutional affiliations.
 ** All rights reserved.  See the file COPYING in the top-level source
 ** directory for licensing information.\endverbatim
 **
 ** \brief The pretty-printer interface for the CVC output language
 **
 ** The pretty-printer interface for the CVC output language.
 **/

#include "printer/cvc/cvc_printer.h"

#include <algorithm>
#include <iostream>
#include <iterator>
#include <stack>
#include <string>
#include <typeinfo>
#include <vector>

#include "expr/expr.h" // for ExprSetDepth etc..
#include "expr/node_manager_attributes.h" // for VarNameAttr
#include "options/language.h" // for LANG_AST
#include "printer/dagification_visitor.h"
#include "options/smt_options.h"
#include "smt/command.h"
#include "smt/smt_engine.h"
#include "smt_util/node_visitor.h"
#include "theory/arrays/theory_arrays_rewriter.h"
#include "theory/substitutions.h"
#include "theory/theory_model.h"

using namespace std;

namespace CVC4 {
namespace printer {
namespace cvc {

void CvcPrinter::toStream(std::ostream& out, TNode n, int toDepth, bool types, size_t dag) const throw() {
  if(dag != 0) {
    DagificationVisitor dv(dag);
    NodeVisitor<DagificationVisitor> visitor;
    visitor.run(dv, n);
    const theory::SubstitutionMap& lets = dv.getLets();
    if(!lets.empty()) {
      out << "LET ";
      bool first = true;
      for(theory::SubstitutionMap::const_iterator i = lets.begin();
          i != lets.end();
          ++i) {
        if(! first) {
          out << ", ";
        } else {
          first = false;
        }
        toStream(out, (*i).second, toDepth, types, false);
        out << " = ";
        toStream(out, (*i).first, toDepth, types, false);
      }
      out << " IN ";
    }
    Node body = dv.getDagifiedBody();
    toStream(out, body, toDepth, types, false);
  } else {
    toStream(out, n, toDepth, types, false);
  }
}

void CvcPrinter::toStream(std::ostream& out, TNode n, int depth, bool types, bool bracket) const throw() {
  if (depth == 0) {
    out << "(...)";
  } else {
    --depth;
  }

  // null
  if(n.getKind() == kind::NULL_EXPR) {
    out << "null";
    return;
  }

  // variables
  if(n.isVar()) {
    string s;
    if(n.getAttribute(expr::VarNameAttr(), s)) {
      out << s;
    } else {
      if(n.getKind() == kind::VARIABLE) {
        out << "var_";
      } else {
        out << n.getKind() << '_';
      }
      out << n.getId();
    }
    if(types) {
      // print the whole type, but not *its* type
      out << ":";
      n.getType().toStream(out, language::output::LANG_CVC4);
    }
    return;
  }
  if(n.isNullaryOp()) {
    if( n.getKind() == kind::UNIVERSE_SET ){
      out << "UNIVERSE :: " << n.getType();
    }else{
      //unknown printer
      out << n.getKind();
    }
    return;
  }

  // constants
  if(n.getMetaKind() == kind::metakind::CONSTANT) {
    switch(n.getKind()) {
    case kind::BITVECTOR_TYPE:
      out << "BITVECTOR(" << n.getConst<BitVectorSize>().size << ")";
      break;
    case kind::CONST_BITVECTOR: {
      const BitVector& bv = n.getConst<BitVector>();
      const Integer& x = bv.getValue();
      out << "0bin";
      unsigned n = bv.getSize();
      while(n-- > 0) {
        out << (x.testBit(n) ? '1' : '0');
      }
      break;
    }
    case kind::CONST_BOOLEAN:
      // the default would print "1" or "0" for bool, that's not correct
      // for our purposes
      out << (n.getConst<bool>() ? "TRUE" : "FALSE");
      break;
    case kind::CONST_RATIONAL: {
      const Rational& rat = n.getConst<Rational>();
      if(rat.isIntegral()) {
        out << rat.getNumerator();
      } else {
        out << '(' << rat.getNumerator() << '/' << rat.getDenominator() << ')';
      }
      break;
    }
    case kind::SUBRANGE_TYPE:
      out << '[' << n.getConst<SubrangeBounds>() << ']';
      break;
    case kind::SUBTYPE_TYPE:
      out << "SUBTYPE(" << n.getConst<Predicate>() << ")";
      break;
    case kind::TYPE_CONSTANT:
      switch(TypeConstant tc = n.getConst<TypeConstant>()) {
      case REAL_TYPE:
        out << "REAL";
        break;
      case INTEGER_TYPE:
        out << "INT";
        break;
      case BOOLEAN_TYPE:
        out << "BOOLEAN";
        break;
      case STRING_TYPE:
        out << "STRING";
        break;
      default:
        out << tc;
        break;
      }
      break;

    case kind::DATATYPE_TYPE: {
      const Datatype& dt = (NodeManager::currentNM()->getDatatypeForIndex( n.getConst< DatatypeIndexConstant >().getIndex() ));
      if( dt.isTuple() ){
        out << '[';
        for (unsigned i = 0; i < dt[0].getNumArgs(); ++ i) {
          if (i > 0) {
            out << ", ";
          }
          Type t = ((SelectorType)dt[0][i].getSelector().getType()).getRangeType();
          out << t;
        }
        out << ']';
      }else if( dt.isRecord() ){
        out << "[# ";
        for (unsigned i = 0; i < dt[0].getNumArgs(); ++ i) {
          if (i > 0) {
            out << ", ";
          }
          Type t = ((SelectorType)dt[0][i].getSelector().getType()).getRangeType();
          out << dt[0][i].getSelector() << ":" << t;
        }
        out << " #]";
      }else{
        out << dt.getName();
      }
    }
      break;

    case kind::EMPTYSET:
      out << "{} :: " << n.getConst<EmptySet>().getType();
      break;

    case kind::STORE_ALL: {
      const ArrayStoreAll& asa = n.getConst<ArrayStoreAll>();
      out << "ARRAY(" << asa.getType().getIndexType() << " OF "
          << asa.getType().getConstituentType() << ") : " << asa.getExpr();
      break;
    }

    default:
      // Fall back to whatever operator<< does on underlying type; we
      // might luck out and print something reasonable.
      kind::metakind::NodeValueConstPrinter::toStream(out, n);
    }

    return;
  }

  enum OpType {
    PREFIX,
    INFIX,
    POSTFIX
  } opType;

  //The default operation type is PREFIX
  opType = PREFIX;

  stringstream op;       // operation (such as '+')

  switch(n.getKind()) {

    // BUILTIN
    case kind::EQUAL:
      if( n[0].getType().isBoolean() ){
        op << "<=>";
      }else{
        op << '=';
      }
      opType = INFIX;
      break;
    case kind::ITE:
      out << "IF ";
      toStream(out, n[0], depth, types, true);
      out << " THEN ";
      toStream(out, n[1], depth, types, true);
      out << " ELSE ";
      toStream(out, n[2], depth, types, true);
      out << " ENDIF";
      return;
      break;
    case kind::SEXPR_TYPE:
      out << '[';
      for (unsigned i = 0; i < n.getNumChildren(); ++ i) {
        if (i > 0) {
          out << ", ";
        }
        toStream(out, n[i], depth, types, false);
      }
      out << ']';
      return;
      break;
    case kind::SEXPR:
      // no-op
      break;
    case kind::LAMBDA:
      out << "(LAMBDA";
      toStream(out, n[0], depth, types, true);
      out << ": ";
      toStream(out, n[1], depth, types, true);
      out << ")";
      return;
      break;
    case kind::APPLY:
      toStream(op, n.getOperator(), depth, types, true);
      break;
    case kind::CHAIN:
    case kind::DISTINCT: // chain and distinct not supported directly in CVC4, blast them away with the rewriter
      toStream(out, theory::Rewriter::rewrite(n), depth, types, true);
      return;
    case kind::SORT_TYPE:
    {
      string name;
      if(n.getAttribute(expr::VarNameAttr(), name)) {
        out << name;
        return;
      }
    }
    break;

    // BOOL
    case kind::AND:
      op << "AND";
      opType = INFIX;
      break;
    case kind::OR:
      op << "OR";
      opType = INFIX;
      break;
    case kind::NOT:
      op << "NOT";
      opType = PREFIX;
      break;
    case kind::XOR:
      op << "XOR";
      opType = INFIX;
      break;
    case kind::IMPLIES:
      op << "=>";
      opType = INFIX;
      break;

    // UF
    case kind::APPLY_UF:
      toStream(op, n.getOperator(), depth, types, false);
      break;
    case kind::CARDINALITY_CONSTRAINT:
    case kind::COMBINED_CARDINALITY_CONSTRAINT:
      out << "CARDINALITY_CONSTRAINT";
      break;

    case kind::FUNCTION_TYPE:
      if (n.getNumChildren() > 1) {
        if (n.getNumChildren() > 2) {
          out << '(';
        }
        for (unsigned i = 1; i < n.getNumChildren(); ++i) {
          if (i > 1) {
            out << ", ";
          }
          toStream(out, n[i - 1], depth, types, false);
        }
        if (n.getNumChildren() > 2) {
          out << ')';
        }
      }
      out << " -> ";
      toStream(out, n[n.getNumChildren() - 1], depth, types, false);
      return;
      break;

    // DATATYPES
    case kind::PARAMETRIC_DATATYPE: {
        const Datatype & dt = (NodeManager::currentNM()->getDatatypeForIndex( n[0].getConst< DatatypeIndexConstant >().getIndex() ));
        out << dt.getName() << '[';
        for(unsigned i = 1; i < n.getNumChildren(); ++i) {
          if(i > 1) {
            out << ',';
          }
          out << n[i];
        }
        out << ']';
      }
      return;
      break;
    case kind::APPLY_TYPE_ASCRIPTION: {
        toStream(out, n[0], depth, types, false);
        out << "::";
        TypeNode t = TypeNode::fromType(n.getOperator().getConst<AscriptionType>().getType());
        out << (t.isFunctionLike() ? t.getRangeType() : t);
      }
      return;
      break;
    case kind::APPLY_CONSTRUCTOR: {
        TypeNode t = n.getType();
        if( t.isTuple() ){
          if( n.getNumChildren()==1 ){
            out << "TUPLE";
          }
        }else if( t.isRecord() ){
          const Record& rec = t.getRecord();
          out << "(# ";
          TNode::iterator i = n.begin();
          bool first = true;
          const Record::FieldVector& fields = rec.getFields();
          for(Record::FieldVector::const_iterator j = fields.begin(); j != fields.end(); ++i, ++j) {
            if(!first) {
              out << ", ";
            }
            out << (*j).first << " := ";
            toStream(out, *i, depth, types, false);
            first = false;
          }
          out << " #)";
          return;
        }else{
          toStream(op, n.getOperator(), depth, types, false);
        }
      }
      break;
    case kind::APPLY_SELECTOR:
    case kind::APPLY_SELECTOR_TOTAL: {
        TypeNode t = n.getType();
        if( t.isTuple() ){
          toStream(out, n[0], depth, types, true);
          out << '.' << Datatype::indexOf( n.getOperator().toExpr() );
        }else if( t.isRecord() ){
          toStream(out, n[0], depth, types, true);
          const Record& rec = t.getRecord();
          unsigned index = Datatype::indexOf( n.getOperator().toExpr() );
          std::pair<std::string, Type> fld = rec[index];
          out << '.' << fld.first;
        }else{
          toStream(op, n.getOperator(), depth, types, false);
        }
      }
      break;
    case kind::APPLY_TESTER: {
      Assert( !n.getType().isTuple() && !n.getType().isRecord() );
      op << "is_";
      unsigned cindex = Datatype::indexOf(n.getOperator().toExpr());
      const Datatype& dt = Datatype::datatypeOf(n.getOperator().toExpr());
      toStream(op, Node::fromExpr(dt[cindex].getConstructor()), depth, types, false);
    }
      break;
    case kind::CONSTRUCTOR_TYPE:
    case kind::SELECTOR_TYPE:
      if(n.getNumChildren() > 1) {
        if(n.getNumChildren() > 2) {
          out << '(';
        }
        for(unsigned i = 0; i < n.getNumChildren() - 1; ++i) {
          if(i > 0) {
            out << ", ";
          }
          toStream(out, n[i], depth, types, false);
        }
        if(n.getNumChildren() > 2) {
          out << ')';
        }
        out << " -> ";
      }
      toStream(out, n[n.getNumChildren() - 1], depth, types, false);
      return;
    case kind::TESTER_TYPE:
      toStream(out, n[0], depth, types, false);
      out << " -> BOOLEAN";
      return;
      break;
    case kind::TUPLE_UPDATE:
      toStream(out, n[0], depth, types, true);
      out << " WITH ." << n.getOperator().getConst<TupleUpdate>().getIndex() << " := ";
      toStream(out, n[1], depth, types, true);
      return;
      break;
    case kind::RECORD_UPDATE:
      toStream(out, n[0], depth, types, true);
      out << " WITH ." << n.getOperator().getConst<RecordUpdate>().getField() << " := ";
      toStream(out, n[1], depth, types, true);
      return;
      break;

    // ARRAYS
    case kind::ARRAY_TYPE:
      out << "ARRAY ";
      toStream(out, n[0], depth, types, false);
      out << " OF ";
      toStream(out, n[1], depth, types, false);
      return;
      break;
    case kind::SELECT:
      toStream(out, n[0], depth, types, true);
      out << '[';
      toStream(out, n[1], depth, types, false);
      out << ']';
      return;
      break;
    case kind::STORE: {
      stack<TNode> stk;
      stk.push(n);
      while(stk.top()[0].getKind() == kind::STORE) {
        stk.push(stk.top()[0]);
      }
      if (bracket) {
        out << '(';
      }
      TNode x = stk.top();
      toStream(out, x[0], depth, types, false);
      out << " WITH [";
      toStream(out, x[1], depth, types, false);
      out << "] := ";
      toStream(out, x[2], depth, types, false);
      stk.pop();
      while(!stk.empty()) {
        x = stk.top();
        out << ", [";
        toStream(out, x[1], depth, types, false);
        out << "] := ";
        toStream(out, x[2], depth, types, false);
        stk.pop();
      }
      if (bracket) {
        out << ')';
      }
      return;
      break;
    }

    // ARITHMETIC
    case kind::PLUS:
      op << '+';
      opType = INFIX;
      break;
    case kind::MULT:
    case kind::NONLINEAR_MULT:
      op << '*';
      opType = INFIX;
      break;
    case kind::MINUS:
      op << '-';
      opType = INFIX;
      break;
    case kind::UMINUS:
      op << '-';
      opType = PREFIX;
      break;
    case kind::DIVISION:
    case kind::DIVISION_TOTAL:
      op << '/';
      opType = INFIX;
      break;
    case kind::INTS_DIVISION:
    case kind::INTS_DIVISION_TOTAL:
      op << "DIV";
      opType = INFIX;
      break;
    case kind::INTS_MODULUS:
    case kind::INTS_MODULUS_TOTAL:
      op << "MOD";
      opType = INFIX;
      break;
    case kind::LT:
      op << '<';
      opType = INFIX;
      break;
    case kind::LEQ:
      op << "<=";
      opType = INFIX;
      break;
    case kind::GT:
      op << '>';
      opType = INFIX;
      break;
    case kind::GEQ:
      op << ">=";
      opType = INFIX;
      break;
    case kind::POW:
      op << '^';
      opType = INFIX;
      break;
    case kind::ABS:
      op << "ABS";
      opType = PREFIX;
      break;
    case kind::IS_INTEGER:
      op << "IS_INTEGER";
      opType = PREFIX;
      break;
    case kind::TO_INTEGER:
      op << "FLOOR";
      opType = PREFIX;
      break;
    case kind::TO_REAL:
      // ignore, there is no to-real in CVC language
      toStream(out, n[0], depth, types, false);
      return;
    case kind::DIVISIBLE:
      out << "DIVISIBLE(";
      toStream(out, n[0], depth, types, false);
      out << ", " << n.getOperator().getConst<Divisible>().k << ")";
      return;

    // BITVECTORS
    case kind::BITVECTOR_XOR:
      op << "BVXOR";
      break;
    case kind::BITVECTOR_NAND:
      op << "BVNAND";
      break;
    case kind::BITVECTOR_NOR:
      op << "BVNOR";
      break;
    case kind::BITVECTOR_XNOR:
      op << "BVXNOR";
      break;
    case kind::BITVECTOR_COMP:
      op << "BVCOMP";
      break;
    case kind::BITVECTOR_UDIV:
      op << "BVUDIV";
      break;
    case kind::BITVECTOR_UDIV_TOTAL:
      op << "BVUDIV_TOTAL";
      break;
    case kind::BITVECTOR_UREM:
      op << "BVUREM";
      break;
    case kind::BITVECTOR_UREM_TOTAL:
      op << "BVUREM_TOTAL";
      break;
    case kind::BITVECTOR_SDIV:
      op << "BVSDIV";
      break;
    case kind::BITVECTOR_SREM:
      op << "BVSREM";
      break;
    case kind::BITVECTOR_SMOD:
      op << "BVSMOD";
      break;
    case kind::BITVECTOR_SHL:
      op << "BVSHL";
      break;
    case kind::BITVECTOR_LSHR:
      op << "BVLSHR";
      break;
    case kind::BITVECTOR_ASHR:
      op << "BVASHR";
      break;
    case kind::BITVECTOR_ULT:
      op << "BVLT";
      break;
    case kind::BITVECTOR_ULE:
      op << "BVLE";
      break;
    case kind::BITVECTOR_UGT:
      op << "BVGT";
      break;
    case kind::BITVECTOR_UGE:
      op << "BVGE";
      break;
    case kind::BITVECTOR_SLT:
      op << "BVSLT";
      break;
    case kind::BITVECTOR_SLE:
      op << "BVSLE";
      break;
    case kind::BITVECTOR_SGT:
      op << "BVSGT";
      break;
    case kind::BITVECTOR_SGE:
      op << "BVSGE";
      break;
    case kind::BITVECTOR_NEG:
      op << "BVUMINUS";
      break;
    case kind::BITVECTOR_NOT:
      op << "~";
      break;
    case kind::BITVECTOR_AND:
      op << "&";
      opType = INFIX;
      break;
    case kind::BITVECTOR_OR:
      op << "|";
      opType = INFIX;
      break;
    case kind::BITVECTOR_CONCAT:
      op << "@";
      opType = INFIX;
      break;
    case kind::BITVECTOR_PLUS: {
      // This interprets a BITVECTOR_PLUS as a bvadd in SMT-LIB
      Assert(n.getType().isBitVector());
      unsigned numc = n.getNumChildren()-2;
      unsigned child = 0;
      while (child < numc) {
        out << "BVPLUS(";
        out << BitVectorType(n.getType().toType()).getSize();
        out << ',';
        toStream(out, n[child], depth, types, false);
        out << ',';
        ++child;
      }
      out << "BVPLUS(";
      out << BitVectorType(n.getType().toType()).getSize();
      out << ',';
      toStream(out, n[child], depth, types, false);
      out << ',';
      toStream(out, n[child + 1], depth, types, false);
      while (child > 0) {
        out << ')';
        --child;
      }
      out << ')';
      return;
      break;
    }
    case kind::BITVECTOR_SUB:
      out << "BVSUB(";
      Assert(n.getType().isBitVector());
      out << BitVectorType(n.getType().toType()).getSize();
      out << ',';
      toStream(out, n[0], depth, types, false);
      out << ',';
      toStream(out, n[1], depth, types, false);
      out << ')';
      return;
      break;
    case kind::BITVECTOR_MULT: {
      Assert(n.getType().isBitVector());
      unsigned numc = n.getNumChildren()-2;
      unsigned child = 0;
      while (child < numc) {
        out << "BVMULT(";
        out << BitVectorType(n.getType().toType()).getSize();
        out << ',';
        toStream(out, n[child], depth, types, false);
        out << ',';
        ++child;
        }
      out << "BVMULT(";
      out << BitVectorType(n.getType().toType()).getSize();
      out << ',';
      toStream(out, n[child], depth, types, false);
      out << ',';
      toStream(out, n[child + 1], depth, types, false);
      while (child > 0) {
        out << ')';
        --child;
      }
      out << ')';
      return;
      break;
    }
    case kind::BITVECTOR_EXTRACT:
      op << n.getOperator().getConst<BitVectorExtract>();
      opType = POSTFIX;
      break;
    case kind::BITVECTOR_BITOF:
      op << n.getOperator().getConst<BitVectorBitOf>();
      opType = POSTFIX;
      break;
    case kind::BITVECTOR_REPEAT:
      out << "BVREPEAT(";
      toStream(out, n[0], depth, types, false);
      out << ", " << n.getOperator().getConst<BitVectorRepeat>() << ')';
      return;
      break;
    case kind::BITVECTOR_ZERO_EXTEND:
      out << "BVZEROEXTEND(";
      toStream(out, n[0], depth, types, false);
      out << ", " << n.getOperator().getConst<BitVectorZeroExtend>() << ')';
      return;
      break;
    case kind::BITVECTOR_SIGN_EXTEND:
      out << "SX(";
      toStream(out, n[0], depth, types, false);
      out << ", " << BitVectorType(n.getType().toType()).getSize() << ')';
      return;
      break;
    case kind::BITVECTOR_ROTATE_LEFT:
      out << "BVROTL(";
      toStream(out, n[0], depth, types, false);
      out << ", " << n.getOperator().getConst<BitVectorRotateLeft>() << ')';
      return;
      break;
    case kind::BITVECTOR_ROTATE_RIGHT:
      out << "BVROTR(";
      toStream(out, n[0], depth, types, false);
      out << ", " << n.getOperator().getConst<BitVectorRotateRight>() << ')';
      return;
      break;

    // SETS
    case kind::SET_TYPE:
      out << "SET OF ";
      toStream(out, n[0], depth, types, false);
      return;
      break;
    case kind::UNION:
      op << '|';
      opType = INFIX;
      break;
    case kind::INTERSECTION:
      op << '&';
      opType = INFIX;
      break;
    case kind::SETMINUS:
      op << '-';
      opType = INFIX;
      break;
    case kind::SUBSET:
      op << "<=";
      opType = INFIX;
      break;
    case kind::MEMBER:
      op << "IS_IN";
      opType = INFIX;
      break;
    case kind::COMPLEMENT:
      op << "~";
      opType = PREFIX;
      break;
    case kind::PRODUCT:
      op << "PRODUCT";
      opType = INFIX;
      break;
    case kind::JOIN:
      op << "JOIN";
      opType = INFIX;
      break;
    case kind::TRANSPOSE:
      op << "TRANSPOSE";
      opType = PREFIX;
      break;
    case kind::TCLOSURE:
      op << "TCLOSURE";
      opType = PREFIX;
      break;
    case kind::IDEN:
      op << "IDEN";
      opType = PREFIX;
      break;
    case kind::JOIN_IMAGE:
      op << "JOIN_IMAGE";
      opType = INFIX;
      break;
    case kind::SINGLETON:
      out << "{";
      toStream(out, n[0], depth, types, false);
      out << "}";
      return;
      break;
    case kind::INSERT: {
      if(bracket) {
        out << '(';
      }
      out << '{';
      size_t i = 0;
      toStream(out, n[i++], depth, types, false);
      for(;i+1 < n.getNumChildren(); ++i) {
        out << ", ";
        toStream(out, n[i], depth, types, false);
      }
      out << "} | ";
      toStream(out, n[i], depth, types, true);
      if(bracket) {
        out << ')';
      }
      return;
      break;
    }
    case kind::CARD: {
      out << "CARD(";
      toStream(out, n[0], depth, types, false);
      out << ")";
      return;
      break;
    }

    // Quantifiers
    case kind::FORALL:
      out << "(FORALL";
      toStream(out, n[0], depth, types, false);
      out << " : ";
      toStream(out, n[1], depth, types, false);
      out << ')';
      // TODO: user patterns?
      return;
    case kind::EXISTS:
      out << "(EXISTS";
      toStream(out, n[0], depth, types, false);
      out << " : ";
      toStream(out, n[1], depth, types, false);
      out << ')';
      // TODO: user patterns?
      return;
    case kind::INST_CONSTANT:
      out << "INST_CONSTANT";
      break;
    case kind::BOUND_VAR_LIST:
      out << '(';
      for(size_t i = 0; i < n.getNumChildren(); ++i) {
        if(i > 0) {
          out << ", ";
        }
        toStream(out, n[i], -1, true, false); // ascribe types
      }
      out << ')';
      return;
    case kind::INST_PATTERN:
      out << "INST_PATTERN";
      break;
    case kind::INST_PATTERN_LIST:
      out << "INST_PATTERN_LIST";
      break;

    default:
      Warning() << "Kind printing not implemented for the case of " << n.getKind() << endl;
      break;
  }

  switch (opType) {
  case PREFIX:
    out << op.str() << '(';
    break;
  case INFIX:
    if (bracket) {
      out << '(';
    }
    break;
  case POSTFIX:
    out << '(';
    break;
  }

  for (unsigned i = 0; i < n.getNumChildren(); ++ i) {
    if (i > 0) {
      if (opType == INFIX) {
        out << ' ' << op.str() << ' ';
      } else {
        out << ", ";
      }
    }
    toStream(out, n[i], depth, types, opType == INFIX);
  }

  switch (opType) {
    case PREFIX:
      out << ')';
      break;
    case INFIX:
      if (bracket) {
        out << ')';
      }
      break;
    case POSTFIX:
      out << ')' << op.str();
      break;
  }

}/* CvcPrinter::toStream(TNode) */

template <class T>
static bool tryToStream(std::ostream& out, const Command* c, bool cvc3Mode) throw();

void CvcPrinter::toStream(std::ostream& out, const Command* c,
                           int toDepth, bool types, size_t dag) const throw() {
  expr::ExprSetDepth::Scope sdScope(out, toDepth);
  expr::ExprPrintTypes::Scope ptScope(out, types);
  expr::ExprDag::Scope dagScope(out, dag);

  if(tryToStream<AssertCommand>(out, c, d_cvc3Mode) ||
     tryToStream<PushCommand>(out, c, d_cvc3Mode) ||
     tryToStream<PopCommand>(out, c, d_cvc3Mode) ||
     tryToStream<CheckSatCommand>(out, c, d_cvc3Mode) ||
     tryToStream<QueryCommand>(out, c, d_cvc3Mode) ||
     tryToStream<ResetCommand>(out, c, d_cvc3Mode) ||
     tryToStream<ResetAssertionsCommand>(out, c, d_cvc3Mode) ||
     tryToStream<QuitCommand>(out, c, d_cvc3Mode) ||
     tryToStream<DeclarationSequence>(out, c, d_cvc3Mode) ||
     tryToStream<CommandSequence>(out, c, d_cvc3Mode) ||
     tryToStream<DeclareFunctionCommand>(out, c, d_cvc3Mode) ||
     tryToStream<DeclareTypeCommand>(out, c, d_cvc3Mode) ||
     tryToStream<DefineTypeCommand>(out, c, d_cvc3Mode) ||
     tryToStream<DefineNamedFunctionCommand>(out, c, d_cvc3Mode) ||
     tryToStream<DefineFunctionCommand>(out, c, d_cvc3Mode) ||
     tryToStream<SimplifyCommand>(out, c, d_cvc3Mode) ||
     tryToStream<GetValueCommand>(out, c, d_cvc3Mode) ||
     tryToStream<GetModelCommand>(out, c, d_cvc3Mode) ||
     tryToStream<GetAssignmentCommand>(out, c, d_cvc3Mode) ||
     tryToStream<GetAssertionsCommand>(out, c, d_cvc3Mode) ||
     tryToStream<GetProofCommand>(out, c, d_cvc3Mode) ||
     tryToStream<GetUnsatCoreCommand>(out, c, d_cvc3Mode) ||
     tryToStream<SetBenchmarkStatusCommand>(out, c, d_cvc3Mode) ||
     tryToStream<SetBenchmarkLogicCommand>(out, c, d_cvc3Mode) ||
     tryToStream<SetInfoCommand>(out, c, d_cvc3Mode) ||
     tryToStream<GetInfoCommand>(out, c, d_cvc3Mode) ||
     tryToStream<SetOptionCommand>(out, c, d_cvc3Mode) ||
     tryToStream<GetOptionCommand>(out, c, d_cvc3Mode) ||
     tryToStream<DatatypeDeclarationCommand>(out, c, d_cvc3Mode) ||
     tryToStream<CommentCommand>(out, c, d_cvc3Mode) ||
     tryToStream<EmptyCommand>(out, c, d_cvc3Mode) ||
     tryToStream<EchoCommand>(out, c, d_cvc3Mode)) {
    return;
  }

  out << "ERROR: don't know how to print a Command of class: "
      << typeid(*c).name() << endl;

}/* CvcPrinter::toStream(Command*) */

template <class T>
static bool tryToStream(std::ostream& out, const CommandStatus* s, bool cvc3Mode) throw();

void CvcPrinter::toStream(std::ostream& out, const CommandStatus* s) const throw() {

  if(tryToStream<CommandSuccess>(out, s, d_cvc3Mode) ||
     tryToStream<CommandFailure>(out, s, d_cvc3Mode) ||
     tryToStream<CommandUnsupported>(out, s, d_cvc3Mode) ||
     tryToStream<CommandInterrupted>(out, s, d_cvc3Mode)) {
    return;
  }

  out << "ERROR: don't know how to print a CommandStatus of class: "
      << typeid(*s).name() << endl;

}/* CvcPrinter::toStream(CommandStatus*) */

void CvcPrinter::toStream(std::ostream& out, const Model& m, const Command* c) const throw() {
  const theory::TheoryModel& tm = (const theory::TheoryModel&) m;
  if(dynamic_cast<const DeclareTypeCommand*>(c) != NULL) {
    TypeNode tn = TypeNode::fromType( ((const DeclareTypeCommand*)c)->getType() );
    if( options::modelUninterpDtEnum() && tn.isSort() &&
        tm.d_rep_set.d_type_reps.find( tn )!=tm.d_rep_set.d_type_reps.end() ){
      out << "DATATYPE" << std::endl;
      out << "  " << dynamic_cast<const DeclareTypeCommand*>(c)->getSymbol() << " = ";
      for( size_t i=0; i<(*tm.d_rep_set.d_type_reps.find(tn)).second.size(); i++ ){
        if (i>0) {
          out << "| ";
        }
        out << (*tm.d_rep_set.d_type_reps.find(tn)).second[i] << " ";
      }
      out << std::endl << "END;" << std::endl;
    } else {
      if( tn.isSort() ){
        // print the cardinality
        if( tm.d_rep_set.d_type_reps.find( tn )!=tm.d_rep_set.d_type_reps.end() ){
          out << "% cardinality of " << tn << " is " << (*tm.d_rep_set.d_type_reps.find(tn)).second.size() << std::endl;
        }
      }
      out << c << std::endl;
      if( tn.isSort() ){
        // print the representatives
        if( tm.d_rep_set.d_type_reps.find( tn )!=tm.d_rep_set.d_type_reps.end() ){
          for( size_t i=0; i<(*tm.d_rep_set.d_type_reps.find(tn)).second.size(); i++ ){
            if( (*tm.d_rep_set.d_type_reps.find(tn)).second[i].isVar() ){
              out << (*tm.d_rep_set.d_type_reps.find(tn)).second[i] << " : " << tn << ";" << std::endl;
            }else{
              out << "% rep: " << (*tm.d_rep_set.d_type_reps.find(tn)).second[i] << std::endl;
            }
          }
        }
      }
    }
  } else if(dynamic_cast<const DeclareFunctionCommand*>(c) != NULL) {
    Node n = Node::fromExpr( ((const DeclareFunctionCommand*)c)->getFunction() );
    if(n.getKind() == kind::SKOLEM) {
      // don't print out internal stuff
      return;
    }
    TypeNode tn = n.getType();
    out << n << " : ";
    if( tn.isFunction() || tn.isPredicate() ){
      out << "(";
      for( size_t i=0; i<tn.getNumChildren()-1; i++ ){
        if( i>0 ) out << ", ";
        out << tn[i];
      }
      out << ") -> " << tn.getRangeType();
    }else{
      out << tn;
    }
    Node val = Node::fromExpr(tm.getSmtEngine()->getValue(n.toExpr()));
    if( options::modelUninterpDtEnum() && val.getKind() == kind::STORE ) {
      TypeNode tn = val[1].getType();
      if (tn.isSort() && tm.d_rep_set.d_type_reps.find( tn )!=tm.d_rep_set.d_type_reps.end() ){
        Cardinality indexCard((*tm.d_rep_set.d_type_reps.find(tn)).second.size());
        val = theory::arrays::TheoryArraysRewriter::normalizeConstant( val, indexCard );
      }
    }
    out << " = " << val << ";" << std::endl;

/*
    //for table format (work in progress)
    bool printedModel = false;
    if( tn.isFunction() ){
      if( options::modelFormatMode()==MODEL_FORMAT_MODE_TABLE ){
        //specialized table format for functions
        RepSetIterator riter( &d_rep_set );
        riter.setFunctionDomain( n );
        while( !riter.isFinished() ){
          std::vector< Node > children;
          children.push_back( n );
          for( int i=0; i<riter.getNumTerms(); i++ ){
            children.push_back( riter.getTerm( i ) );
          }
          Node nn = NodeManager::currentNM()->mkNode( APPLY_UF, children );
          Node val = getValue( nn );
          out << val << " ";
          riter.increment();
        }
        printedModel = true;
      }
    }
*/
  }else{
    out << c << std::endl;
  }
}

static void toStream(std::ostream& out, const AssertCommand* c, bool cvc3Mode) throw() {
  out << "ASSERT " << c->getExpr() << ";";
}

static void toStream(std::ostream& out, const PushCommand* c, bool cvc3Mode) throw() {
  out << "PUSH;";
}

static void toStream(std::ostream& out, const PopCommand* c, bool cvc3Mode) throw() {
  out << "POP;";
}

static void toStream(std::ostream& out, const CheckSatCommand* c, bool cvc3Mode) throw() {
  Expr e = c->getExpr();
  if(cvc3Mode) {
    out << "PUSH; ";
  }
  if(!e.isNull()) {
    out << "CHECKSAT " << e << ";";
  } else {
    out << "CHECKSAT;";
  }
  if(cvc3Mode) {
    out << " POP;";
  }
}

static void toStream(std::ostream& out, const QueryCommand* c, bool cvc3Mode) throw() {
  Expr e = c->getExpr();
  if(cvc3Mode) {
    out << "PUSH; ";
  }
  if(!e.isNull()) {
    out << "QUERY " << e << ";";
  } else {
    out << "QUERY TRUE;";
  }
  if(cvc3Mode) {
    out << " POP;";
  }
}

static void toStream(std::ostream& out, const ResetCommand* c, bool cvc3Mode) throw() {
  out << "RESET;";
}

static void toStream(std::ostream& out, const ResetAssertionsCommand* c, bool cvc3Mode) throw() {
  out << "RESET ASSERTIONS;";
}

static void toStream(std::ostream& out, const QuitCommand* c, bool cvc3Mode) throw() {
  //out << "EXIT;";
}

static void toStream(std::ostream& out, const CommandSequence* c, bool cvc3Mode) throw() {
  for(CommandSequence::const_iterator i = c->begin();
      i != c->end();
      ++i) {
    out << *i << endl;
  }
}

static void toStream(std::ostream& out, const DeclarationSequence* c, bool cvc3Mode) throw() {
  DeclarationSequence::const_iterator i = c->begin();
  for(;;) {
    DeclarationDefinitionCommand* dd =
      static_cast<DeclarationDefinitionCommand*>(*i++);
    if(i != c->end()) {
      out << dd->getSymbol() << ", ";
    } else {
      out << *dd;
      break;
    }
  }
}

static void toStream(std::ostream& out, const DeclareFunctionCommand* c, bool cvc3Mode) throw() {
  out << c->getSymbol() << " : " << c->getType() << ";";
}

static void toStream(std::ostream& out, const DefineFunctionCommand* c, bool cvc3Mode) throw() {
  Expr func = c->getFunction();
  const vector<Expr>& formals = c->getFormals();
  Expr formula = c->getFormula();
  out << func << " : " << func.getType() << " = ";
  if(formals.size() > 0) {
    out << "LAMBDA(";
    vector<Expr>::const_iterator i = formals.begin();
    while(i != formals.end()) {
      out << (*i) << ":" << (*i).getType();
      if(++i != formals.end()) {
        out << ", ";
      }
    }
    out << "): ";
  }
  out << formula << ";";
}

static void toStream(std::ostream& out, const DeclareTypeCommand* c, bool cvc3Mode) throw() {
  if(c->getArity() > 0) {
    //TODO?
    out << "ERROR: Don't know how to print parameterized type declaration "
           "in CVC language." << endl;
  } else {
    out << c->getSymbol() << " : TYPE;";
  }
}

static void toStream(std::ostream& out, const DefineTypeCommand* c, bool cvc3Mode) throw() {
  if(c->getParameters().size() > 0) {
    out << "ERROR: Don't know how to print parameterized type definition "
           "in CVC language:" << endl << c->toString() << endl;
  } else {
    out << c->getSymbol() << " : TYPE = " << c->getType() << ";";
  }
}

static void toStream(std::ostream& out, const DefineNamedFunctionCommand* c, bool cvc3Mode) throw() {
  toStream(out, static_cast<const DefineFunctionCommand*>(c), cvc3Mode);
}

static void toStream(std::ostream& out, const SimplifyCommand* c, bool cvc3Mode) throw() {
  out << "TRANSFORM " << c->getTerm() << ";";
}

static void toStream(std::ostream& out, const GetValueCommand* c, bool cvc3Mode) throw() {
  const vector<Expr>& terms = c->getTerms();
  Assert(!terms.empty());
  out << "GET_VALUE ";
  copy(terms.begin(), terms.end() - 1, ostream_iterator<Expr>(out, ";\nGET_VALUE "));
  out << terms.back() << ";";
}

static void toStream(std::ostream& out, const GetModelCommand* c, bool cvc3Mode) throw() {
  out << "COUNTERMODEL;";
}

static void toStream(std::ostream& out, const GetAssignmentCommand* c, bool cvc3Mode) throw() {
  out << "% (get-assignment)";
}

static void toStream(std::ostream& out, const GetAssertionsCommand* c, bool cvc3Mode) throw() {
  out << "WHERE;";
}

static void toStream(std::ostream& out, const GetProofCommand* c, bool cvc3Mode) throw() {
  out << "DUMP_PROOF;";
}

static void toStream(std::ostream& out, const GetUnsatCoreCommand* c, bool cvc3Mode) throw() {
  out << "DUMP_UNSAT_CORE;";
}

static void toStream(std::ostream& out, const SetBenchmarkStatusCommand* c, bool cvc3Mode) throw() {
  out << "% (set-info :status " << c->getStatus() << ")";
}

static void toStream(std::ostream& out, const SetBenchmarkLogicCommand* c, bool cvc3Mode) throw() {
  out << "OPTION \"logic\" \"" << c->getLogic() << "\";";
}

static void toStream(std::ostream& out, const SetInfoCommand* c, bool cvc3Mode) throw() {
  out << "% (set-info " << c->getFlag() << " ";
  OutputLanguage language =
      cvc3Mode ? language::output::LANG_CVC3 : language::output::LANG_CVC4;
  SExpr::toStream(out, c->getSExpr(), language);
  out << ")";
}

static void toStream(std::ostream& out, const GetInfoCommand* c, bool cvc3Mode) throw() {
  out << "% (get-info " << c->getFlag() << ")";
}

static void toStream(std::ostream& out, const SetOptionCommand* c, bool cvc3Mode) throw() {
  out << "OPTION \"" << c->getFlag() << "\" ";
  SExpr::toStream(out, c->getSExpr(), language::output::LANG_CVC4);
  out << ";";
}

static void toStream(std::ostream& out, const GetOptionCommand* c, bool cvc3Mode) throw() {
  out << "% (get-option " << c->getFlag() << ")";
}

static void toStream(std::ostream& out, const DatatypeDeclarationCommand* c, bool cvc3Mode) throw() {
  const vector<DatatypeType>& datatypes = c->getDatatypes();
  //do not print tuple/datatype internal declarations
  if( datatypes.size()!=1 || ( !datatypes[0].getDatatype().isTuple() && !datatypes[0].getDatatype().isRecord() ) ){
    out << "DATATYPE" << endl;
    bool firstDatatype = true;
    for(vector<DatatypeType>::const_iterator i = datatypes.begin(),
          i_end = datatypes.end();
        i != i_end;
        ++i) {
      if(! firstDatatype) {
        out << ',' << endl;
      }
      const Datatype& dt = (*i).getDatatype();
      out << "  " << dt.getName();
      if(dt.isParametric()) {
        out << '[';
        for(size_t j = 0; j < dt.getNumParameters(); ++j) {
          if(j > 0) {
            out << ',';
          }
          out << dt.getParameter(j);
        }
        out << ']';
      }
      out << " = ";
      bool firstConstructor = true;
      for(Datatype::const_iterator j = dt.begin(); j != dt.end(); ++j) {
        if(! firstConstructor) {
          out << " | ";
        }
        firstConstructor = false;
        const DatatypeConstructor& c = *j;
        out << c.getName();
        if(c.getNumArgs() > 0) {
          out << '(';
          bool firstSelector = true;
          for(DatatypeConstructor::const_iterator k = c.begin(); k != c.end(); ++k) {
            if(! firstSelector) {
              out << ", ";
            }
            firstSelector = false;
            const DatatypeConstructorArg& selector = *k;
            Type t = SelectorType(selector.getType()).getRangeType();
            if( t.isDatatype() ){
              const Datatype & sdt = ((DatatypeType)t).getDatatype();
              out << selector.getName() << ": " << sdt.getName();
            }else{
              out << selector.getName() << ": " << t;
            }
          }
          out << ')';
        }
      }
    }
    out << endl << "END;";
  }
}

static void toStream(std::ostream& out, const CommentCommand* c, bool cvc3Mode) throw() {
  out << "% " << c->getComment();
}

static void toStream(std::ostream& out, const EmptyCommand* c, bool cvc3Mode) throw() {
}

static void toStream(std::ostream& out, const EchoCommand* c, bool cvc3Mode) throw() {
  out << "ECHO \"" << c->getOutput() << "\";";
}

template <class T>
static bool tryToStream(std::ostream& out, const Command* c, bool cvc3Mode) throw() {
  if(typeid(*c) == typeid(T)) {
    toStream(out, dynamic_cast<const T*>(c), cvc3Mode);
    return true;
  }
  return false;
}

static void toStream(std::ostream& out, const CommandSuccess* s, bool cvc3Mode) throw() {
  if(Command::printsuccess::getPrintSuccess(out)) {
    out << "OK" << endl;
  }
}

static void toStream(std::ostream& out, const CommandUnsupported* s, bool cvc3Mode) throw() {
  out << "UNSUPPORTED" << endl;
}

static void toStream(std::ostream& out, const CommandInterrupted* s, bool cvc3Mode) throw() {
  out << "INTERRUPTED" << endl;
}

static void toStream(std::ostream& out, const CommandFailure* s, bool cvc3Mode) throw() {
  out << s->getMessage() << endl;
}

template <class T>
static bool tryToStream(std::ostream& out, const CommandStatus* s, bool cvc3Mode) throw() {
  if(typeid(*s) == typeid(T)) {
    toStream(out, dynamic_cast<const T*>(s), cvc3Mode);
    return true;
  }
  return false;
}

}/* CVC4::printer::cvc namespace */
}/* CVC4::printer namespace */
}/* CVC4 namespace */
