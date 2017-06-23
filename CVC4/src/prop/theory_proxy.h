/*********************                                                        */
/*! \file theory_proxy.h
 ** \verbatim
 ** Top contributors (to current version):
 **   Tim King, Morgan Deters, Dejan Jovanovic
 ** This file is part of the CVC4 project.
 ** Copyright (c) 2009-2016 by the authors listed in the file AUTHORS
 ** in the top-level source directory) and their institutional affiliations.
 ** All rights reserved.  See the file COPYING in the top-level source
 ** directory for licensing information.\endverbatim
 **
 ** \brief SAT Solver.
 **
 ** SAT Solver.
 **/

#include "cvc4_private.h"

#ifndef __CVC4__PROP__SAT_H
#define __CVC4__PROP__SAT_H

// Just defining this for now, since there's no other SAT solver bindings.
// Optional blocks below will be unconditionally included
#define __CVC4_USE_MINISAT

#include <iosfwd>

#include "context/cdqueue.h"
#include "expr/expr_stream.h"
#include "expr/node.h"
#include "prop/sat_solver.h"
#include "smt_util/lemma_channels.h"
#include "smt_util/lemma_input_channel.h"
#include "smt_util/lemma_output_channel.h"
#include "theory/theory.h"
#include "util/statistics_registry.h"

namespace CVC4 {

class DecisionEngine;
class TheoryEngine;

namespace prop {

class PropEngine;
class CnfStream;

/**
 * The proxy class that allows the SatSolver to communicate with the theories
 */
class TheoryProxy {
public:
  TheoryProxy(PropEngine* propEngine,
              TheoryEngine* theoryEngine,
              DecisionEngine* decisionEngine,
              context::Context* context,
              CnfStream* cnfStream,
              std::ostream* replayLog,
              ExprStream* replayStream,
              LemmaChannels* globals);

  ~TheoryProxy();


  void theoryCheck(theory::Theory::Effort effort);

  void explainPropagation(SatLiteral l, SatClause& explanation);

  void theoryPropagate(SatClause& output);

  void enqueueTheoryLiteral(const SatLiteral& l);

  SatLiteral getNextTheoryDecisionRequest();

  SatLiteral getNextDecisionEngineRequest(bool& stopSearch);

  bool theoryNeedCheck() const;

  /**
   * Notifies of a new variable at a decision level.
   */
  void variableNotify(SatVariable var);

  TNode getNode(SatLiteral lit);

  void notifyRestart();

  void notifyNewLemma(SatClause& lemma);

  SatLiteral getNextReplayDecision();

  void logDecision(SatLiteral lit);

  void spendResource(unsigned ammount);

  bool isDecisionEngineDone();

  bool isDecisionRelevant(SatVariable var);

  SatValue getDecisionPolarity(SatVariable var);

  /** Shorthand for Dump("state") << PopCommand() */
  void dumpStatePop();

 private:
  /** The prop engine we are using. */
  PropEngine* d_propEngine;

  /** The CNF engine we are using. */
  CnfStream* d_cnfStream;

  /** The decision engine we are using. */
  DecisionEngine* d_decisionEngine;

  /** The theory engine we are using. */
  TheoryEngine* d_theoryEngine;


  /** Container for inputChannel() and outputChannel(). */
  LemmaChannels* d_channels;

  /** Stream on which to log replay events. */
  std::ostream* d_replayLog;

  /** Stream for replaying decisions. */
  ExprStream* d_replayStream;

  /** The lemma input channel we are using. */
  LemmaInputChannel* inputChannel();

  /** The lemma output channel we are using. */
  LemmaOutputChannel* outputChannel();

  /** Queue of asserted facts */
  context::CDQueue<TNode> d_queue;

  /**
   * Set of all lemmas that have been "shared" in the portfolio---i.e.,
   * all imported and exported lemmas.
   */
  std::hash_set<Node, NodeHashFunction> d_shared;

  /**
   * Statistic: the number of replayed decisions (via --replay).
   */
  IntStat d_replayedDecisions;

};/* class SatSolver */

}/* CVC4::prop namespace */

}/* CVC4 namespace */

#endif /* __CVC4__PROP__SAT_H */
