/*********************                                                        */
/*! \file maybe.h
 ** \verbatim
 ** Top contributors (to current version):
 **   Tim King
 ** This file is part of the CVC4 project.
 ** Copyright (c) 2009-2016 by the authors listed in the file AUTHORS
 ** in the top-level source directory) and their institutional affiliations.
 ** All rights reserved.  See the file COPYING in the top-level source
 ** directory for licensing information.\endverbatim
 **
 ** \brief This provides a templated Maybe construct.
 **
 ** This class provides a templated Maybe<T> construct.
 ** This follows the rough pattern of the Maybe monad in haskell.
 ** A Maybe is an algebraic type that is either Nothing | Just T
 **
 ** T must support T() and operator=.
 **
 ** This has a couple of uses:
 ** - There is no reasonable value or particularly clean way to represent
 **   Nothing using a value of T
 ** - High level of assurance that a value is not used before it is set.
 **/
#include "cvc4_private.h"

#pragma once

#include <ostream>

#include "base/exception.h"

namespace CVC4 {

template <class T>
class Maybe {
private:
  bool d_just;
  T d_value;

public:
  Maybe() : d_just(false), d_value(){}
  Maybe(const T& val): d_just(true), d_value(val){}

  Maybe& operator=(const T& v){
    d_just = true;
    d_value = v;
    return *this;
  }

  inline bool nothing() const { return !d_just; }
  inline bool just() const { return d_just; }

  void clear() {
    if(just()){
      d_just = false;
      d_value = T();
    }
  }

  T& value() {
    Assert(just(), "Maybe::value() requires the maybe to be set.");
    return d_value;
  }
  const T& constValue() const {
    Assert(just(), "Maybe::constValue() requires the maybe to be set.");
    return d_value;
  }

  operator const T&() const { return constValue(); }
};

template <class T>
inline std::ostream& operator<<(std::ostream& out, const Maybe<T>& m){
  out << "{";
  if(m.nothing()){
    out << "Nothing";
  }else{
    out << "Just ";
    out << m.constValue();
  }
  out << "}";
  return out;
}

}/* CVC4 namespace */
