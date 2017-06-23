/*********************                                                        */
/*! \file theory_bv_white.h
 ** \verbatim
 ** Top contributors (to current version):
 **   Liana Hadarean, Tim King
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


#include <cxxtest/TestSuite.h>

#include "theory/theory.h"
#include "smt/smt_engine.h"
#include "smt/smt_engine_scope.h"
#include "theory/bv/theory_bv.h"
#include "theory/bv/bitblaster_template.h"
#include "expr/node.h"
#include "expr/node_manager.h"
#include "context/context.h"

#include "theory/theory_test_utils.h"

#include <vector>

using namespace CVC4;
using namespace CVC4::theory;
using namespace CVC4::theory::bv;
using namespace CVC4::theory::bv::utils; 
using namespace CVC4::expr;
using namespace CVC4::context;
using namespace CVC4::smt;

using namespace std;

class TheoryBVWhite : public CxxTest::TestSuite {

  ExprManager* d_em;
  NodeManager* d_nm;
  SmtEngine* d_smt;
  SmtScope* d_scope;
  EagerBitblaster* d_bb;

public:

  TheoryBVWhite() {}

  void setUp() {
    d_em = new ExprManager();
    d_nm = NodeManager::fromExprManager(d_em);
    d_smt = new SmtEngine(d_em);
    d_scope = new SmtScope(d_smt);
    d_smt->setOption("bitblast", SExpr("eager"));
    d_bb = new EagerBitblaster(dynamic_cast<TheoryBV*>(d_smt->d_theoryEngine->d_theoryTable[THEORY_BV]));
  }

  void tearDown() {
    delete d_bb;
    delete d_scope;
    delete d_smt;
    delete d_em;
  }
 
  void testBitblasterCore() {
    Node x = d_nm->mkVar("x", d_nm->mkBitVectorType(16));
    Node y = d_nm->mkVar("y", d_nm->mkBitVectorType(16));
    Node x_plus_y = d_nm->mkNode(kind::BITVECTOR_PLUS, x, y);
    Node one = d_nm->mkConst<BitVector>(BitVector(16, 1u));
    Node x_shl_one = d_nm->mkNode(kind::BITVECTOR_SHL, x, one);
    Node eq = d_nm->mkNode(kind::EQUAL, x_plus_y, x_shl_one);
    Node not_x_eq_y = d_nm->mkNode(kind::NOT, d_nm->mkNode(kind::EQUAL, x, y));
    
    d_bb->bbFormula(eq);
    d_bb->bbFormula(not_x_eq_y);

    bool res = d_bb->solve(); 
    TS_ASSERT (res == false);
  }
};/* class TheoryBVWhite */
