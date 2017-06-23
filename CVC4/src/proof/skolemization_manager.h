/*********************                                                        */
/*! \file skolemization_manager.h
 ** \verbatim
 ** Top contributors (to current version):
 **   Guy Katz
 ** This file is part of the CVC4 project.
 ** Copyright (c) 2009-2016 by the authors listed in the file AUTHORS
 ** in the top-level source directory) and their institutional affiliations.
 ** All rights reserved.  See the file COPYING in the top-level source
 ** directory for licensing information.\endverbatim
 **
 ** [[ Add lengthier description here ]]

 ** \todo document this file

 **/

#include "cvc4_private.h"

#ifndef __CVC4__SKOLEMIZATION_MANAGER_H
#define __CVC4__SKOLEMIZATION_MANAGER_H

#include <iostream>
#include <map>
#include "proof/proof.h"
#include "util/proof.h"
#include "expr/node.h"
#include "theory/logic_info.h"
#include "theory/substitutions.h"

namespace CVC4 {

class SkolemizationManager {
public:
  void registerSkolem(Node disequality, Node skolem);
  bool hasSkolem(Node disequality);
  Node getSkolem(Node disequality);
  Node getDisequality(Node skolem);
  bool isSkolem(Node skolem);
  void clear();

  std::hash_map<Node, Node, NodeHashFunction>::const_iterator begin();
  std::hash_map<Node, Node, NodeHashFunction>::const_iterator end();

private:
  std::hash_map<Node, Node, NodeHashFunction> d_disequalityToSkolem;
  std::hash_map<Node, Node, NodeHashFunction> d_skolemToDisequality;
};

}/* CVC4 namespace */



#endif /* __CVC4__SKOLEMIZATION_MANAGER_H */
