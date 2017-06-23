/*********************                                                        */
/*! \file fun_def_engine.h
 ** \verbatim
 ** Top contributors (to current version):
 **   Andrew Reynolds, Tim King
 ** This file is part of the CVC4 project.
 ** Copyright (c) 2009-2016 by the authors listed in the file AUTHORS
 ** in the top-level source directory) and their institutional affiliations.
 ** All rights reserved.  See the file COPYING in the top-level source
 ** directory for licensing information.\endverbatim
 **
 ** \brief Specialized techniques for (recursively) defined functions
 **/

#include "cvc4_private.h"

#ifndef __CVC4__QUANTIFIERS_FUN_DEF_ENGINE_H
#define __CVC4__QUANTIFIERS_FUN_DEF_ENGINE_H

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include "expr/node.h"
#include "expr/type_node.h"
#include "theory/quantifiers_engine.h"

namespace CVC4 {
namespace theory {
namespace quantifiers {

//module for handling (recursively) defined functions
class FunDefEngine : public QuantifiersModule {
private:

public:
  FunDefEngine( QuantifiersEngine * qe, context::Context* c );
  ~FunDefEngine(){}

  /* whether this module needs to check this round */
  bool needsCheck( Theory::Effort e );
  /* reset at a round */
  void reset_round( Theory::Effort e );
  /* Call during quantifier engine's check */
  void check( Theory::Effort e, unsigned quant_e );
  /* Called for new quantifiers */
  void registerQuantifier( Node q );
  /** called for everything that gets asserted */
  void assertNode( Node n );
  /** Identify this module (for debugging, dynamic configuration, etc..) */
  std::string identify() const { return "FunDefEngine"; }
};


}
}
}

#endif
