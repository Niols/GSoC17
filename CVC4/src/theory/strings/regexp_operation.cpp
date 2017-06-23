/*********************                                                        */
/*! \file regexp_operation.cpp
 ** \verbatim
 ** Top contributors (to current version):
 **   Morgan Deters, Tianyi Liang, Tim King
 ** This file is part of the CVC4 project.
 ** Copyright (c) 2009-2016 by the authors listed in the file AUTHORS
 ** in the top-level source directory) and their institutional affiliations.
 ** All rights reserved.  See the file COPYING in the top-level source
 ** directory for licensing information.\endverbatim
 **
 ** \brief Symbolic Regular Expresion Operations
 **
 ** Symbolic Regular Expresion Operations
 **/

#include "theory/strings/regexp_operation.h"

#include "expr/kind.h"
#include "options/strings_options.h"

namespace CVC4 {
namespace theory {
namespace strings {

RegExpOpr::RegExpOpr()
  : d_lastchar( options::stdASCII()? '\x7f' : '\xff' ),
    RMAXINT( LONG_MAX )
{
  d_emptyString = NodeManager::currentNM()->mkConst( ::CVC4::String("") );
  d_true = NodeManager::currentNM()->mkConst( true );
  d_false = NodeManager::currentNM()->mkConst( false );
  d_emptySingleton = NodeManager::currentNM()->mkNode( kind::STRING_TO_REGEXP, d_emptyString );
  d_zero = NodeManager::currentNM()->mkConst( ::CVC4::Rational(0) );
  d_one = NodeManager::currentNM()->mkConst( ::CVC4::Rational(1) );
  std::vector< Node > nvec;
  d_emptyRegexp = NodeManager::currentNM()->mkNode( kind::REGEXP_EMPTY, nvec );
  d_sigma = NodeManager::currentNM()->mkNode( kind::REGEXP_SIGMA, nvec );
  d_sigma_star = NodeManager::currentNM()->mkNode( kind::REGEXP_STAR, d_sigma );
}

RegExpOpr::~RegExpOpr(){ 

}

int RegExpOpr::gcd ( int a, int b ) {
  int c;
  while ( a != 0 ) {
     c = a; a = b%a;  b = c;
  }
  return b;
}

bool RegExpOpr::checkConstRegExp( Node r ) {
  Trace("strings-regexp-cstre") << "RegExp-CheckConstRegExp starts with /" << mkString( r ) << "/" << std::endl;
  bool ret = true;
  if( d_cstre_cache.find( r ) != d_cstre_cache.end() ) {
    ret = d_cstre_cache[r];
  } else {
    if(r.getKind() == kind::STRING_TO_REGEXP) {
      Node tmp = Rewriter::rewrite( r[0] );
      ret = tmp.isConst();
    } else {
      for(unsigned i=0; i<r.getNumChildren(); ++i) {
        if(!checkConstRegExp(r[i])) {
          ret = false; break;
        }
      }
    }
    d_cstre_cache[r] = ret;
  }
  return ret;
}

// 0-unknown, 1-yes, 2-no
int RegExpOpr::delta( Node r, Node &exp ) {
  Trace("regexp-delta") << "RegExp-Delta starts with /" << mkString( r ) << "/" << std::endl;
  int ret = 0;
  if( d_delta_cache.find( r ) != d_delta_cache.end() ) {
    ret = d_delta_cache[r].first;
    exp = d_delta_cache[r].second;
  } else {
    int k = r.getKind();
    switch( k ) {
      case kind::REGEXP_EMPTY: {
        ret = 2;
        break;
      }
      case kind::REGEXP_SIGMA: {
        ret = 2;
        break;
      }
      case kind::STRING_TO_REGEXP: {
        Node tmp = Rewriter::rewrite(r[0]);
        if(tmp.isConst()) {
          if(tmp == d_emptyString) {
            ret = 1;
          } else {
            ret = 2;
          }
        } else {
          ret = 0;
          if(tmp.getKind() == kind::STRING_CONCAT) {
            for(unsigned i=0; i<tmp.getNumChildren(); i++) {
              if(tmp[i].isConst()) {
                ret = 2; break;
              }
            }

          }
          if(ret == 0) {
            exp = r[0].eqNode(d_emptyString);
          }
        }
        break;
      }
      case kind::REGEXP_CONCAT: {
        bool flag = false;
        std::vector< Node > vec_nodes;
        for(unsigned i=0; i<r.getNumChildren(); ++i) {
          Node exp2;
          int tmp = delta( r[i], exp2 );
          if(tmp == 2) {
            ret = 2;
            break;
          } else if(tmp == 0) {
            vec_nodes.push_back( exp2 );
            flag = true;
          }
        }
        if(ret != 2) {
          if(!flag) {
            ret = 1;
          } else {
            exp = vec_nodes.size()==1 ? vec_nodes[0] : NodeManager::currentNM()->mkNode(kind::AND, vec_nodes);
          }
        }
        break;
      }
      case kind::REGEXP_UNION: {
        bool flag = false;
        std::vector< Node > vec_nodes;
        for(unsigned i=0; i<r.getNumChildren(); ++i) {
          Node exp2;
          int tmp = delta( r[i], exp2 );
          if(tmp == 1) {
            ret = 1;
            break;
          } else if(tmp == 0) {
            vec_nodes.push_back( exp2 );
            flag = true;
          }
        }
        if(ret != 1) {
          if(!flag) {
            ret = 2;
          } else {
            exp = vec_nodes.size()==1 ? vec_nodes[0] : NodeManager::currentNM()->mkNode(kind::OR, vec_nodes);
          }
        }
        break;
      }
      case kind::REGEXP_INTER: {
        bool flag = false;
        std::vector< Node > vec_nodes;
        for(unsigned i=0; i<r.getNumChildren(); ++i) {
          Node exp2;
          int tmp = delta( r[i], exp2 );
          if(tmp == 2) {
            ret = 2;
            break;
          } else if(tmp == 0) {
            vec_nodes.push_back( exp2 );
            flag = true;
          }
        }
        if(ret != 2) {
          if(!flag) {
            ret = 1;
          } else {
            exp = vec_nodes.size()==1 ? vec_nodes[0] : NodeManager::currentNM()->mkNode(kind::AND, vec_nodes);
          }
        }
        break;
      }
      case kind::REGEXP_STAR: {
        ret = 1;
        break;
      }
      case kind::REGEXP_PLUS: {
        ret = delta( r[0], exp );
        break;
      }
      case kind::REGEXP_OPT: {
        ret = 1;
        break;
      }
      case kind::REGEXP_RANGE: {
        ret = 2;
        break;
      }
      case kind::REGEXP_LOOP: {
        if(r[1] == d_zero) {
          ret = 1;
        } else {
          ret = delta(r[0], exp);
        }
        break;
      }
      default: {
        //Trace("strings-error") << "Unsupported term: " << mkString( r ) << " in delta of RegExp." << std::endl;
        Unreachable();
      }
    }
    if(!exp.isNull()) {
      exp = Rewriter::rewrite(exp);
    }
    std::pair< int, Node > p(ret, exp);
    d_delta_cache[r] = p;
  }
  Trace("regexp-delta") << "RegExp-Delta returns : " << ret << std::endl;
  return ret;
}

// 0-unknown, 1-yes, 2-no
int RegExpOpr::derivativeS( Node r, CVC4::String c, Node &retNode ) {
  Assert( c.size() < 2 );
  Trace("regexp-derive") << "RegExp-derive starts with /" << mkString( r ) << "/, c=" << c << std::endl;

  int ret = 1;
  retNode = d_emptyRegexp;

  PairNodeStr dv = std::make_pair( r, c );
  if( d_deriv_cache.find( dv ) != d_deriv_cache.end() ) {
    retNode = d_deriv_cache[dv].first;
    ret = d_deriv_cache[dv].second;
  } else if( c.isEmptyString() ) {
    Node expNode;
    ret = delta( r, expNode );
    if(ret == 0) {
      retNode = NodeManager::currentNM()->mkNode(kind::ITE, expNode, r, d_emptyRegexp);
    } else if(ret == 1) {
      retNode = r;
    }
    std::pair< Node, int > p(retNode, ret);
    d_deriv_cache[dv] = p;
  } else {
    switch( r.getKind() ) {
      case kind::REGEXP_EMPTY: {
        ret = 2;
        break;
      }
      case kind::REGEXP_SIGMA: {
        retNode = d_emptySingleton;
        break;
      }
      case kind::REGEXP_RANGE: {
        CVC4::String a = r[0].getConst<String>();
        CVC4::String b = r[1].getConst<String>();
        retNode = (a <= c && c <= b) ? d_emptySingleton : d_emptyRegexp;
        break;
      }
      case kind::STRING_TO_REGEXP: {
        Node tmp = Rewriter::rewrite(r[0]);
        if(tmp.isConst()) {
          if(tmp == d_emptyString) {
            ret = 2;
          } else {
            if(tmp.getConst< CVC4::String >().getFirstChar() == c.getFirstChar()) {
              retNode =  NodeManager::currentNM()->mkNode( kind::STRING_TO_REGEXP,
                tmp.getConst< CVC4::String >().size() == 1 ? d_emptyString : NodeManager::currentNM()->mkConst( tmp.getConst< CVC4::String >().substr(1) ) );
            } else {
              ret = 2;
            }
          }
        } else {
          ret = 0;
          Node rest;
          if(tmp.getKind() == kind::STRING_CONCAT) {
            Node t2 = tmp[0];
            if(t2.isConst()) {
              if(t2.getConst< CVC4::String >().getFirstChar() == c.getFirstChar()) {
                Node n =  NodeManager::currentNM()->mkNode( kind::STRING_TO_REGEXP,
                  tmp.getConst< CVC4::String >().size() == 1 ? d_emptyString : NodeManager::currentNM()->mkConst( tmp.getConst< CVC4::String >().substr(1) ) );
                std::vector< Node > vec_nodes;
                vec_nodes.push_back(n);
                for(unsigned i=1; i<tmp.getNumChildren(); i++) {
                  vec_nodes.push_back(tmp[i]);
                }
                retNode = NodeManager::currentNM()->mkNode(kind::REGEXP_CONCAT, vec_nodes);
                ret = 1;
              } else {
                ret = 2;
              }
            } else {
              tmp = tmp[0];
              std::vector< Node > vec_nodes;
              for(unsigned i=1; i<tmp.getNumChildren(); i++) {
                vec_nodes.push_back(tmp[i]);
              }
              rest = NodeManager::currentNM()->mkNode(kind::REGEXP_CONCAT, vec_nodes);
            }
          }
          if(ret == 0) {
            Node sk = NodeManager::currentNM()->mkSkolem( "rsp", NodeManager::currentNM()->stringType(), "Split RegExp" );
            retNode = NodeManager::currentNM()->mkNode(kind::STRING_TO_REGEXP, sk);
            if(!rest.isNull()) {
              retNode = Rewriter::rewrite(NodeManager::currentNM()->mkNode(kind::REGEXP_CONCAT, retNode, rest));
            }
            Node exp = tmp.eqNode(NodeManager::currentNM()->mkNode(kind::STRING_CONCAT,
                        NodeManager::currentNM()->mkConst(c), sk));
            retNode = Rewriter::rewrite(NodeManager::currentNM()->mkNode(kind::ITE, exp, retNode, d_emptyRegexp));
          }
        }
        break;
      }
      case kind::REGEXP_CONCAT: {
        std::vector< Node > vec_nodes;
        std::vector< Node > delta_nodes;
        Node dnode = d_true;
        for(unsigned i=0; i<r.getNumChildren(); ++i) {
          Node dc;
          Node exp2;
          int rt = derivativeS(r[i], c, dc);
          if(rt != 2) {
            if(rt == 0) {
              ret = 0;
            }
            std::vector< Node > vec_nodes2;
            if(dc != d_emptySingleton) {
              vec_nodes2.push_back( dc );
            }
            for(unsigned j=i+1; j<r.getNumChildren(); ++j) {
              if(r[j] != d_emptySingleton) {
                vec_nodes2.push_back( r[j] );
              }
            }
            Node tmp = vec_nodes2.size()==0 ? d_emptySingleton :
              vec_nodes2.size()==1 ? vec_nodes2[0] : NodeManager::currentNM()->mkNode( kind::REGEXP_CONCAT, vec_nodes2 );
            if(dnode != d_true) {
              tmp = Rewriter::rewrite(NodeManager::currentNM()->mkNode(kind::ITE, dnode, tmp, d_emptyRegexp));
              ret = 0;
            }
            if(std::find(vec_nodes.begin(), vec_nodes.end(), tmp) == vec_nodes.end()) {
              vec_nodes.push_back( tmp );
            }
          }
          Node exp3;
          int rt2 = delta( r[i], exp3 );
          if( rt2 == 0 ) {
            dnode = Rewriter::rewrite(NodeManager::currentNM()->mkNode(kind::AND, dnode, exp3));
          } else if( rt2 == 2 ) {
            break;
          }
        }
        retNode = vec_nodes.size() == 0 ? d_emptyRegexp :
              ( vec_nodes.size()==1 ? vec_nodes[0] : NodeManager::currentNM()->mkNode( kind::REGEXP_UNION, vec_nodes ) );
        if(retNode == d_emptyRegexp) {
          ret = 2;
        }
        break;
      }
      case kind::REGEXP_UNION: {
        std::vector< Node > vec_nodes;
        for(unsigned i=0; i<r.getNumChildren(); ++i) {
          Node dc;
          int rt = derivativeS(r[i], c, dc);
          if(rt == 0) {
            ret = 0;
          }
          if(rt != 2) {
            if(std::find(vec_nodes.begin(), vec_nodes.end(), dc) == vec_nodes.end()) {
              vec_nodes.push_back( dc );
            }
          }
          //Trace("regexp-derive") << "RegExp-derive OR R[" << i << "] " << mkString(r[i]) << " returns " << mkString(dc) << std::endl;
        }
        retNode = vec_nodes.size() == 0 ? d_emptyRegexp :
              ( vec_nodes.size()==1 ? vec_nodes[0] : NodeManager::currentNM()->mkNode( kind::REGEXP_UNION, vec_nodes ) );
        if(retNode == d_emptyRegexp) {
          ret = 2;
        }
        break;
      }
      case kind::REGEXP_INTER: {
        bool flag = true;
        bool flag_sg = false;
        std::vector< Node > vec_nodes;
        for(unsigned i=0; i<r.getNumChildren(); ++i) {
          Node dc;
          int rt = derivativeS(r[i], c, dc);
          if(rt == 0) {
            ret = 0;
          } else if(rt == 2) {
            flag = false;
            break;
          }
          if(dc == d_sigma_star) {
            flag_sg = true;
          } else {
            if(std::find(vec_nodes.begin(), vec_nodes.end(), dc) == vec_nodes.end()) {
              vec_nodes.push_back( dc );
            }
          }
        }
        if(flag) {
          if(vec_nodes.size() == 0 && flag_sg) {
            retNode = d_sigma_star;
          } else {
            retNode = vec_nodes.size() == 0 ? d_emptyRegexp :
                  ( vec_nodes.size()==1 ? vec_nodes[0] : NodeManager::currentNM()->mkNode( kind::REGEXP_INTER, vec_nodes ) );
            if(retNode == d_emptyRegexp) {
              ret = 2;
            }
          }
        } else {
          retNode = d_emptyRegexp;
          ret = 2;
        }
        break;
      }
      case kind::REGEXP_STAR: {
        Node dc;
        ret = derivativeS(r[0], c, dc);
        retNode = dc==d_emptyRegexp ? dc : (dc==d_emptySingleton ? r : NodeManager::currentNM()->mkNode( kind::REGEXP_CONCAT, dc, r ));
        break;
      }
      case kind::REGEXP_LOOP: {
        if(r[1] == r[2] && r[1] == d_zero) {
          ret = 2;
          //retNode = d_emptyRegexp;
        } else {
          Node dc;
          ret = derivativeS(r[0], c, dc);
          if(dc==d_emptyRegexp) {
            unsigned l = r[1].getConst<Rational>().getNumerator().toUnsignedInt();
            unsigned u = r[2].getConst<Rational>().getNumerator().toUnsignedInt();
            Node r2 = NodeManager::currentNM()->mkNode(kind::REGEXP_LOOP, r[0], 
              NodeManager::currentNM()->mkConst(CVC4::Rational(l==0? 0 : (l-1))),
              NodeManager::currentNM()->mkConst(CVC4::Rational(u-1)));
            retNode = dc==d_emptySingleton? r2 : NodeManager::currentNM()->mkNode( kind::REGEXP_CONCAT, dc, r2 );
          } else {
            retNode = d_emptyRegexp;
          }
        }
        break;
      }
      default: {
        //Trace("strings-error") << "Unsupported term: " << mkString( r ) << " in derivative of RegExp." << std::endl;
        Unreachable();
      }
    }
    if(retNode != d_emptyRegexp) {
      retNode = Rewriter::rewrite( retNode );
    }
    std::pair< Node, int > p(retNode, ret);
    d_deriv_cache[dv] = p;
  }

  Trace("regexp-derive") << "RegExp-derive returns : /" << mkString( retNode ) << "/" << std::endl;
  return ret;
}

Node RegExpOpr::derivativeSingle( Node r, CVC4::String c ) {
  Assert( c.size() < 2 );
  Trace("regexp-derive") << "RegExp-derive starts with /" << mkString( r ) << "/, c=" << c << std::endl;
  Node retNode = d_emptyRegexp;
  PairNodeStr dv = std::make_pair( r, c );
  if( d_dv_cache.find( dv ) != d_dv_cache.end() ) {
    retNode = d_dv_cache[dv];
  } else if( c.isEmptyString() ){
    Node exp;
    int tmp = delta( r, exp );
    if(tmp == 0) {
      // TODO variable
      retNode = d_emptyRegexp;
    } else if(tmp == 1) {
      retNode = r;
    } else {
      retNode = d_emptyRegexp;
    }
  } else {
    int k = r.getKind();
    switch( k ) {
      case kind::REGEXP_EMPTY: {
        retNode = d_emptyRegexp;
        break;
      }
      case kind::REGEXP_SIGMA: {
        retNode = NodeManager::currentNM()->mkNode( kind::STRING_TO_REGEXP, d_emptyString );
        break;
      }
      case kind::REGEXP_RANGE: {
        CVC4::String a = r[0].getConst<String>();
        CVC4::String b = r[1].getConst<String>();
        retNode = (a <= c && c <= b) ? d_emptySingleton : d_emptyRegexp;
        break;
      }
      case kind::STRING_TO_REGEXP: {
        if(r[0].isConst()) {
          if(r[0] == d_emptyString) {
            retNode = d_emptyRegexp;
          } else {
            if(r[0].getConst< CVC4::String >().getFirstChar() == c.getFirstChar()) {
              retNode =  NodeManager::currentNM()->mkNode( kind::STRING_TO_REGEXP,
                r[0].getConst< CVC4::String >().size() == 1 ? d_emptyString : NodeManager::currentNM()->mkConst( r[0].getConst< CVC4::String >().substr(1) ) );
            } else {
              retNode = d_emptyRegexp;
            }
          }
        } else {
          // TODO variable
          retNode = d_emptyRegexp;
        }
        break;
      }
      case kind::REGEXP_CONCAT: {
        Node rees = NodeManager::currentNM()->mkNode( kind::STRING_TO_REGEXP, d_emptyString );
        std::vector< Node > vec_nodes;
        for(unsigned i=0; i<r.getNumChildren(); ++i) {
          Node dc = derivativeSingle(r[i], c);
          if(dc != d_emptyRegexp) {
            std::vector< Node > vec_nodes2;
            if(dc != rees) {
              vec_nodes2.push_back( dc );
            }
            for(unsigned j=i+1; j<r.getNumChildren(); ++j) {
              if(r[j] != rees) {
                vec_nodes2.push_back( r[j] );
              }
            }
            Node tmp = vec_nodes2.size()==0 ? rees :
              vec_nodes2.size()==1 ? vec_nodes2[0] : NodeManager::currentNM()->mkNode( kind::REGEXP_CONCAT, vec_nodes2 );
            if(std::find(vec_nodes.begin(), vec_nodes.end(), tmp) == vec_nodes.end()) {
              vec_nodes.push_back( tmp );
            }
          }
          Node exp;
          if( delta( r[i], exp ) != 1 ) {
            break;
          }
        }
        retNode = vec_nodes.size() == 0 ? d_emptyRegexp :
              ( vec_nodes.size()==1 ? vec_nodes[0] : NodeManager::currentNM()->mkNode( kind::REGEXP_UNION, vec_nodes ) );
        break;
      }
      case kind::REGEXP_UNION: {
        std::vector< Node > vec_nodes;
        for(unsigned i=0; i<r.getNumChildren(); ++i) {
          Node dc = derivativeSingle(r[i], c);
          if(dc != d_emptyRegexp) {
            if(std::find(vec_nodes.begin(), vec_nodes.end(), dc) == vec_nodes.end()) {
              vec_nodes.push_back( dc );
            }
          }
          //Trace("regexp-derive") << "RegExp-derive OR R[" << i << "] /" << mkString(r[i]) << "/ returns /" << mkString(dc) << "/" << std::endl;
        }
        retNode = vec_nodes.size() == 0 ? d_emptyRegexp :
              ( vec_nodes.size()==1 ? vec_nodes[0] : NodeManager::currentNM()->mkNode( kind::REGEXP_UNION, vec_nodes ) );
        break;
      }
      case kind::REGEXP_INTER: {
        bool flag = true;
        bool flag_sg = false;
        std::vector< Node > vec_nodes;
        for(unsigned i=0; i<r.getNumChildren(); ++i) {
          Node dc = derivativeSingle(r[i], c);
          if(dc != d_emptyRegexp) {
            if(dc == d_sigma_star) {
              flag_sg = true;
            } else {
              if(std::find(vec_nodes.begin(), vec_nodes.end(), dc) == vec_nodes.end()) {
                vec_nodes.push_back( dc );
              }
            }
          } else {
            flag = false;
            break;
          }
        }
        if(flag) {
          if(vec_nodes.size() == 0 && flag_sg) {
            retNode = d_sigma_star;
          } else {
            retNode = vec_nodes.size() == 0 ? d_emptyRegexp :
                  ( vec_nodes.size()==1 ? vec_nodes[0] : NodeManager::currentNM()->mkNode( kind::REGEXP_INTER, vec_nodes ) );
          }
        } else {
          retNode = d_emptyRegexp;
        }
        break;
      }
      case kind::REGEXP_STAR: {
        Node dc = derivativeSingle(r[0], c);
        if(dc != d_emptyRegexp) {
          retNode = dc==d_emptySingleton? r : NodeManager::currentNM()->mkNode( kind::REGEXP_CONCAT, dc, r );
        } else {
          retNode = d_emptyRegexp;
        }
        break;
      }
      case kind::REGEXP_LOOP: {
        if(r[1] == r[2] && r[1] == d_zero) {
          retNode = d_emptyRegexp;
        } else {
          Node dc = derivativeSingle(r[0], c);
          if(dc != d_emptyRegexp) {
            unsigned l = r[1].getConst<Rational>().getNumerator().toUnsignedInt();
            unsigned u = r[2].getConst<Rational>().getNumerator().toUnsignedInt();
            Node r2 = NodeManager::currentNM()->mkNode(kind::REGEXP_LOOP, r[0], 
              NodeManager::currentNM()->mkConst(CVC4::Rational(l==0? 0 : (l-1))),
              NodeManager::currentNM()->mkConst(CVC4::Rational(u-1)));
            retNode = dc==d_emptySingleton? r2 : NodeManager::currentNM()->mkNode( kind::REGEXP_CONCAT, dc, r2 );
          } else {
            retNode = d_emptyRegexp;
          }
        }
        //Trace("regexp-derive") << "RegExp-derive : REGEXP_LOOP returns /" << mkString(retNode) << "/" << std::endl;
        break;
      }
      default: {
        Trace("strings-error") << "Unsupported term: " << mkString( r ) << " in derivative of RegExp." << std::endl;
        Unreachable();
      }
    }
    if(retNode != d_emptyRegexp) {
      retNode = Rewriter::rewrite( retNode );
    }
    d_dv_cache[dv] = retNode;
  }
  Trace("regexp-derive") << "RegExp-derive returns : /" << mkString( retNode ) << "/" << std::endl;
  return retNode;
}

//TODO:
bool RegExpOpr::guessLength( Node r, int &co ) {
  int k = r.getKind();
  switch( k ) {
    case kind::STRING_TO_REGEXP:
    {
      if(r[0].isConst()) {
        co += r[0].getConst< CVC4::String >().size();
        return true;
      } else {
        return false;
      }
    }
      break;
    case kind::REGEXP_CONCAT:
    {
      for(unsigned i=0; i<r.getNumChildren(); ++i) {
        if(!guessLength( r[i], co)) {
          return false;
        }
      }
      return true;
    }
      break;
    case kind::REGEXP_UNION:
    {
      int g_co;
      for(unsigned i=0; i<r.getNumChildren(); ++i) {
        int cop = 0;
        if(!guessLength( r[i], cop)) {
          return false;
        }
        if(i == 0) {
          g_co = cop;
        } else {
          g_co = gcd(g_co, cop);
        }
      }
      return true;
    }
      break;
    case kind::REGEXP_INTER:
    {
      int g_co;
      for(unsigned i=0; i<r.getNumChildren(); ++i) {
        int cop = 0;
        if(!guessLength( r[i], cop)) {
          return false;
        }
        if(i == 0) {
          g_co = cop;
        } else {
          g_co = gcd(g_co, cop);
        }
      }
      return true;
    }
      break;
    case kind::REGEXP_STAR:
    {
      co = 0;
      return true;
    }
      break;
    default:
      Trace("strings-error") << "Unsupported term: " << mkString( r ) << " in membership of RegExp." << std::endl;
      return false;
  }
}

void RegExpOpr::firstChars( Node r, std::set<unsigned char> &pcset, SetNodes &pvset ) {
  Trace("regexp-fset") << "Start FSET(" << mkString(r) << ")" << std::endl;
  std::map< Node, std::pair< std::set<unsigned char>, SetNodes > >::const_iterator itr = d_fset_cache.find(r);
  if(itr != d_fset_cache.end()) {
    pcset.insert((itr->second).first.begin(), (itr->second).first.end());
    pvset.insert((itr->second).second.begin(), (itr->second).second.end());
  } else {
    std::set<unsigned char> cset;
    SetNodes vset;
    int k = r.getKind();
    switch( k ) {
      case kind::REGEXP_EMPTY: {
        break;
      }
      case kind::REGEXP_SIGMA: {
        for(unsigned char i='\0'; i<=d_lastchar; i++) {
          cset.insert(i);
        }
        break;
      }
      case kind::REGEXP_RANGE: {
        unsigned char a = r[0].getConst<String>().getFirstChar();
        unsigned char b = r[1].getConst<String>().getFirstChar();
        for(unsigned char c=a; c<=b; c++) {
          cset.insert(c);
        }
        break;
      }
      case kind::STRING_TO_REGEXP: {
        Node st = Rewriter::rewrite(r[0]);
        if(st.isConst()) {
          CVC4::String s = st.getConst< CVC4::String >();
          if(s.size() != 0) {
            cset.insert(s.getFirstChar());
          }
        } else if(st.getKind() == kind::VARIABLE) {
          vset.insert( st );
        } else {
          if(st[0].isConst()) {
            CVC4::String s = st[0].getConst< CVC4::String >();
            cset.insert(s.getFirstChar());
          } else {
            vset.insert( st[0] );
          }
        }
        break;
      }
      case kind::REGEXP_CONCAT: {
        for(unsigned i=0; i<r.getNumChildren(); i++) {
          firstChars(r[i], cset, vset);
          Node n = r[i];
          Node exp;
          int r = delta( n, exp );
          if(r != 1) {
            break;
          }
        }
        break;
      }
      case kind::REGEXP_UNION: {
        for(unsigned i=0; i<r.getNumChildren(); i++) {
          firstChars(r[i], cset, vset);
        }
        break;
      }
      case kind::REGEXP_INTER: {
        //TODO: Overapproximation for now
        //for(unsigned i=0; i<r.getNumChildren(); i++) {
        // firstChars(r[i], cset, vset);
        //}
        firstChars(r[0], cset, vset);
        break;
      }
      case kind::REGEXP_STAR: {
        firstChars(r[0], cset, vset);
        break;
      }
      case kind::REGEXP_LOOP: {
        firstChars(r[0], cset, vset);
        break;
      }
      default: {
        Trace("regexp-error") << "Unsupported term: " << r << " in firstChars." << std::endl;
        Unreachable();
      }
    }
    pcset.insert(cset.begin(), cset.end());
    pvset.insert(vset.begin(), vset.end());
    std::pair< std::set<unsigned char>, SetNodes > p(cset, vset);
    d_fset_cache[r] = p;
  }

  if(Trace.isOn("regexp-fset")) {
    Trace("regexp-fset") << "END FSET(" << mkString(r) << ") = {";
    for(std::set<unsigned char>::const_iterator itr = pcset.begin();
      itr != pcset.end(); itr++) {
        if(itr != pcset.begin()) {
          Trace("regexp-fset") << ",";
        }
        Trace("regexp-fset") << (*itr);
      }
    Trace("regexp-fset") << "}" << std::endl;
  }
}

bool RegExpOpr::follow( Node r, CVC4::String c, std::vector< unsigned char > &vec_chars ) {
  int k = r.getKind();
  switch( k ) {
    case kind::STRING_TO_REGEXP:
    {
      if(r[0].isConst()) {
        if(r[0] != d_emptyString) {
          unsigned char t1 = r[0].getConst< CVC4::String >().getFirstChar();
          if(c.isEmptyString()) {
            vec_chars.push_back( t1 );
            return true;
          } else {
            unsigned char t2 = c.getFirstChar();
            if(t1 != t2) {
              return false;
            } else {
              if(c.size() >= 2) {
                vec_chars.push_back( c.substr(1,1).getFirstChar() );
              } else {
                vec_chars.push_back( '\0' );
              }
              return true;
            }
          }
        } else {
          return false;
        }
      } else {
        return false;
      }
    }
      break;
    case kind::REGEXP_CONCAT:
    {
      for(unsigned i=0; i<r.getNumChildren(); ++i) {
        if( follow(r[i], c, vec_chars) ) {
          if(vec_chars[vec_chars.size() - 1] == '\0') {
            vec_chars.pop_back();
            c = d_emptyString.getConst< CVC4::String >();
          }
        } else {
          return false;
        }
      }
      vec_chars.push_back( '\0' );
      return true;
    }
      break;
    case kind::REGEXP_UNION:
    {
      bool flag = false;
      for(unsigned i=0; i<r.getNumChildren(); ++i) {
        if( follow(r[i], c, vec_chars) ) {
          flag=true;
        }
      }
      return flag;
    }
      break;
    case kind::REGEXP_INTER:
    {
      std::vector< unsigned char > vt2;
      for(unsigned i=0; i<r.getNumChildren(); ++i) {
        std::vector< unsigned char > v_tmp;
        if( !follow(r[i], c, v_tmp) ) {
          return false;
        }
        std::vector< unsigned char > vt3(vt2);
        vt2.clear();
        std::set_intersection( vt3.begin(), vt3.end(), v_tmp.begin(), v_tmp.end(), vt2.begin() );
        if(vt2.size() == 0) {
          return false;
        }
      }
      vec_chars.insert( vec_chars.end(), vt2.begin(), vt2.end() );
      return true;
    }
      break;
    case kind::REGEXP_STAR:
    {
      if(follow(r[0], c, vec_chars)) {
        if(vec_chars[vec_chars.size() - 1] == '\0') {
          if(c.isEmptyString()) {
            return true;
          } else {
            vec_chars.pop_back();
            c = d_emptyString.getConst< CVC4::String >();
            return follow(r[0], c, vec_chars);
          }
        } else {
          return true;
        }
      } else {
        vec_chars.push_back( '\0' );
        return true;
      }
    }
      break;
    default: {
      Trace("strings-error") << "Unsupported term: " << mkString( r ) << " in follow of RegExp." << std::endl;
      //AlwaysAssert( false );
      //return Node::null();
      return false;
    }
  }
}

Node RegExpOpr::mkAllExceptOne( unsigned char exp_c ) {
  std::vector< Node > vec_nodes;
  for(unsigned char c=d_char_start; c<=d_char_end; ++c) {
    if(c != exp_c ) {
      Node n = NodeManager::currentNM()->mkNode( kind::STRING_TO_REGEXP, NodeManager::currentNM()->mkConst( ::CVC4::String( c ) ) );
      vec_nodes.push_back( n );
    }
  }
  return NodeManager::currentNM()->mkNode( kind::REGEXP_UNION, vec_nodes );
}

//simplify
void RegExpOpr::simplify(Node t, std::vector< Node > &new_nodes, bool polarity) {
  Trace("strings-regexp-simpl") << "RegExp-Simpl starts with " << t << ", polarity=" << polarity << std::endl;
  Assert(t.getKind() == kind::STRING_IN_REGEXP);
  Node str = Rewriter::rewrite(t[0]);
  Node re  = Rewriter::rewrite(t[1]);
  if(polarity) {
    simplifyPRegExp( str, re, new_nodes );
  } else {
    simplifyNRegExp( str, re, new_nodes );
  }
  Trace("strings-regexp-simpl") << "RegExp-Simpl  returns (" << new_nodes.size() << "):\n";
  for(unsigned i=0; i<new_nodes.size(); i++) {
    Trace("strings-regexp-simpl") << "\t" << new_nodes[i] << std::endl;
  }
}
void RegExpOpr::simplifyNRegExp( Node s, Node r, std::vector< Node > &new_nodes ) {
  std::pair < Node, Node > p(s, r);
  std::map < std::pair< Node, Node >, Node >::const_iterator itr = d_simpl_neg_cache.find(p);
  if(itr != d_simpl_neg_cache.end()) {
    new_nodes.push_back( itr->second );
  } else {
    int k = r.getKind();
    Node conc;
    switch( k ) {
      case kind::REGEXP_EMPTY: {
        conc = d_true;
        break;
      }
      case kind::REGEXP_SIGMA: {
        conc = d_one.eqNode(NodeManager::currentNM()->mkNode(kind::STRING_LENGTH, s)).negate();
        break;
      }
      case kind::REGEXP_RANGE: {
        std::vector< Node > vec;
        unsigned char a = r[0].getConst<String>().getFirstChar();
        unsigned char b = r[1].getConst<String>().getFirstChar();
        for(unsigned char c=a; c<=b; c++) {
          Node tmp = s.eqNode( NodeManager::currentNM()->mkConst( CVC4::String(c) ) ).negate();
          vec.push_back( tmp );
        }
        conc = vec.size()==1? vec[0] : NodeManager::currentNM()->mkNode(kind::AND, vec);
        break;
      }
      case kind::STRING_TO_REGEXP: {
        conc = s.eqNode(r[0]).negate();
        break;
      }
      case kind::REGEXP_CONCAT: {
        //TODO: rewrite empty
        Node lens = NodeManager::currentNM()->mkNode(kind::STRING_LENGTH, s);
        Node b1 = NodeManager::currentNM()->mkBoundVar(NodeManager::currentNM()->integerType());
        Node b1v = NodeManager::currentNM()->mkNode(kind::BOUND_VAR_LIST, b1);
        Node g1 = NodeManager::currentNM()->mkNode( kind::AND, NodeManager::currentNM()->mkNode(kind::GEQ, b1, d_zero),
              NodeManager::currentNM()->mkNode( kind::GEQ, NodeManager::currentNM()->mkNode(kind::STRING_LENGTH, s), b1 ) );
        Node s1 = Rewriter::rewrite(NodeManager::currentNM()->mkNode(kind::STRING_SUBSTR, s, d_zero, b1));
        Node s2 = Rewriter::rewrite(NodeManager::currentNM()->mkNode(kind::STRING_SUBSTR, s, b1, NodeManager::currentNM()->mkNode(kind::MINUS, lens, b1)));
        Node s1r1 = NodeManager::currentNM()->mkNode(kind::STRING_IN_REGEXP, s1, r[0]).negate();
        if(r[0].getKind() == kind::STRING_TO_REGEXP) {
          s1r1 = s1.eqNode(r[0][0]).negate();
        } else if(r[0].getKind() == kind::REGEXP_EMPTY) {
          s1r1 = d_true;
        }
        Node r2 = r[1];
        if(r.getNumChildren() > 2) {
          std::vector< Node > nvec;
          for(unsigned i=1; i<r.getNumChildren(); i++) {
            nvec.push_back( r[i] );
          }
          r2 = NodeManager::currentNM()->mkNode(kind::REGEXP_CONCAT, nvec);
        }
        r2 = Rewriter::rewrite(r2);
        Node s2r2 = NodeManager::currentNM()->mkNode(kind::STRING_IN_REGEXP, s2, r2).negate();
        if(r2.getKind() == kind::STRING_TO_REGEXP) {
          s2r2 = s2.eqNode(r2[0]).negate();
        } else if(r2.getKind() == kind::REGEXP_EMPTY) {
          s2r2 = d_true;
        }

        conc = NodeManager::currentNM()->mkNode(kind::OR, s1r1, s2r2);
        conc = NodeManager::currentNM()->mkNode(kind::IMPLIES, g1, conc);
        conc = NodeManager::currentNM()->mkNode(kind::FORALL, b1v, conc);
        break;
      }
      case kind::REGEXP_UNION: {
        std::vector< Node > c_and;
        for(unsigned i=0; i<r.getNumChildren(); ++i) {
          if(r[i].getKind() == kind::STRING_TO_REGEXP) {
            c_and.push_back( r[i][0].eqNode(s).negate() );
          } else if(r[i].getKind() == kind::REGEXP_EMPTY) {
            continue;
          } else {
            c_and.push_back(NodeManager::currentNM()->mkNode(kind::STRING_IN_REGEXP, s, r[i]).negate());
          }
        }
        conc = c_and.size() == 0 ? d_true :
            c_and.size() == 1 ? c_and[0] : NodeManager::currentNM()->mkNode(kind::AND, c_and);
        break;
      }
      case kind::REGEXP_INTER: {
        bool emptyflag = false;
        std::vector< Node > c_or;
        for(unsigned i=0; i<r.getNumChildren(); ++i) {
          if(r[i].getKind() == kind::STRING_TO_REGEXP) {
            c_or.push_back( r[i][0].eqNode(s).negate() );
          } else if(r[i].getKind() == kind::REGEXP_EMPTY) {
            emptyflag = true;
            break;
          } else {
            c_or.push_back(NodeManager::currentNM()->mkNode(kind::STRING_IN_REGEXP, s, r[i]).negate());
          }
        }
        if(emptyflag) {
          conc = d_true;
        } else {
          conc = c_or.size() == 1 ? c_or[0] : NodeManager::currentNM()->mkNode(kind::OR, c_or);
        }
        break;
      }
      case kind::REGEXP_STAR: {
        if(s == d_emptyString) {
          conc = d_false;
        } else if(r[0].getKind() == kind::REGEXP_EMPTY) {
          conc = s.eqNode(d_emptyString).negate();
        } else if(r[0].getKind() == kind::REGEXP_SIGMA) {
          conc = d_false;
        } else {
          Node lens = NodeManager::currentNM()->mkNode(kind::STRING_LENGTH, s);
          Node sne = s.eqNode(d_emptyString).negate();
          Node b1 = NodeManager::currentNM()->mkBoundVar(NodeManager::currentNM()->integerType());
          Node b1v = NodeManager::currentNM()->mkNode(kind::BOUND_VAR_LIST, b1);
          Node g1 = NodeManager::currentNM()->mkNode( kind::AND, NodeManager::currentNM()->mkNode(kind::GEQ, b1, d_one),
                NodeManager::currentNM()->mkNode( kind::GEQ, lens, b1 ) );
          //internal
          Node s1 = NodeManager::currentNM()->mkNode(kind::STRING_SUBSTR, s, d_zero, b1);
          Node s2 = NodeManager::currentNM()->mkNode(kind::STRING_SUBSTR, s, b1, NodeManager::currentNM()->mkNode(kind::MINUS, lens, b1));
          Node s1r1 = NodeManager::currentNM()->mkNode(kind::STRING_IN_REGEXP, s1, r[0]).negate();
          Node s2r2 = NodeManager::currentNM()->mkNode(kind::STRING_IN_REGEXP, s2, r).negate();

          conc = NodeManager::currentNM()->mkNode(kind::OR, s1r1, s2r2);
          conc = NodeManager::currentNM()->mkNode(kind::IMPLIES, g1, conc);
          conc = NodeManager::currentNM()->mkNode(kind::FORALL, b1v, conc);
          conc = NodeManager::currentNM()->mkNode(kind::AND, sne, conc);
        }
        break;
      }
      case kind::REGEXP_LOOP: {
        Assert(r.getNumChildren() == 3);
        if(r[1] == r[2]) {
          if(r[1] == d_zero) {
            conc = s.eqNode(d_emptyString).negate();
          } else if(r[1] == d_one) {
            conc = NodeManager::currentNM()->mkNode(kind::STRING_IN_REGEXP, s, r[0]).negate();
          } else {
            //unroll for now
            unsigned l = r[1].getConst<Rational>().getNumerator().toUnsignedInt();
            std::vector<Node> vec;
            for(unsigned i=0; i<l; i++) {
              vec.push_back(r[0]);
            }
            Node r2 = NodeManager::currentNM()->mkNode(kind::REGEXP_CONCAT, vec);
            conc = NodeManager::currentNM()->mkNode(kind::STRING_IN_REGEXP, s, r2).negate();
          }
        } else {
          Assert(r[1] == d_zero);
          //unroll for now
          unsigned u = r[2].getConst<Rational>().getNumerator().toUnsignedInt();
          std::vector<Node> vec;
          vec.push_back(d_emptySingleton);
          std::vector<Node> vec2;
          for(unsigned i=1; i<=u; i++) {
            vec2.push_back(r[0]);
            Node r2 = i==1? r[0] : NodeManager::currentNM()->mkNode(kind::REGEXP_CONCAT, vec);
            vec.push_back(r2);
          }
          Node r3 = NodeManager::currentNM()->mkNode(kind::REGEXP_UNION, vec);
          conc = NodeManager::currentNM()->mkNode(kind::STRING_IN_REGEXP, s, r3).negate();
        }
        break;
      }
      default: {
        Trace("strings-error") << "Unsupported term: " << r << " in simplifyNRegExp." << std::endl;
        Assert( false, "Unsupported Term" );
      }
    }
    conc = Rewriter::rewrite( conc );
    new_nodes.push_back( conc );
    d_simpl_neg_cache[p] = conc;
  }
}
void RegExpOpr::simplifyPRegExp( Node s, Node r, std::vector< Node > &new_nodes ) {
  std::pair < Node, Node > p(s, r);
  std::map < std::pair< Node, Node >, Node >::const_iterator itr = d_simpl_cache.find(p);
  if(itr != d_simpl_cache.end()) {
    new_nodes.push_back( itr->second );
  } else {
    int k = r.getKind();
    Node conc;
    switch( k ) {
      case kind::REGEXP_EMPTY: {
        conc = d_false;
        break;
      }
      case kind::REGEXP_SIGMA: {
        conc = d_one.eqNode(NodeManager::currentNM()->mkNode(kind::STRING_LENGTH, s));
        break;
      }
      case kind::REGEXP_RANGE: {
        conc = s.eqNode( r[0] );
        if(r[0] != r[1]) {
          unsigned char a = r[0].getConst<String>().getFirstChar();
          unsigned char b = r[1].getConst<String>().getFirstChar();
          a += 1;
          Node tmp = a!=b? NodeManager::currentNM()->mkNode(kind::STRING_IN_REGEXP, s,
            NodeManager::currentNM()->mkNode(kind::REGEXP_RANGE,
              NodeManager::currentNM()->mkConst( CVC4::String(a) ),
              r[1])) :
            s.eqNode(r[1]);
          conc = NodeManager::currentNM()->mkNode(kind::OR, conc, tmp);
        }
        /*
        unsigned char a = r[0].getConst<String>().getFirstChar();
        unsigned char b = r[1].getConst<String>().getFirstChar();
        std::vector< Node > vec;
        for(unsigned char c=a; c<=b; c++) {
          Node t2 = s.eqNode( NodeManager::currentNM()->mkConst( CVC4::String(c) ));
          vec.push_back( t2 );
        }
        conc = vec.empty()? d_emptySingleton : vec.size()==1? vec[0] : NodeManager::currentNM()->mkNode(kind::OR, vec);
        */
        break;
      }
      case kind::STRING_TO_REGEXP: {
        conc = s.eqNode(r[0]);
        break;
      }
      case kind::REGEXP_CONCAT: {
        std::vector< Node > nvec;
        std::vector< Node > cc;
        bool emptyflag = false;
        for(unsigned i=0; i<r.getNumChildren(); ++i) {
          if(r[i].getKind() == kind::STRING_TO_REGEXP) {
            cc.push_back( r[i][0] );
          } else if(r[i].getKind() == kind::REGEXP_EMPTY) {
            emptyflag = true;
            break;
          } else {
            Node sk = NodeManager::currentNM()->mkSkolem( "rc", s.getType(), "created for regular expression concat" );
            Node lem = NodeManager::currentNM()->mkNode(kind::STRING_IN_REGEXP, sk, r[i]);
            nvec.push_back(lem);
            cc.push_back(sk);
          }
        }
        if(emptyflag) {
          conc = d_false;
        } else {
          Node lem = s.eqNode( NodeManager::currentNM()->mkNode(kind::STRING_CONCAT, cc) );
          nvec.push_back(lem);
          conc = nvec.size() == 1 ? nvec[0] : NodeManager::currentNM()->mkNode(kind::AND, nvec);
        }
        break;
      }
      case kind::REGEXP_UNION: {
        std::vector< Node > c_or;
        for(unsigned i=0; i<r.getNumChildren(); ++i) {
          if(r[i].getKind() == kind::STRING_TO_REGEXP) {
            c_or.push_back( r[i][0].eqNode(s) );
          } else if(r[i].getKind() == kind::REGEXP_EMPTY) {
            continue;
          } else {
            c_or.push_back(NodeManager::currentNM()->mkNode(kind::STRING_IN_REGEXP, s, r[i]));
          }
        }
        conc = c_or.size() == 0 ? d_false :
            c_or.size() == 1 ? c_or[0] : NodeManager::currentNM()->mkNode(kind::OR, c_or);
        break;
      }
      case kind::REGEXP_INTER: {
        std::vector< Node > c_and;
        bool emptyflag = false;
        for(unsigned i=0; i<r.getNumChildren(); ++i) {
          if(r[i].getKind() == kind::STRING_TO_REGEXP) {
            c_and.push_back( r[i][0].eqNode(s) );
          } else if(r[i].getKind() == kind::REGEXP_EMPTY) {
            emptyflag = true;
            break;
          } else {
            c_and.push_back(NodeManager::currentNM()->mkNode(kind::STRING_IN_REGEXP, s, r[i]));
          }
        }
        if(emptyflag) {
          conc = d_false;
        } else {
          conc = c_and.size() == 1 ? c_and[0] : NodeManager::currentNM()->mkNode(kind::AND, c_and);
        }
        break;
      }
      case kind::REGEXP_STAR: {
        if(s == d_emptyString) {
          conc = d_true;
        } else if(r[0].getKind() == kind::REGEXP_EMPTY) {
          conc = s.eqNode(d_emptyString);
        } else if(r[0].getKind() == kind::REGEXP_SIGMA) {
          conc = d_true;
        } else {
          Node se = s.eqNode(d_emptyString);
          Node sinr = NodeManager::currentNM()->mkNode(kind::STRING_IN_REGEXP, s, r[0]);
          Node sk1 = NodeManager::currentNM()->mkSkolem( "rs", s.getType(), "created for regular expression star" );
          Node sk2 = NodeManager::currentNM()->mkSkolem( "rs", s.getType(), "created for regular expression star" );
          Node s1nz = sk1.eqNode(d_emptyString).negate();
          Node s2nz = sk2.eqNode(d_emptyString).negate();
          Node s1inr = NodeManager::currentNM()->mkNode(kind::STRING_IN_REGEXP, sk1, r[0]);
          Node s2inrs = NodeManager::currentNM()->mkNode(kind::STRING_IN_REGEXP, sk2, r);
          Node s12 = s.eqNode(NodeManager::currentNM()->mkNode(kind::STRING_CONCAT, sk1, sk2));

          conc = NodeManager::currentNM()->mkNode(kind::AND, s12, s1nz, s2nz, s1inr, s2inrs);
          conc = NodeManager::currentNM()->mkNode(kind::OR, se, sinr, conc);
        }
        break;
      }
      case kind::REGEXP_LOOP: {
        Assert(r.getNumChildren() == 3);
        if(r[1] == d_zero) {
          if(r[2] == d_zero) {
            conc = s.eqNode( d_emptyString );
          } else {
            //R{0,n}
            if(s != d_emptyString) {
              Node sk1 = NodeManager::currentNM()->mkSkolem( "lps", s.getType(), "created for regular expression loop" );
              Node sk2 = NodeManager::currentNM()->mkSkolem( "lps", s.getType(), "created for regular expression loop" );
              Node seq12 = s.eqNode(NodeManager::currentNM()->mkNode(kind::STRING_CONCAT, sk1, sk2));
              Node sk1ne = sk1.eqNode(d_emptyString).negate();
              Node sk1inr = NodeManager::currentNM()->mkNode(kind::STRING_IN_REGEXP, sk1, r[0]);
              unsigned u = r[2].getConst<Rational>().getNumerator().toUnsignedInt();
              Node u1 = NodeManager::currentNM()->mkConst(CVC4::Rational(u - 1));
              Node sk2inru = NodeManager::currentNM()->mkNode(kind::STRING_IN_REGEXP, sk2,
                NodeManager::currentNM()->mkNode(kind::REGEXP_LOOP, r[0], d_zero, u1));
              conc = NodeManager::currentNM()->mkNode(kind::AND, seq12, sk1ne, sk1inr, sk2inru);
              conc = NodeManager::currentNM()->mkNode(kind::OR,
                s.eqNode(d_emptyString), conc);
            } else {
              conc = d_true;
            }
          }
        } else {
          //R^n
          Node sk1 = NodeManager::currentNM()->mkSkolem( "lps", s.getType(), "created for regular expression loop" );
          Node sk2 = NodeManager::currentNM()->mkSkolem( "lps", s.getType(), "created for regular expression loop" );
          Node seq12 = s.eqNode(NodeManager::currentNM()->mkNode(kind::STRING_CONCAT, sk1, sk2));
          Node sk1ne = sk1.eqNode(d_emptyString).negate();
          Node sk1inr = NodeManager::currentNM()->mkNode(kind::STRING_IN_REGEXP, sk1, r[0]);
          unsigned u = r[2].getConst<Rational>().getNumerator().toUnsignedInt();
          Node u1 = NodeManager::currentNM()->mkConst(CVC4::Rational(u - 1));
          Node sk2inru = NodeManager::currentNM()->mkNode(kind::STRING_IN_REGEXP, sk2,
            NodeManager::currentNM()->mkNode(kind::REGEXP_LOOP, r[0], u1, u1));
          conc = NodeManager::currentNM()->mkNode(kind::AND, seq12, sk1ne, sk1inr, sk2inru);
        }
        break;
      }
      default: {
        Trace("strings-error") << "Unsupported term: " << r << " in simplifyPRegExp." << std::endl;
        Assert( false, "Unsupported Term" );
      }
    }
    conc = Rewriter::rewrite( conc );
    new_nodes.push_back( conc );
    d_simpl_cache[p] = conc;
  }
}

void RegExpOpr::getCharSet( Node r, std::set<unsigned char> &pcset, SetNodes &pvset ) {
  std::map< Node, std::pair< std::set<unsigned char>, SetNodes > >::const_iterator itr = d_cset_cache.find(r);
  if(itr != d_cset_cache.end()) {
    pcset.insert((itr->second).first.begin(), (itr->second).first.end());
    pvset.insert((itr->second).second.begin(), (itr->second).second.end());
  } else {
    std::set<unsigned char> cset;
    SetNodes vset;
    int k = r.getKind();
    switch( k ) {
      case kind::REGEXP_EMPTY: {
        break;
      }
      case kind::REGEXP_SIGMA: {
        for(unsigned char i='\0'; i<=d_lastchar; i++) {
          cset.insert(i);
        }
        break;
      }
      case kind::REGEXP_RANGE: {
        unsigned char a = r[0].getConst<String>().getFirstChar();
        unsigned char b = r[1].getConst<String>().getFirstChar();
        for(unsigned char i=a; i<=b; i++) {
          cset.insert(i);
        }
        break;
      }
      case kind::STRING_TO_REGEXP: {
        Node st = Rewriter::rewrite(r[0]);
        if(st.isConst()) {
          CVC4::String s = st.getConst< CVC4::String >();
          s.getCharSet( cset );
        } else if(st.getKind() == kind::VARIABLE) {
          vset.insert( st );
        } else {
          for(unsigned i=0; i<st.getNumChildren(); i++) {
            if(st[i].isConst()) {
              CVC4::String s = st[i].getConst< CVC4::String >();
              s.getCharSet( cset );
            } else {
              vset.insert( st[i] );
            }
          }
        }
        break;
      }
      case kind::REGEXP_CONCAT: {
        for(unsigned i=0; i<r.getNumChildren(); i++) {
          getCharSet(r[i], cset, vset);
        }
        break;
      }
      case kind::REGEXP_UNION: {
        for(unsigned i=0; i<r.getNumChildren(); i++) {
          getCharSet(r[i], cset, vset);
        }
        break;
      }
      case kind::REGEXP_INTER: {
        //TODO: Overapproximation for now
        //for(unsigned i=0; i<r.getNumChildren(); i++) {
          //getCharSet(r[i], cset, vset);
        //}
        getCharSet(r[0], cset, vset);
        break;
      }
      case kind::REGEXP_STAR: {
        getCharSet(r[0], cset, vset);
        break;
      }
      case kind::REGEXP_LOOP: {
        getCharSet(r[0], cset, vset);
        break;
      }
      default: {
        //Trace("strings-error") << "Unsupported term: " << r << " in getCharSet." << std::endl;
        Unreachable();
      }
    }
    pcset.insert(cset.begin(), cset.end());
    pvset.insert(vset.begin(), vset.end());
    std::pair< std::set<unsigned char>, SetNodes > p(cset, vset);
    d_cset_cache[r] = p;

    Trace("regexp-cset") << "CSET( " << mkString(r) << " ) = { ";
    for(std::set<unsigned char>::const_iterator itr = cset.begin();
      itr != cset.end(); itr++) {
        Trace("regexp-cset") << (*itr) << ",";
      }
    Trace("regexp-cset") << " }" << std::endl;
  }
}

bool RegExpOpr::isPairNodesInSet(std::set< PairNodes > &s, Node n1, Node n2) {
  for(std::set< PairNodes >::const_iterator itr = s.begin();
      itr != s.end(); ++itr) {
    if((itr->first == n1 && itr->second == n2) ||
       (itr->first == n2 && itr->second == n1)) {
      return true;
    }
  }
  return false;
}

bool RegExpOpr::containC2(unsigned cnt, Node n) {
  if(n.getKind() == kind::REGEXP_RV) {
    Assert(n[0].getConst<Rational>() <= RMAXINT, "Exceeded LONG_MAX in RegExpOpr::containC2");
    unsigned y = n[0].getConst<Rational>().getNumerator().toUnsignedInt();
    return cnt == y;
  } else if(n.getKind() == kind::REGEXP_CONCAT) {
    for( unsigned i=0; i<n.getNumChildren(); i++ ) {
      if(containC2(cnt, n[i])) {
        return true;
      }
    }
  } else if(n.getKind() == kind::REGEXP_STAR) {
    return containC2(cnt, n[0]);
  } else if(n.getKind() == kind::REGEXP_LOOP) {
    return containC2(cnt, n[0]);
  } else if(n.getKind() == kind::REGEXP_UNION) {
    for( unsigned i=0; i<n.getNumChildren(); i++ ) {
      if(containC2(cnt, n[i])) {
        return true;
      }
    }
  }
  return false;
}
Node RegExpOpr::convert1(unsigned cnt, Node n) {
  Trace("regexp-debug") << "Converting " << n << " at " << cnt << "... " << std::endl;
  Node r1, r2;
  convert2(cnt, n, r1, r2);
  Trace("regexp-debug") << "... getting r1=" << r1 << ", and r2=" << r2 << std::endl;
  Node ret = r1==d_emptySingleton ? r2 : NodeManager::currentNM()->mkNode(kind::REGEXP_CONCAT, 
     NodeManager::currentNM()->mkNode(kind::REGEXP_STAR, r1), r2);
  ret = Rewriter::rewrite( ret );
  Trace("regexp-debug") << "... done convert at " << cnt << ", with return " << ret << std::endl;
  return ret;
}
void RegExpOpr::convert2(unsigned cnt, Node n, Node &r1, Node &r2) {
  if(n == d_emptyRegexp) {
    r1 = d_emptyRegexp;
    r2 = d_emptyRegexp;
  } else if(n == d_emptySingleton) {
    r1 = d_emptySingleton;
    r2 = d_emptySingleton;
  } else if(n.getKind() == kind::REGEXP_RV) {
    Assert(n[0].getConst<Rational>() <= RMAXINT, "Exceeded LONG_MAX in RegExpOpr::convert2");
    unsigned y = n[0].getConst<Rational>().getNumerator().toUnsignedInt();
    r1 = d_emptySingleton;
    if(cnt == y) {
      r2 = d_emptyRegexp;
    } else {
      r2 = n;
    }
  } else if(n.getKind() == kind::REGEXP_CONCAT) {
    bool flag = true;
    std::vector<Node> vr1, vr2;
    for( unsigned i=0; i<n.getNumChildren(); i++ ) {
      if(containC2(cnt, n[i])) {
        Node t1, t2;
        convert2(cnt, n[i], t1, t2);
        vr1.push_back(t1);
        r1 = vr1.size()==0 ? d_emptyRegexp : vr1.size()==1 ? vr1[0] :
             NodeManager::currentNM()->mkNode(kind::REGEXP_CONCAT, vr1);
        vr2.push_back(t2);
        for( unsigned j=i+1; j<n.getNumChildren(); j++ ) {
          vr2.push_back(n[j]);
        }
        r2 = vr2.size()==0 ? d_emptyRegexp : vr2.size()==1 ? vr2[0] :
             NodeManager::currentNM()->mkNode(kind::REGEXP_CONCAT, vr2);
        flag = false;
        break;
      } else {
        vr1.push_back(n[i]);
      }
    }
    if(flag) {
      r1 = d_emptySingleton;
      r2 = n;
    }
  } else if(n.getKind() == kind::REGEXP_UNION) {
    std::vector<Node> vr1, vr2;
    for( unsigned i=0; i<n.getNumChildren(); i++ ) {
      Node t1, t2;
      convert2(cnt, n[i], t1, t2);
      vr1.push_back(t1);
      vr2.push_back(t2);
    }
    r1 = NodeManager::currentNM()->mkNode(kind::REGEXP_UNION, vr1);
    r2 = NodeManager::currentNM()->mkNode(kind::REGEXP_UNION, vr2);
  } else if(n.getKind() == kind::STRING_TO_REGEXP || n.getKind() == kind::REGEXP_SIGMA || n.getKind() == kind::REGEXP_RANGE) {
      r1 = d_emptySingleton;
      r2 = n;
  } else if(n.getKind() == kind::REGEXP_LOOP) {
    //TODO:LOOP
    r1 = d_emptySingleton;
    r2 = n;
    //Unreachable();
  } else {
    //is it possible?
    Unreachable();
  }
}

bool RegExpOpr::testNoRV(Node r) {
  std::map< Node, bool >::const_iterator itr = d_norv_cache.find(r);
  if(itr != d_norv_cache.end()) {
    return itr->second;
  } else {
    if(r.getKind() == kind::REGEXP_RV) {
      return false;
    } else if(r.getNumChildren() > 1) {
      for(unsigned int i=0; i<r.getNumChildren(); i++) {
        if(!testNoRV(r[i])) {
          return false;
        }
      }
    }
    return true;
  }
}

Node RegExpOpr::intersectInternal( Node r1, Node r2, std::map< PairNodes, Node > cache, unsigned cnt ) {
  //Assert(checkConstRegExp(r1) && checkConstRegExp(r2));
  if(r1 > r2) {
    TNode tmpNode = r1;
    r1 = r2;
    r2 = tmpNode;
  }
  Trace("regexp-int") << "Starting INTERSECT(" << cnt << "):\n  "<< mkString(r1) << ",\n  " << mkString(r2) << std::endl;
  //if(Trace.isOn("regexp-debug")) {
  //  Trace("regexp-debug") << "... with cache:\n";
  //  for(std::map< PairNodes, Node >::const_iterator itr=cache.begin();
  //      itr!=cache.end();itr++) {
  //        Trace("regexp-debug") << "(" << itr->first.first << "," << itr->first.second << ")->" << itr->second << std::endl;
  //      }
  //}
  std::pair < Node, Node > p(r1, r2);
  std::map < PairNodes, Node >::const_iterator itr = d_inter_cache.find(p);
  Node rNode;
  if(itr != d_inter_cache.end()) {
    rNode = itr->second;
  } else {
    Trace("regexp-int-debug") << " ... not in cache" << std::endl;
    if(r1 == d_emptyRegexp || r2 == d_emptyRegexp) {
      Trace("regexp-int-debug") << " ... one is empty set" << std::endl;
      rNode = d_emptyRegexp;
    } else if(r1 == d_emptySingleton || r2 == d_emptySingleton) {
      Trace("regexp-int-debug") << " ... one is empty singleton" << std::endl;
      Node exp;
      int r = delta((r1 == d_emptySingleton ? r2 : r1), exp);
      if(r == 0) {
        //TODO: variable
        Unreachable();
      } else if(r == 1) {
        rNode = d_emptySingleton;
      } else {
        rNode = d_emptyRegexp;
      }
    } else if(r1 == r2) {
      Trace("regexp-int-debug") << " ... equal" << std::endl;
      rNode = r1; //convert1(cnt, r1);
    } else {
      Trace("regexp-int-debug") << " ... normal checking" << std::endl;
      std::map< PairNodes, Node >::const_iterator itrcache = cache.find(p);
      if(itrcache != cache.end()) {
        rNode = itrcache->second;
      } else {
        Trace("regexp-int-debug") << " ... normal without cache" << std::endl;
        std::vector< unsigned char > cset;
        std::set< unsigned char > cset1, cset2;
        std::set< Node > vset1, vset2;
        firstChars(r1, cset1, vset1);
        firstChars(r2, cset2, vset2);
        Trace("regexp-int-debug") << " ... got fset" << std::endl;
        std::set_intersection(cset1.begin(), cset1.end(), cset2.begin(), cset2.end(),
             std::inserter(cset, cset.begin()));
        std::vector< Node > vec_nodes;
        Node delta_exp;
        Trace("regexp-int-debug") << " ... try delta" << std::endl;
        int flag = delta(r1, delta_exp);
        int flag2 = delta(r2, delta_exp);
        Trace("regexp-int-debug") << " ... delta1=" << flag << ", delta2=" << flag2 << std::endl;
        if(flag != 2 && flag2 != 2) {
          if(flag == 1 && flag2 == 1) {
            vec_nodes.push_back(d_emptySingleton);
          } else {
            //TODO: variable
            Unreachable();
          }
        }
        if(Trace.isOn("regexp-int-debug")) {
          Trace("regexp-int-debug") << "Try CSET(" << cset.size() << ") = {";
          for(std::vector<unsigned char>::const_iterator itr = cset.begin();
            itr != cset.end(); itr++) {
            //CVC4::String c( *itr );
            if(itr != cset.begin()) {
              Trace("regexp-int-debug") << ", ";
            }
            Trace("regexp-int-debug") << ( *itr );
          }
          Trace("regexp-int-debug") << std::endl;
        }
        std::map< PairNodes, Node > cacheX;
        for(std::vector<unsigned char>::const_iterator itr = cset.begin();
          itr != cset.end(); itr++) {
          CVC4::String c( *itr );
          Trace("regexp-int-debug") << "Try character " << c << " ... " << std::endl;
          Node r1l = derivativeSingle(r1, c);
          Node r2l = derivativeSingle(r2, c);
          Trace("regexp-int-debug") << "  ... got partial(r1,c) = " << mkString(r1l) << std::endl;
          Trace("regexp-int-debug") << "  ... got partial(r2,c) = " << mkString(r2l) << std::endl;
          Node rt;
          
          if(r1l > r2l) {
            Node tnode = r1l;
            r1l = r2l; r2l = tnode;
          }
          PairNodes pp(r1l, r2l);
          std::map< PairNodes, Node >::const_iterator itr2 = cacheX.find(pp);
          if(itr2 != cacheX.end()) {
            rt = itr2->second;
          } else {
            std::map< PairNodes, Node > cache2(cache);
            cache2[ p ] = NodeManager::currentNM()->mkNode(kind::REGEXP_RV, NodeManager::currentNM()->mkConst(CVC4::Rational(cnt)));
            rt = intersectInternal(r1l, r2l, cache2, cnt+1);
            cacheX[ pp ] = rt;
          }

          rt = Rewriter::rewrite( NodeManager::currentNM()->mkNode(kind::REGEXP_CONCAT,
            NodeManager::currentNM()->mkNode(kind::STRING_TO_REGEXP, NodeManager::currentNM()->mkConst(c)), rt) );

          Trace("regexp-int-debug") << "  ... got p(r1,c) && p(r2,c) = " << mkString(rt) << std::endl;
          vec_nodes.push_back(rt);
        }
        rNode = Rewriter::rewrite( vec_nodes.size()==0 ? d_emptyRegexp : vec_nodes.size()==1 ? vec_nodes[0] :
            NodeManager::currentNM()->mkNode(kind::REGEXP_UNION, vec_nodes) );
        rNode = convert1(cnt, rNode);
        rNode = Rewriter::rewrite( rNode );
      }
    }
    Trace("regexp-int-debug") << "  ... try testing no RV of " << mkString(rNode) << std::endl;
    if(testNoRV(rNode)) {
      d_inter_cache[p] = rNode;
    }
  }
  Trace("regexp-int") << "End(" << cnt << ") of INTERSECT( " << mkString(r1) << ", " << mkString(r2) << " ) = " << mkString(rNode) << std::endl;
  return rNode;
}

Node RegExpOpr::removeIntersection(Node r) {
  Assert( checkConstRegExp(r) );
  std::map < Node, Node >::const_iterator itr = d_rm_inter_cache.find(r);
  Node retNode;
  if(itr != d_rm_inter_cache.end()) {
    retNode = itr->second;
  } else {
    switch(r.getKind()) {
      case kind::REGEXP_EMPTY: {
        retNode = r;
        break;
      }
      case kind::REGEXP_SIGMA: {
        retNode = r;
        break;
      }
      case kind::REGEXP_RANGE: {
        retNode = r;
        break;
      }
      case kind::STRING_TO_REGEXP: {
        retNode = r;
        break;
      }
      case kind::REGEXP_CONCAT: {
        std::vector< Node > vec_nodes;
        for(unsigned i=0; i<r.getNumChildren(); i++) {
          Node tmpNode = removeIntersection( r[i] );
          vec_nodes.push_back( tmpNode );
        }
        retNode = Rewriter::rewrite( NodeManager::currentNM()->mkNode(kind::REGEXP_CONCAT, vec_nodes) );
        break;
      }
      case kind::REGEXP_UNION: {
        std::vector< Node > vec_nodes;
        for(unsigned i=0; i<r.getNumChildren(); i++) {
          Node tmpNode = removeIntersection( r[i] );
          vec_nodes.push_back( tmpNode );
        }
        retNode = Rewriter::rewrite( NodeManager::currentNM()->mkNode(kind::REGEXP_UNION, vec_nodes) );
        break;
      }
      case kind::REGEXP_INTER: {
        retNode = removeIntersection( r[0] );
        for(unsigned i=1; i<r.getNumChildren(); i++) {
          bool spflag = false;
          Node tmpNode = removeIntersection( r[i] );
          retNode = intersect( retNode, tmpNode, spflag );
        }
        break;
      }
      case kind::REGEXP_STAR: {
        retNode = removeIntersection( r[0] );
        retNode = Rewriter::rewrite( NodeManager::currentNM()->mkNode(kind::REGEXP_STAR, retNode) );
        break;
      }
      case kind::REGEXP_LOOP: {
        retNode = removeIntersection( r[0] );
        retNode = Rewriter::rewrite( NodeManager::currentNM()->mkNode(kind::REGEXP_LOOP, retNode, r[1], r[2]) );
        break;
      }
      default: {
        Unreachable();
      }
    }
    d_rm_inter_cache[r] = retNode;
  }
  Trace("regexp-intersect") << "Remove INTERSECTION( " << mkString(r) << " ) = " << mkString(retNode) << std::endl;
  return retNode;
}

Node RegExpOpr::intersect(Node r1, Node r2, bool &spflag) {
  if(checkConstRegExp(r1) && checkConstRegExp(r2)) {
    Node rr1 = removeIntersection(r1);
    Node rr2 = removeIntersection(r2);
    std::map< PairNodes, Node > cache;
    Trace("regexp-intersect") << "Start INTERSECTION(\n\t" << mkString(r1) << ",\n\t"<< mkString(r2) << ")" << std::endl;
    Node retNode = intersectInternal(rr1, rr2, cache, 1);
    Trace("regexp-intersect") << "End INTERSECTION(\n\t" << mkString(r1) << ",\n\t"<< mkString(r2) << ") =\n\t" << mkString(retNode) << std::endl;
    return retNode;
  } else {
    spflag = true;
    return Node::null();
  }
}

Node RegExpOpr::complement(Node r, int &ret) {
  Node rNode;
  ret = 1;
  if(d_compl_cache.find(r) != d_compl_cache.end()) {
    rNode = d_compl_cache[r].first;
    ret = d_compl_cache[r].second;
  } else {
    if(r == d_emptyRegexp) {
      rNode = d_sigma_star;
    } else if(r == d_emptySingleton) {
      rNode = NodeManager::currentNM()->mkNode(kind::REGEXP_CONCAT, d_sigma, d_sigma_star);
    } else if(!checkConstRegExp(r)) {
      //TODO: var to be extended
      ret = 0;
    } else {
      std::set<unsigned char> cset;
      SetNodes vset;
      firstChars(r, cset, vset);
      std::vector< Node > vec_nodes;
      for(unsigned char i=0; i<=d_lastchar; i++) {
        CVC4::String c(i);
        Node n = NodeManager::currentNM()->mkNode(kind::STRING_TO_REGEXP, NodeManager::currentNM()->mkConst(c));
        Node r2;
        if(cset.find(i) == cset.end()) {
          r2 = d_sigma_star;
        } else {
          int rt;
          derivativeS(r, c, r2);
          if(r2 == r) {
            r2 = d_emptyRegexp;
          } else {
            r2 = complement(r2, rt);
          }
        }
        n = Rewriter::rewrite(NodeManager::currentNM()->mkNode(kind::REGEXP_CONCAT, n, r2));
        vec_nodes.push_back(n);
      }
      rNode = vec_nodes.size()==0? d_emptyRegexp : vec_nodes.size()==1? vec_nodes[0] :
            NodeManager::currentNM()->mkNode(kind::REGEXP_UNION, vec_nodes);
    }
    rNode = Rewriter::rewrite(rNode);
    std::pair< Node, int > p(rNode, ret);
    d_compl_cache[r] = p;
  }
  Trace("regexp-compl") << "COMPL( " << mkString(r) << " ) = " << mkString(rNode) << ", ret=" << ret << std::endl;
  return rNode;
}

void RegExpOpr::splitRegExp(Node r, std::vector< PairNodes > &pset) {
  Assert(checkConstRegExp(r));
  if(d_split_cache.find(r) != d_split_cache.end()) {
    pset = d_split_cache[r];
  } else {
    switch( r.getKind() ) {
      case kind::REGEXP_EMPTY: {
        break;
      }
      case kind::REGEXP_OPT: {
        PairNodes tmp(d_emptySingleton, d_emptySingleton);
        pset.push_back(tmp);
      }
      case kind::REGEXP_RANGE:
      case kind::REGEXP_SIGMA: {
        PairNodes tmp1(d_emptySingleton, r);
        PairNodes tmp2(r, d_emptySingleton);
        pset.push_back(tmp1);
        pset.push_back(tmp2);
        break;
      }
      case kind::STRING_TO_REGEXP: {
        Assert(r[0].isConst());
        CVC4::String s = r[0].getConst< CVC4::String >();
        PairNodes tmp1(d_emptySingleton, r);
        pset.push_back(tmp1);
        for(unsigned i=1; i<s.size(); i++) {
          CVC4::String s1 = s.substr(0, i);
          CVC4::String s2 = s.substr(i);
          Node n1 = NodeManager::currentNM()->mkNode(kind::STRING_TO_REGEXP, NodeManager::currentNM()->mkConst(s1));
          Node n2 = NodeManager::currentNM()->mkNode(kind::STRING_TO_REGEXP, NodeManager::currentNM()->mkConst(s2));
          PairNodes tmp3(n1, n2);
          pset.push_back(tmp3);
        }
        PairNodes tmp2(r, d_emptySingleton);
        pset.push_back(tmp2);
        break;
      }
      case kind::REGEXP_CONCAT: {
        for(unsigned i=0; i<r.getNumChildren(); i++) {
          std::vector< PairNodes > tset;
          splitRegExp(r[i], tset);
          std::vector< Node > hvec;
          std::vector< Node > tvec;
          for(unsigned j=0; j<=i; j++) {
            hvec.push_back(r[j]);
          }
          for(unsigned j=i; j<r.getNumChildren(); j++) {
            tvec.push_back(r[j]);
          }
          for(unsigned j=0; j<tset.size(); j++) {
            hvec[i] = tset[j].first;
            tvec[0] = tset[j].second;
            Node r1 = Rewriter::rewrite( hvec.size()==1?hvec[0]:NodeManager::currentNM()->mkNode(kind::REGEXP_CONCAT, hvec) );
            Node r2 = Rewriter::rewrite( tvec.size()==1?tvec[0]:NodeManager::currentNM()->mkNode(kind::REGEXP_CONCAT, tvec) );
            PairNodes tmp2(r1, r2);
            pset.push_back(tmp2);
          }
        }
        break;
      }
      case kind::REGEXP_UNION: {
        for(unsigned i=0; i<r.getNumChildren(); ++i) {
          std::vector< PairNodes > tset;
          splitRegExp(r[i], tset);
          pset.insert(pset.end(), tset.begin(), tset.end());
        }
        break;
      }
      case kind::REGEXP_INTER: {
        bool spflag = false;
        Node tmp = r[0];
        for(unsigned i=1; i<r.getNumChildren(); i++) {
          tmp = intersect(tmp, r[i], spflag);
        }
        splitRegExp(tmp, pset);
        break;
      }
      case kind::REGEXP_STAR: {
        std::vector< PairNodes > tset;
        splitRegExp(r[0], tset);
        PairNodes tmp1(d_emptySingleton, d_emptySingleton);
        pset.push_back(tmp1);
        for(unsigned i=0; i<tset.size(); i++) {
          Node r1 = tset[i].first==d_emptySingleton ? r : NodeManager::currentNM()->mkNode(kind::REGEXP_CONCAT, r, tset[i].first);
          Node r2 = tset[i].second==d_emptySingleton ? r : NodeManager::currentNM()->mkNode(kind::REGEXP_CONCAT, tset[i].second, r);
          PairNodes tmp2(r1, r2);
          pset.push_back(tmp2);
        }
        break;
      }
      case kind::REGEXP_LOOP: {
        unsigned l = r[1].getConst<Rational>().getNumerator().toUnsignedInt();
        unsigned u = r[2].getConst<Rational>().getNumerator().toUnsignedInt();
        if(l == u) {
          //R^n
          if(l == 0) {
            PairNodes tmp1(d_emptySingleton, d_emptySingleton);
            pset.push_back(tmp1);
          } else if(l == 1) {
            splitRegExp(r[0], pset);
          } else {
            std::vector< PairNodes > tset;
            splitRegExp(r[0], tset);
            for(unsigned j=0; j<l; j++) {
              Node num1 = NodeManager::currentNM()->mkConst(CVC4::Rational(j));
              Node r1 = j==0? d_emptySingleton : j==1? r[0] : NodeManager::currentNM()->mkNode(kind::REGEXP_LOOP, r[0], num1, num1);
              unsigned j2 = l - j - 1;
              Node num2 = NodeManager::currentNM()->mkConst(CVC4::Rational(j2));
              Node r2 = j2==0? d_emptySingleton : j2==1? r[0] : NodeManager::currentNM()->mkNode(kind::REGEXP_LOOP, r[0], num2, num2);
              for(unsigned i=0; i<tset.size(); i++) {
                r1 = tset[i].first==d_emptySingleton? r1 : NodeManager::currentNM()->mkNode(kind::REGEXP_CONCAT, r1, tset[i].first);
                r2 = tset[i].second==d_emptySingleton? r2 : NodeManager::currentNM()->mkNode(kind::REGEXP_CONCAT, tset[i].second, r2);
                PairNodes tmp2(r1, r2);
                pset.push_back(tmp2);
              }
            }
          }
        } else {
          //R{0,n}
          PairNodes tmp1(d_emptySingleton, d_emptySingleton);
          pset.push_back(tmp1);
          std::vector< PairNodes > tset;
          splitRegExp(r[0], tset);
          pset.insert(pset.end(), tset.begin(), tset.end());
          for(unsigned k=2; k<=u; k++) {
            for(unsigned j=0; j<k; j++) {
              Node num1 = NodeManager::currentNM()->mkConst(CVC4::Rational(j));
              Node r1 = j==0? d_emptySingleton : j==1? r[0] : NodeManager::currentNM()->mkNode(kind::REGEXP_LOOP, r[0], num1, num1);
              unsigned j2 = k - j - 1;
              Node num2 = NodeManager::currentNM()->mkConst(CVC4::Rational(j2));
              Node r2 = j2==0? d_emptySingleton : j2==1? r[0] : NodeManager::currentNM()->mkNode(kind::REGEXP_LOOP, r[0], num2, num2);
              for(unsigned i=0; i<tset.size(); i++) {
                r1 = tset[i].first==d_emptySingleton? r1 : NodeManager::currentNM()->mkNode(kind::REGEXP_CONCAT, r1, tset[i].first);
                r2 = tset[i].second==d_emptySingleton? r2 : NodeManager::currentNM()->mkNode(kind::REGEXP_CONCAT, tset[i].second, r2);
                PairNodes tmp2(r1, r2);
                pset.push_back(tmp2);
              }
            }
          }
        }
        break;
      }
      case kind::REGEXP_PLUS: {
        std::vector< PairNodes > tset;
        splitRegExp(r[0], tset);
        for(unsigned i=0; i<tset.size(); i++) {
          Node r1 = NodeManager::currentNM()->mkNode(kind::REGEXP_CONCAT, r, tset[i].first);
          Node r2 = NodeManager::currentNM()->mkNode(kind::REGEXP_CONCAT, tset[i].second, r);
          PairNodes tmp2(r1, r2);
          pset.push_back(tmp2);
        }
        break;
      }
      default: {
        Trace("strings-error") << "Unsupported term: " << r << " in splitRegExp." << std::endl;
        Assert( false );
        //return Node::null();
      }
    }
    d_split_cache[r] = pset;
  }
}

void RegExpOpr::flattenRegExp(Node r, std::vector< std::pair< CVC4::String, unsigned > > &fvec) {
  Assert(false);
  Assert(checkConstRegExp(r));
  switch( r.getKind() ) {
      case kind::REGEXP_EMPTY: {
        //TODO
        break;
      }
      case kind::REGEXP_SIGMA: {
        CVC4::String s("a");
        std::pair< CVC4::String, unsigned > tmp(s, 0);
        //TODO
        break;
      }
      case kind::STRING_TO_REGEXP: {
        Assert(r[0].isConst());
        CVC4::String s = r[0].getConst< CVC4::String >();
        std::pair< CVC4::String, unsigned > tmp(s, 0);
        //TODO
        break;
      }
      case kind::REGEXP_CONCAT: {
        for(unsigned i=0; i<r.getNumChildren(); i++) {
          //TODO
        }
        break;
      }
      case kind::REGEXP_UNION: {
        for(unsigned i=0; i<r.getNumChildren(); ++i) {
          //TODO
        }
        break;
      }
      case kind::REGEXP_INTER: {
        //TODO
        break;
      }
      case kind::REGEXP_STAR: {
        //TODO
        break;
      }
      case kind::REGEXP_LOOP: {
        //TODO
        break;
      }
      default: {
        Unreachable();
      }
  }
}

void RegExpOpr::disjunctRegExp(Node r, std::vector<Node> &vec_or) {
  switch(r.getKind()) {
      case kind::REGEXP_EMPTY: {
        vec_or.push_back(r);
        break;
      }
      case kind::REGEXP_SIGMA: {
        vec_or.push_back(r);
        break;
      }
      case kind::REGEXP_RANGE: {
        vec_or.push_back(r);
        break;
      }
      case kind::STRING_TO_REGEXP: {
        vec_or.push_back(r);
        break;
      }
      case kind::REGEXP_CONCAT: {
        disjunctRegExp(r[0], vec_or);
        for(unsigned i=1; i<r.getNumChildren(); i++) {
          std::vector<Node> vec_con;
          disjunctRegExp(r[i], vec_con);
          std::vector<Node> vec_or2;
          for(unsigned k1=0; k1<vec_or.size(); k1++) {
            for(unsigned k2=0; k2<vec_con.size(); k2++) {
              Node tmp = Rewriter::rewrite( NodeManager::currentNM()->mkNode(kind::REGEXP_CONCAT, vec_or[k1], vec_con[k2]) );
              if(std::find(vec_or2.begin(), vec_or2.end(), tmp) == vec_or2.end()) {
                vec_or2.push_back( tmp );
              }
            }
          }
          vec_or = vec_or2;
        }
        break;
      }
      case kind::REGEXP_UNION: {
        for(unsigned i=0; i<r.getNumChildren(); ++i) {
          std::vector<Node> vec_or2;
          disjunctRegExp(r[i], vec_or2);
          vec_or.insert(vec_or.end(), vec_or2.begin(), vec_or2.end());
        }
        break;
      }
      case kind::REGEXP_INTER: {
        Assert(checkConstRegExp(r));
        Node rtmp = r[0];
        bool spflag = false;
        for(unsigned i=1; i<r.getNumChildren(); ++i) {
          rtmp = intersect(rtmp, r[i], spflag);
        }
        disjunctRegExp(rtmp, vec_or);
        break;
      }
      case kind::REGEXP_STAR: {
        vec_or.push_back(r);
        break;
      }
      case kind::REGEXP_LOOP: {
        vec_or.push_back(r);
        break;
      }
      default: {
        Unreachable();
      }
  }
}

//printing
std::string RegExpOpr::niceChar(Node r) {
  if(r.isConst()) {
    std::string s = r.getConst<CVC4::String>().toString() ;
    return s == "." ? "\\." : s;
  } else {
    std::string ss = "$" + r.toString();
    return ss;
  }
}
std::string RegExpOpr::mkString( Node r ) {
  std::string retStr;
  if(r.isNull()) {
    retStr = "\\E";
  } else {
    int k = r.getKind();
    switch( k ) {
      case kind::REGEXP_EMPTY: {
        retStr += "\\E";
        break;
      }
      case kind::REGEXP_SIGMA: {
        retStr += ".";
        break;
      }
      case kind::STRING_TO_REGEXP: {
        std::string tmp( niceChar( r[0] ) );
        retStr += tmp.size()==1? tmp : "(" + tmp + ")";
        break;
      }
      case kind::REGEXP_CONCAT: {
        retStr += "(";
        for(unsigned i=0; i<r.getNumChildren(); ++i) {
          //if(i != 0) retStr += ".";
          retStr += mkString( r[i] );
        }
        retStr += ")";
        break;
      }
      case kind::REGEXP_UNION: {
        retStr += "(";
        for(unsigned i=0; i<r.getNumChildren(); ++i) {
          if(i != 0) retStr += "|";
          retStr += mkString( r[i] );
        }
        retStr += ")";
        break;
      }
      case kind::REGEXP_INTER: {
        retStr += "(";
        for(unsigned i=0; i<r.getNumChildren(); ++i) {
          if(i != 0) retStr += "&";
          retStr += mkString( r[i] );
        }
        retStr += ")";
        break;
      }
      case kind::REGEXP_STAR: {
        retStr += mkString( r[0] );
        retStr += "*";
        break;
      }
      case kind::REGEXP_PLUS: {
        retStr += mkString( r[0] );
        retStr += "+";
        break;
      }
      case kind::REGEXP_OPT: {
        retStr += mkString( r[0] );
        retStr += "?";
        break;
      }
      case kind::REGEXP_RANGE: {
        retStr += "[";
        retStr += niceChar( r[0] );
        retStr += "-";
        retStr += niceChar( r[1] );
        retStr += "]";
        break;
      }
      case kind::REGEXP_LOOP: {
        retStr += "(";
        retStr += mkString(r[0]);
        retStr += ")";
        retStr += "{";
        retStr += r[1].getConst<Rational>().toString();
        retStr += ",";
        if(r.getNumChildren() == 3) {
          retStr += r[2].getConst<Rational>().toString();
        }
        retStr += "}";
        break;
      }
      case kind::REGEXP_RV: {
        retStr += "<";
        retStr += r[0].getConst<Rational>().getNumerator().toString();
        retStr += ">";
        break;
      }
      default:
        Trace("strings-error") << "Unsupported term: " << r << " in RegExp." << std::endl;
        //Assert( false );
        //return Node::null();
    }
  }

  return retStr;
}

}/* CVC4::theory::strings namespace */
}/* CVC4::theory namespace */
}/* CVC4 namespace */
