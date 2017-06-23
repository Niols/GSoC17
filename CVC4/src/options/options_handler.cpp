/*********************                                                        */
/*! \file options_handler.cpp
 ** \verbatim
 ** Top contributors (to current version):
 **   Tim King, Andrew Reynolds
 ** This file is part of the CVC4 project.
 ** Copyright (c) 2009-2016 by the authors listed in the file AUTHORS
 ** in the top-level source directory) and their institutional affiliations.
 ** All rights reserved.  See the file COPYING in the top-level source
 ** directory for licensing information.\endverbatim
 **
 ** \brief Interface for custom handlers and predicates options.
 **
 ** Interface for custom handlers and predicates options.
 **/

#include "options/options_handler.h"

#include <ostream>
#include <string>
#include <cerrno>

#include "cvc4autoconfig.h"

#include "base/configuration.h"
#include "base/cvc4_assert.h"
#include "base/exception.h"
#include "base/modal_exception.h"
#include "base/output.h"
#include "lib/strtok_r.h"
#include "options/arith_heuristic_pivot_rule.h"
#include "options/arith_propagation_mode.h"
#include "options/arith_unate_lemma_mode.h"
#include "options/base_options.h"
#include "options/bv_bitblast_mode.h"
#include "options/bv_options.h"
#include "options/decision_mode.h"
#include "options/decision_options.h"
#include "options/didyoumean.h"
#include "options/language.h"
#include "options/option_exception.h"
#include "options/printer_modes.h"
#include "options/quantifiers_modes.h"
#include "options/simplification_mode.h"
#include "options/smt_options.h"
#include "options/theory_options.h"
#include "options/theoryof_mode.h"
#include "options/ufss_mode.h"

namespace CVC4 {
namespace options {

OptionsHandler::OptionsHandler(Options* options) : d_options(options) { }

void OptionsHandler::notifyForceLogic(const std::string& option){
  d_options->d_forceLogicListeners.notify();
}

void OptionsHandler::notifyBeforeSearch(const std::string& option)
    throw(ModalException)
{
  try{
    d_options->d_beforeSearchListeners.notify();
  } catch (ModalException&){
    std::stringstream ss;
    ss << "cannot change option `" << option
       << "' after final initialization (i.e., after logic has been set)";
    throw ModalException(ss.str());
  }
}


void OptionsHandler::notifyTlimit(const std::string& option) {
  d_options->d_tlimitListeners.notify();
}

void OptionsHandler::notifyTlimitPer(const std::string& option) {
  d_options->d_tlimitPerListeners.notify();
}

void OptionsHandler::notifyRlimit(const std::string& option) {
  d_options->d_rlimitListeners.notify();
}

void OptionsHandler::notifyRlimitPer(const std::string& option) {
  d_options->d_rlimitPerListeners.notify();
}


unsigned long OptionsHandler::tlimitHandler(std::string option, std::string optarg) throw(OptionException)  {
  unsigned long ms;
  std::istringstream convert(optarg);
  if (!(convert >> ms)) {
    throw OptionException("option `"+option+"` requires a number as an argument");
  }
  return ms;
}

unsigned long OptionsHandler::tlimitPerHandler(std::string option, std::string optarg) throw(OptionException) {
  unsigned long ms;

  std::istringstream convert(optarg);
  if (!(convert >> ms)) {
    throw OptionException("option `"+option+"` requires a number as an argument");
  }
  return ms;
}

unsigned long OptionsHandler::rlimitHandler(std::string option, std::string optarg) throw(OptionException) {
  unsigned long ms;

  std::istringstream convert(optarg);
  if (!(convert >> ms)) {
    throw OptionException("option `"+option+"` requires a number as an argument");
  }
  return ms;
}


unsigned long OptionsHandler::rlimitPerHandler(std::string option, std::string optarg) throw(OptionException) {
  unsigned long ms;

  std::istringstream convert(optarg);
  if (!(convert >> ms)) {
    throw OptionException("option `"+option+"` requires a number as an argument");
  }

  return ms;
}


/* options/base_options_handlers.h */
void OptionsHandler::notifyPrintSuccess(std::string option) {
  d_options->d_setPrintSuccessListeners.notify();
}

// theory/arith/options_handlers.h
const std::string OptionsHandler::s_arithUnateLemmasHelp = "\
Unate lemmas are generated before SAT search begins using the relationship\n\
of constant terms and polynomials.\n\
Modes currently supported by the --unate-lemmas option:\n\
+ none \n\
+ ineqs \n\
  Outputs lemmas of the general form (<= p c) implies (<= p d) for c < d.\n\
+ eqs \n\
  Outputs lemmas of the general forms\n\
  (= p c) implies (<= p d) for c < d, or\n\
  (= p c) implies (not (= p d)) for c != d.\n\
+ all \n\
  A combination of inequalities and equalities.\n\
";

const std::string OptionsHandler::s_arithPropagationModeHelp = "\
This decides on kind of propagation arithmetic attempts to do during the search.\n\
+ none\n\
+ unate\n\
 use constraints to do unate propagation\n\
+ bi (Bounds Inference)\n\
  infers bounds on basic variables using the upper and lower bounds of the\n\
  non-basic variables in the tableau.\n\
+both\n\
";

const std::string OptionsHandler::s_errorSelectionRulesHelp = "\
This decides on the rule used by simplex during heuristic rounds\n\
for deciding the next basic variable to select.\n\
Heuristic pivot rules available:\n\
+min\n\
  The minimum abs() value of the variable's violation of its bound. (default)\n\
+max\n\
  The maximum violation the bound\n\
+varord\n\
  The variable order\n\
";

ArithUnateLemmaMode OptionsHandler::stringToArithUnateLemmaMode(std::string option, std::string optarg) throw(OptionException) {
  if(optarg == "all") {
    return ALL_PRESOLVE_LEMMAS;
  } else if(optarg == "none") {
    return NO_PRESOLVE_LEMMAS;
  } else if(optarg == "ineqs") {
    return INEQUALITY_PRESOLVE_LEMMAS;
  } else if(optarg == "eqs") {
    return EQUALITY_PRESOLVE_LEMMAS;
  } else if(optarg == "help") {
    puts(s_arithUnateLemmasHelp.c_str());
    exit(1);
  } else {
    throw OptionException(std::string("unknown option for --unate-lemmas: `") +
                          optarg + "'.  Try --unate-lemmas help.");
  }
}

ArithPropagationMode OptionsHandler::stringToArithPropagationMode(std::string option, std::string optarg) throw(OptionException) {
  if(optarg == "none") {
    return NO_PROP;
  } else if(optarg == "unate") {
    return UNATE_PROP;
  } else if(optarg == "bi") {
    return BOUND_INFERENCE_PROP;
  } else if(optarg == "both") {
    return BOTH_PROP;
  } else if(optarg == "help") {
    puts(s_arithPropagationModeHelp.c_str());
    exit(1);
  } else {
    throw OptionException(std::string("unknown option for --arith-prop: `") +
                          optarg + "'.  Try --arith-prop help.");
  }
}

ErrorSelectionRule OptionsHandler::stringToErrorSelectionRule(std::string option, std::string optarg) throw(OptionException) {
  if(optarg == "min") {
    return MINIMUM_AMOUNT;
  } else if(optarg == "varord") {
    return VAR_ORDER;
  } else if(optarg == "max") {
    return MAXIMUM_AMOUNT;
  } else if(optarg == "help") {
    puts(s_errorSelectionRulesHelp.c_str());
    exit(1);
  } else {
    throw OptionException(std::string("unknown option for --heuristic-pivot-rule: `") +
                          optarg + "'.  Try --heuristic-pivot-rule help.");
  }
}
// theory/quantifiers/options_handlers.h

const std::string OptionsHandler::s_instWhenHelp = "\
Modes currently supported by the --inst-when option:\n\
\n\
full-last-call (default)\n\
+ Alternate running instantiation rounds at full effort and last\n\
  call.  In other words, interleave instantiation and theory combination.\n\
\n\
full\n\
+ Run instantiation round at full effort, before theory combination.\n\
\n\
full-delay \n\
+ Run instantiation round at full effort, before theory combination, after\n\
  all other theories have finished.\n\
\n\
full-delay-last-call \n\
+ Alternate running instantiation rounds at full effort after all other\n\
  theories have finished, and last call.  \n\
\n\
last-call\n\
+ Run instantiation at last call effort, after theory combination and\n\
  and theories report sat.\n\
\n\
";

const std::string OptionsHandler::s_literalMatchHelp = "\
Literal match modes currently supported by the --literal-match option:\n\
\n\
none \n\
+ Do not use literal matching.\n\
\n\
use (default)\n\
+ Consider phase requirements of triggers conservatively. For example, the\n\
  trigger P( x ) in forall( x ). ( P( x ) V ~Q( x ) ) will not be matched with\n\
  terms in the equivalence class of true, and likewise Q( x ) will not be matched\n\
  terms in the equivalence class of false. Extends to equality.\n\
\n\
agg-predicate \n\
+ Consider phase requirements aggressively for predicates. In the above example,\n\
  only match P( x ) with terms that are in the equivalence class of false.\n\
\n\
agg \n\
+ Consider the phase requirements aggressively for all triggers.\n\
\n\
";

const std::string OptionsHandler::s_mbqiModeHelp = "\
Model-based quantifier instantiation modes currently supported by the --mbqi option:\n\
\n\
default \n\
+ Use algorithm from Section 5.4.2 of thesis Finite Model Finding in Satisfiability \n\
  Modulo Theories.\n\
\n\
none \n\
+ Disable model-based quantifier instantiation.\n\
\n\
gen-ev \n\
+ Use model-based quantifier instantiation algorithm from CADE 24 finite\n\
  model finding paper based on generalizing evaluations.\n\
\n\
fmc-interval \n\
+ Same as default, but with intervals for models of integer functions.\n\
\n\
abs \n\
+ Use abstract MBQI algorithm (uses disjoint sets). \n\
\n\
";

const std::string OptionsHandler::s_qcfWhenModeHelp = "\
Quantifier conflict find modes currently supported by the --quant-cf-when option:\n\
\n\
default \n\
+ Default, apply conflict finding at full effort.\n\
\n\
last-call \n\
+ Apply conflict finding at last call, after theory combination and \n\
  and all theories report sat. \n\
\n\
std \n\
+ Apply conflict finding at standard effort.\n\
\n\
std-h \n\
+ Apply conflict finding at standard effort when heuristic says to. \n\
\n\
";

const std::string OptionsHandler::s_qcfModeHelp = "\
Quantifier conflict find modes currently supported by the --quant-cf option:\n\
\n\
prop-eq \n\
+ Default, apply QCF algorithm to propagate equalities as well as conflicts. \n\
\n\
conflict \n\
+ Apply QCF algorithm to find conflicts only.\n\
\n\
partial \n\
+ Apply QCF algorithm to instantiate heuristically as well. \n\
\n\
mc \n\
+ Apply QCF algorithm in a complete way, so that a model is ensured when it fails. \n\
\n\
";

const std::string OptionsHandler::s_userPatModeHelp = "\
User pattern modes currently supported by the --user-pat option:\n\
\n\
trust \n\
+ When provided, use only user-provided patterns for a quantified formula.\n\
\n\
use \n\
+ Use both user-provided and auto-generated patterns when patterns\n\
  are provided for a quantified formula.\n\
\n\
resort \n\
+ Use user-provided patterns only after auto-generated patterns saturate.\n\
\n\
ignore \n\
+ Ignore user-provided patterns. \n\
\n\
interleave \n\
+ Alternate between use/resort. \n\
\n\
";

const std::string OptionsHandler::s_triggerSelModeHelp = "\
Trigger selection modes currently supported by the --trigger-sel option:\n\
\n\
min | default \n\
+ Consider only minimal subterms that meet criteria for triggers.\n\
\n\
max \n\
+ Consider only maximal subterms that meet criteria for triggers. \n\
\n\
all \n\
+ Consider all subterms that meet criteria for triggers. \n\
\n\
min-s-max \n\
+ Consider only minimal subterms that meet criteria for single triggers, maximal otherwise. \n\
\n\
min-s-all \n\
+ Consider only minimal subterms that meet criteria for single triggers, all otherwise. \n\
\n\
";
const std::string OptionsHandler::s_triggerActiveSelModeHelp = "\
Trigger active selection modes currently supported by the --trigger-sel option:\n\
\n\
all \n\
+ Make all triggers active. \n\
\n\
min \n\
+ Activate triggers with minimal ground terms.\n\
\n\
max \n\
+ Activate triggers with maximal ground terms. \n\
\n\
";
const std::string OptionsHandler::s_prenexQuantModeHelp = "\
Prenex quantifiers modes currently supported by the --prenex-quant option:\n\
\n\
none \n\
+ Do no prenex nested quantifiers. \n\
\n\
default | simple \n\
+ Default, do simple prenexing of same sign quantifiers.\n\
\n\
dnorm \n\
+ Prenex to disjunctive prenex normal form.\n\
\n\
norm \n\
+ Prenex to prenex normal form.\n\
\n\
";

const std::string OptionsHandler::s_cegqiFairModeHelp = "\
Modes for enforcing fairness for counterexample guided quantifier instantion, supported by --cegqi-fair:\n\
\n\
uf-dt-size \n\
+ Enforce fairness using an uninterpreted function for datatypes size.\n\
\n\
default | dt-size \n\
+ Default, enforce fairness using size operator.\n\
\n\
dt-height-bound \n\
+ Enforce fairness by height bound predicate.\n\
\n\
none \n\
+ Do not enforce fairness. \n\
\n\
";

const std::string OptionsHandler::s_termDbModeHelp = "\
Modes for term database, supported by --term-db-mode:\n\
\n\
all  \n\
+ Quantifiers module considers all ground terms.\n\
\n\
relevant \n\
+ Quantifiers module considers only ground terms connected to current assertions. \n\
\n\
";

const std::string OptionsHandler::s_iteLiftQuantHelp = "\
Modes for term database, supported by --ite-lift-quant:\n\
\n\
none  \n\
+ Do not lift if-then-else in quantified formulas.\n\
\n\
simple  \n\
+ Lift if-then-else in quantified formulas if results in smaller term size.\n\
\n\
all \n\
+ Lift if-then-else in quantified formulas. \n\
\n\
";

const std::string OptionsHandler::s_cegqiSingleInvHelp = "\
Modes for single invocation techniques, supported by --cegqi-si:\n\
\n\
none  \n\
+ Do not use single invocation techniques.\n\
\n\
use (default) \n\
+ Use single invocation techniques only if grammar is not restrictive.\n\
\n\
all-abort  \n\
+ Always use single invocation techniques, abort if solution reconstruction will likely fail,\
  for instance, when the grammar does not have ITE and solution requires it.\n\
\n\
all \n\
+ Always use single invocation techniques. \n\
\n\
";

const std::string OptionsHandler::s_sygusInvTemplHelp = "\
Template modes for sygus invariant synthesis, supported by --sygus-inv-templ:\n\
\n\
none  \n\
+ Synthesize invariant directly.\n\
\n\
pre  \n\
+ Synthesize invariant based on weakening of precondition .\n\
\n\
post \n\
+ Synthesize invariant based on strengthening of postcondition. \n\
\n\
";

const std::string OptionsHandler::s_macrosQuantHelp = "\
Modes for quantifiers macro expansion, supported by --macros-quant-mode:\n\
\n\
all \n\
+ Infer definitions for functions, including those containing quantified formulas.\n\
\n\
ground (default) \n\
+ Only infer ground definitions for functions.\n\
\n\
ground-uf \n\
+ Only infer ground definitions for functions that result in triggers for all free variables.\n\
\n\
";

const std::string OptionsHandler::s_quantDSplitHelp = "\
Modes for quantifiers splitting, supported by --quant-dsplit-mode:\n\
\n\
none \n\
+ Never split quantified formulas.\n\
\n\
default \n\
+ Split quantified formulas over some finite datatypes when finite model finding is enabled.\n\
\n\
agg \n\
+ Aggressively split quantified formulas.\n\
\n\
";

const std::string OptionsHandler::s_quantRepHelp = "\
Modes for quantifiers representative selection, supported by --quant-rep-mode:\n\
\n\
ee \n\
+ Let equality engine choose representatives.\n\
\n\
first (default) \n\
+ Choose terms that appear first.\n\
\n\
depth \n\
+ Choose terms that are of minimal depth.\n\
\n\
";

const std::string OptionsHandler::s_fmfBoundMinModeModeHelp = "\
Modes for finite model finding bound minimization, supported by --fmf-bound-min-mode:\n\
\n\
none \n\
+ Do not minimize inferred bounds.\n\
\n\
int (default) \n\
+ Minimize integer ranges only.\n\
\n\
setc \n\
+ Minimize cardinality of set membership ranges only.\n\
\n\
all \n\
+ Minimize all inferred bounds.\n\
\n\
";


theory::quantifiers::InstWhenMode OptionsHandler::stringToInstWhenMode(std::string option, std::string optarg) throw(OptionException) {
  if(optarg == "pre-full") {
    return theory::quantifiers::INST_WHEN_PRE_FULL;
  } else if(optarg == "full") {
    return theory::quantifiers::INST_WHEN_FULL;
  } else if(optarg == "full-delay") {
    return theory::quantifiers::INST_WHEN_FULL_DELAY;
  } else if(optarg == "full-last-call") {
    return theory::quantifiers::INST_WHEN_FULL_LAST_CALL;
  } else if(optarg == "full-delay-last-call") {
    return theory::quantifiers::INST_WHEN_FULL_DELAY_LAST_CALL;
  } else if(optarg == "last-call") {
    return theory::quantifiers::INST_WHEN_LAST_CALL;
  } else if(optarg == "help") {
    puts(s_instWhenHelp.c_str());
    exit(1);
  } else {
    throw OptionException(std::string("unknown option for --inst-when: `") +
                          optarg + "'.  Try --inst-when help.");
  }
}

void OptionsHandler::checkInstWhenMode(std::string option, theory::quantifiers::InstWhenMode mode) throw(OptionException)  {
  if(mode == theory::quantifiers::INST_WHEN_PRE_FULL) {
    throw OptionException(std::string("Mode pre-full for ") + option + " is not supported in this release.");
  }
}

theory::quantifiers::LiteralMatchMode OptionsHandler::stringToLiteralMatchMode(std::string option, std::string optarg) throw(OptionException) {
  if(optarg ==  "none") {
    return theory::quantifiers::LITERAL_MATCH_NONE;
  } else if(optarg ==  "use") {
    return theory::quantifiers::LITERAL_MATCH_USE;
  } else if(optarg ==  "agg-predicate") {
    return theory::quantifiers::LITERAL_MATCH_AGG_PREDICATE;
  } else if(optarg ==  "agg") {
    return theory::quantifiers::LITERAL_MATCH_AGG;
  } else if(optarg ==  "help") {
    puts(s_literalMatchHelp.c_str());
    exit(1);
  } else {
    throw OptionException(std::string("unknown option for --literal-matching: `") +
                          optarg + "'.  Try --literal-matching help.");
  }
}

void OptionsHandler::checkLiteralMatchMode(std::string option, theory::quantifiers::LiteralMatchMode mode) throw(OptionException) {

}

theory::quantifiers::MbqiMode OptionsHandler::stringToMbqiMode(std::string option, std::string optarg) throw(OptionException) {
  if(optarg == "gen-ev") {
    return theory::quantifiers::MBQI_GEN_EVAL;
  } else if(optarg == "none") {
    return theory::quantifiers::MBQI_NONE;
  } else if(optarg == "default" || optarg ==  "fmc") {
    return theory::quantifiers::MBQI_FMC;
  } else if(optarg == "fmc-interval") {
    return theory::quantifiers::MBQI_FMC_INTERVAL;
  } else if(optarg == "abs") {
    return theory::quantifiers::MBQI_ABS;
  } else if(optarg == "trust") {
    return theory::quantifiers::MBQI_TRUST;
  } else if(optarg == "help") {
    puts(s_mbqiModeHelp.c_str());
    exit(1);
  } else {
    throw OptionException(std::string("unknown option for --mbqi: `") +
                          optarg + "'.  Try --mbqi help.");
  }
}


void OptionsHandler::checkMbqiMode(std::string option, theory::quantifiers::MbqiMode mode) throw(OptionException) {}


theory::quantifiers::QcfWhenMode OptionsHandler::stringToQcfWhenMode(std::string option, std::string optarg) throw(OptionException) {
  if(optarg ==  "default") {
    return theory::quantifiers::QCF_WHEN_MODE_DEFAULT;
  } else if(optarg ==  "last-call") {
    return theory::quantifiers::QCF_WHEN_MODE_LAST_CALL;
  } else if(optarg ==  "std") {
    return theory::quantifiers::QCF_WHEN_MODE_STD;
  } else if(optarg ==  "std-h") {
    return theory::quantifiers::QCF_WHEN_MODE_STD_H;
  } else if(optarg ==  "help") {
    puts(s_qcfWhenModeHelp.c_str());
    exit(1);
  } else {
    throw OptionException(std::string("unknown option for --quant-cf-when: `") +
                          optarg + "'.  Try --quant-cf-when help.");
  }
}

theory::quantifiers::QcfMode OptionsHandler::stringToQcfMode(std::string option, std::string optarg) throw(OptionException) {
  if(optarg ==  "conflict") {
    return theory::quantifiers::QCF_CONFLICT_ONLY;
  } else if(optarg ==  "default" || optarg == "prop-eq") {
    return theory::quantifiers::QCF_PROP_EQ;
  } else if(optarg == "partial") {
    return theory::quantifiers::QCF_PARTIAL;
  } else if(optarg ==  "help") {
    puts(s_qcfModeHelp.c_str());
    exit(1);
  } else {
    throw OptionException(std::string("unknown option for --quant-cf-mode: `") +
                          optarg + "'.  Try --quant-cf-mode help.");
  }
}

theory::quantifiers::UserPatMode OptionsHandler::stringToUserPatMode(std::string option, std::string optarg) throw(OptionException) {
  if(optarg == "use") {
    return theory::quantifiers::USER_PAT_MODE_USE;
  } else if(optarg ==  "default" || optarg == "trust") {
    return theory::quantifiers::USER_PAT_MODE_TRUST;
  } else if(optarg == "resort") {
    return theory::quantifiers::USER_PAT_MODE_RESORT;
  } else if(optarg == "ignore") {
    return theory::quantifiers::USER_PAT_MODE_IGNORE;
  } else if(optarg == "interleave") {
    return theory::quantifiers::USER_PAT_MODE_INTERLEAVE;
  } else if(optarg ==  "help") {
    puts(s_userPatModeHelp.c_str());
    exit(1);
  } else {
    throw OptionException(std::string("unknown option for --user-pat: `") +
                          optarg + "'.  Try --user-pat help.");
  }
}

theory::quantifiers::TriggerSelMode OptionsHandler::stringToTriggerSelMode(std::string option, std::string optarg) throw(OptionException) {
  if(optarg ==  "default" || optarg == "min") {
    return theory::quantifiers::TRIGGER_SEL_MIN;
  } else if(optarg == "max") {
    return theory::quantifiers::TRIGGER_SEL_MAX;
  } else if(optarg == "min-s-max") {
    return theory::quantifiers::TRIGGER_SEL_MIN_SINGLE_MAX;
  } else if(optarg == "min-s-all") {
    return theory::quantifiers::TRIGGER_SEL_MIN_SINGLE_ALL;
  } else if(optarg == "all") {
    return theory::quantifiers::TRIGGER_SEL_ALL;
  } else if(optarg ==  "help") {
    puts(s_triggerSelModeHelp.c_str());
    exit(1);
  } else {
    throw OptionException(std::string("unknown option for --trigger-sel: `") +
                          optarg + "'.  Try --trigger-sel help.");
  }
}

theory::quantifiers::TriggerActiveSelMode OptionsHandler::stringToTriggerActiveSelMode(std::string option, std::string optarg) throw(OptionException) {
  if(optarg ==  "default" || optarg == "all") {
    return theory::quantifiers::TRIGGER_ACTIVE_SEL_ALL;
  } else if(optarg == "min") {
    return theory::quantifiers::TRIGGER_ACTIVE_SEL_MIN;
  } else if(optarg == "max") {
    return theory::quantifiers::TRIGGER_ACTIVE_SEL_MAX;
  } else if(optarg ==  "help") {
    puts(s_triggerActiveSelModeHelp.c_str());
    exit(1);
  } else {
    throw OptionException(std::string("unknown option for --trigger-active-sel: `") +
                          optarg + "'.  Try --trigger-active-sel help.");
  }
}

theory::quantifiers::PrenexQuantMode OptionsHandler::stringToPrenexQuantMode(std::string option, std::string optarg) throw(OptionException) {
  if(optarg== "default" || optarg== "simple" ) {
    return theory::quantifiers::PRENEX_QUANT_SIMPLE;
  } else if(optarg == "none") {
    return theory::quantifiers::PRENEX_QUANT_NONE;
  } else if(optarg == "dnorm") {
    return theory::quantifiers::PRENEX_QUANT_DISJ_NORMAL;
  } else if(optarg == "norm") {
    return theory::quantifiers::PRENEX_QUANT_NORMAL;
  } else if(optarg ==  "help") {
    puts(s_prenexQuantModeHelp.c_str());
    exit(1);
  } else {
    throw OptionException(std::string("unknown option for --prenex-quant: `") +
                          optarg + "'.  Try --prenex-quant help.");
  }
}

theory::quantifiers::CegqiFairMode OptionsHandler::stringToCegqiFairMode(std::string option, std::string optarg) throw(OptionException) {
  if(optarg == "uf-dt-size" ) {
    return theory::quantifiers::CEGQI_FAIR_UF_DT_SIZE;
  } else if(optarg == "default" || optarg == "dt-size") {
    return theory::quantifiers::CEGQI_FAIR_DT_SIZE;
  } else if(optarg == "dt-height-bound" ){
    return theory::quantifiers::CEGQI_FAIR_DT_HEIGHT_PRED;
  //} else if(optarg == "dt-size-bound" ){
  //  return theory::quantifiers::CEGQI_FAIR_DT_SIZE_PRED;
  } else if(optarg == "none") {
    return theory::quantifiers::CEGQI_FAIR_NONE;
  } else if(optarg ==  "help") {
    puts(s_cegqiFairModeHelp.c_str());
    exit(1);
  } else {
    throw OptionException(std::string("unknown option for --cegqi-fair: `") +
                          optarg + "'.  Try --cegqi-fair help.");
  }
}

theory::quantifiers::TermDbMode OptionsHandler::stringToTermDbMode(std::string option, std::string optarg) throw(OptionException) {
  if(optarg == "all" ) {
    return theory::quantifiers::TERM_DB_ALL;
  } else if(optarg == "relevant") {
    return theory::quantifiers::TERM_DB_RELEVANT;
  } else if(optarg ==  "help") {
    puts(s_termDbModeHelp.c_str());
    exit(1);
  } else {
    throw OptionException(std::string("unknown option for --term-db-mode: `") +
                          optarg + "'.  Try --term-db-mode help.");
  }
}

theory::quantifiers::IteLiftQuantMode OptionsHandler::stringToIteLiftQuantMode(std::string option, std::string optarg) throw(OptionException) {
  if(optarg == "all" ) {
    return theory::quantifiers::ITE_LIFT_QUANT_MODE_ALL;
  } else if(optarg == "simple") {
    return theory::quantifiers::ITE_LIFT_QUANT_MODE_SIMPLE;
  } else if(optarg == "none") {
    return theory::quantifiers::ITE_LIFT_QUANT_MODE_NONE;
  } else if(optarg ==  "help") {
    puts(s_iteLiftQuantHelp.c_str());
    exit(1);
  } else {
    throw OptionException(std::string("unknown option for --ite-lift-quant: `") +
                          optarg + "'.  Try --ite-lift-quant help.");
  }
}

theory::quantifiers::CegqiSingleInvMode OptionsHandler::stringToCegqiSingleInvMode(std::string option, std::string optarg) throw(OptionException) {
  if(optarg == "none" ) {
    return theory::quantifiers::CEGQI_SI_MODE_NONE;
  } else if(optarg == "use" || optarg == "default") {
    return theory::quantifiers::CEGQI_SI_MODE_USE;
  } else if(optarg == "all-abort") {
    return theory::quantifiers::CEGQI_SI_MODE_ALL_ABORT;
  } else if(optarg == "all") {
    return theory::quantifiers::CEGQI_SI_MODE_ALL;
  } else if(optarg ==  "help") {
    puts(s_cegqiSingleInvHelp.c_str());
    exit(1);
  } else {
    throw OptionException(std::string("unknown option for --cegqi-si: `") +
                          optarg + "'.  Try --cegqi-si help.");
  }
}

theory::quantifiers::SygusInvTemplMode OptionsHandler::stringToSygusInvTemplMode(std::string option, std::string optarg) throw(OptionException) {
  if(optarg == "none" ) {
    return theory::quantifiers::SYGUS_INV_TEMPL_MODE_NONE;
  } else if(optarg == "pre") {
    return theory::quantifiers::SYGUS_INV_TEMPL_MODE_PRE;
  } else if(optarg == "post") {
    return theory::quantifiers::SYGUS_INV_TEMPL_MODE_POST;
  } else if(optarg ==  "help") {
    puts(s_sygusInvTemplHelp.c_str());
    exit(1);
  } else {
    throw OptionException(std::string("unknown option for --sygus-inv-templ: `") +
                          optarg + "'.  Try --sygus-inv-templ help.");
  }
}

theory::quantifiers::MacrosQuantMode OptionsHandler::stringToMacrosQuantMode(std::string option, std::string optarg) throw(OptionException) {
  if(optarg == "all" ) {
    return theory::quantifiers::MACROS_QUANT_MODE_ALL;
  } else if(optarg == "ground") {
    return theory::quantifiers::MACROS_QUANT_MODE_GROUND;
  } else if(optarg == "ground-uf") {
    return theory::quantifiers::MACROS_QUANT_MODE_GROUND_UF;
  } else if(optarg ==  "help") {
    puts(s_macrosQuantHelp.c_str());
    exit(1);
  } else {
    throw OptionException(std::string("unknown option for --macros-quant-mode: `") +
                          optarg + "'.  Try --macros-quant-mode help.");
  }
}

theory::quantifiers::QuantDSplitMode OptionsHandler::stringToQuantDSplitMode(std::string option, std::string optarg) throw(OptionException) {
  if(optarg == "none" ) {
    return theory::quantifiers::QUANT_DSPLIT_MODE_NONE;
  } else if(optarg == "default") {
    return theory::quantifiers::QUANT_DSPLIT_MODE_DEFAULT;
  } else if(optarg == "agg") {
    return theory::quantifiers::QUANT_DSPLIT_MODE_AGG;
  } else if(optarg ==  "help") {
    puts(s_quantDSplitHelp.c_str());
    exit(1);
  } else {
    throw OptionException(std::string("unknown option for --quant-dsplit-mode: `") +
                          optarg + "'.  Try --quant-dsplit-mode help.");
  }
}

theory::quantifiers::QuantRepMode OptionsHandler::stringToQuantRepMode(std::string option, std::string optarg) throw(OptionException) {
  if(optarg == "none" ) {
    return theory::quantifiers::QUANT_REP_MODE_EE;
  } else if(optarg == "first" || optarg == "default") {
    return theory::quantifiers::QUANT_REP_MODE_FIRST;
  } else if(optarg == "depth") {
    return theory::quantifiers::QUANT_REP_MODE_DEPTH;
  } else if(optarg ==  "help") {
    puts(s_quantRepHelp.c_str());
    exit(1);
  } else {
    throw OptionException(std::string("unknown option for --quant-rep-mode: `") +
                          optarg + "'.  Try --quant-rep-mode help.");
  }
}


theory::quantifiers::FmfBoundMinMode OptionsHandler::stringToFmfBoundMinMode(std::string option, std::string optarg) throw(OptionException) {
  if(optarg == "none" ) {
    return theory::quantifiers::FMF_BOUND_MIN_NONE;
  } else if(optarg == "int" || optarg == "default") {
    return theory::quantifiers::FMF_BOUND_MIN_INT_RANGE;
  } else if(optarg == "setc" || optarg == "default") {
    return theory::quantifiers::FMF_BOUND_MIN_SET_CARD;
  } else if(optarg == "all") {
    return theory::quantifiers::FMF_BOUND_MIN_ALL;
  } else if(optarg ==  "help") {
    puts(s_fmfBoundMinModeModeHelp.c_str());
    exit(1);
  } else {
    throw OptionException(std::string("unknown option for --fmf-bound-min-mode: `") +
                          optarg + "'.  Try --fmf-bound-min-mode help.");
  }
}

// theory/bv/options_handlers.h
void OptionsHandler::abcEnabledBuild(std::string option, bool value) throw(OptionException) {
#ifndef CVC4_USE_ABC
  if(value) {
    std::stringstream ss;
    ss << "option `" << option << "' requires an abc-enabled build of CVC4; this binary was not built with abc support";
    throw OptionException(ss.str());
  }
#endif /* CVC4_USE_ABC */
}

void OptionsHandler::abcEnabledBuild(std::string option, std::string value) throw(OptionException) {
#ifndef CVC4_USE_ABC
  if(!value.empty()) {
    std::stringstream ss;
    ss << "option `" << option << "' requires an abc-enabled build of CVC4; this binary was not built with abc support";
    throw OptionException(ss.str());
  }
#endif /* CVC4_USE_ABC */
}

void OptionsHandler::satSolverEnabledBuild(std::string option,
                                           bool value) throw(OptionException) {
#ifndef CVC4_USE_CRYPTOMINISAT
  if(value) {
    std::stringstream ss;
    ss << "option `" << option << "' requires an cryptominisat-enabled build of CVC4; this binary was not built with cryptominisat support";
    throw OptionException(ss.str());
  }
#endif /* CVC4_USE_CRYPTOMINISAT */
}

void OptionsHandler::satSolverEnabledBuild(std::string option,
                                           std::string value) throw(OptionException) {
#ifndef CVC4_USE_CRYPTOMINISAT
  if(!value.empty()) {
    std::stringstream ss;
    ss << "option `" << option << "' requires an cryptominisat-enabled build of CVC4; this binary was not built with cryptominisat support";
    throw OptionException(ss.str());
  }
#endif /* CVC4_USE_CRYPTOMINISAT */
}

const std::string OptionsHandler::s_bvSatSolverHelp = "\
Sat solvers currently supported by the --bv-sat-solver option:\n\
\n\
minisat (default)\n\
\n\
cryptominisat\n\
";

theory::bv::SatSolverMode OptionsHandler::stringToSatSolver(std::string option,
                                                            std::string optarg) throw(OptionException) {
  if(optarg == "minisat") {
    return theory::bv::SAT_SOLVER_MINISAT;
  } else if(optarg == "cryptominisat") {
    
    if (options::incrementalSolving() &&
        options::incrementalSolving.wasSetByUser()) {
      throw OptionException(std::string("Cryptominsat does not support incremental mode. \n\
                                         Try --bv-sat-solver=minisat"));
    }

    if (options::bitblastMode() == theory::bv::BITBLAST_MODE_LAZY &&
        options::bitblastMode.wasSetByUser()) {
      throw OptionException(std::string("Cryptominsat does not support lazy bit-blsating. \n\
                                         Try --bv-sat-solver=minisat"));
    }
    if (!options::bitvectorToBool.wasSetByUser()) {
      options::bitvectorToBool.set(true);
    }

    // if (!options::bvAbstraction.wasSetByUser() &&
    //     !options::skolemizeArguments.wasSetByUser()) {
    //   options::bvAbstraction.set(true);
    //   options::skolemizeArguments.set(true); 
    // }
    return theory::bv::SAT_SOLVER_CRYPTOMINISAT;
  } else if(optarg == "help") {
    puts(s_bvSatSolverHelp.c_str());
    exit(1);
  } else {
    throw OptionException(std::string("unknown option for --bv-sat-solver: `") +
                          optarg + "'.  Try --bv-sat-solver=help.");
  }
}

const std::string OptionsHandler::s_bitblastingModeHelp = "\
Bit-blasting modes currently supported by the --bitblast option:\n\
\n\
lazy (default)\n\
+ Separate boolean structure and term reasoning between the core\n\
  SAT solver and the bv SAT solver\n\
\n\
eager\n\
+ Bitblast eagerly to bv SAT solver\n\
";

theory::bv::BitblastMode OptionsHandler::stringToBitblastMode(std::string option, std::string optarg) throw(OptionException) {
  if(optarg == "lazy") {
    if (!options::bitvectorPropagate.wasSetByUser()) {
      options::bitvectorPropagate.set(true);
    }
    if (!options::bitvectorEqualitySolver.wasSetByUser()) {
      options::bitvectorEqualitySolver.set(true);
    }
    if (!options::bitvectorEqualitySlicer.wasSetByUser()) {
      if (options::incrementalSolving() ||
          options::produceModels()) {
        options::bitvectorEqualitySlicer.set(theory::bv::BITVECTOR_SLICER_OFF);
      } else {
        options::bitvectorEqualitySlicer.set(theory::bv::BITVECTOR_SLICER_AUTO);
      }
    }

    if (!options::bitvectorInequalitySolver.wasSetByUser()) {
      options::bitvectorInequalitySolver.set(true);
    }
    if (!options::bitvectorAlgebraicSolver.wasSetByUser()) {
      options::bitvectorAlgebraicSolver.set(true);
    }
    return theory::bv::BITBLAST_MODE_LAZY;
  } else if(optarg == "eager") {

    if (options::incrementalSolving() &&
        options::incrementalSolving.wasSetByUser()) {
      throw OptionException(std::string("Eager bit-blasting does not currently support incremental mode. \n\
                                         Try --bitblast=lazy"));
    }
    if (!options::bitvectorToBool.wasSetByUser()) {
      options::bitvectorToBool.set(true);
    }

    if (!options::bvAbstraction.wasSetByUser() &&
        !options::skolemizeArguments.wasSetByUser()) {
      options::bvAbstraction.set(true);
      options::skolemizeArguments.set(true);
    }
    return theory::bv::BITBLAST_MODE_EAGER;
  } else if(optarg == "help") {
    puts(s_bitblastingModeHelp.c_str());
    exit(1);
  } else {
    throw OptionException(std::string("unknown option for --bitblast: `") +
                          optarg + "'.  Try --bitblast=help.");
  }
}

const std::string OptionsHandler::s_bvSlicerModeHelp = "\
Bit-vector equality slicer modes supported by the --bv-eq-slicer option:\n\
\n\
auto (default)\n\
+ Turn slicer on if input has only equalities over core symbols\n\
\n\
on\n\
+ Turn slicer on\n\
\n\
off\n\
+ Turn slicer off\n\
";

theory::bv::BvSlicerMode OptionsHandler::stringToBvSlicerMode(std::string option, std::string optarg) throw(OptionException) {
  if(optarg == "auto") {
    return theory::bv::BITVECTOR_SLICER_AUTO;
  } else if(optarg == "on") {
    return theory::bv::BITVECTOR_SLICER_ON;
  } else if(optarg == "off") {
    return theory::bv::BITVECTOR_SLICER_OFF;
  } else if(optarg == "help") {
    puts(s_bitblastingModeHelp.c_str());
    exit(1);
  } else {
    throw OptionException(std::string("unknown option for --bv-eq-slicer: `") +
                          optarg + "'.  Try --bv-eq-slicer=help.");
  }
}

void OptionsHandler::setBitblastAig(std::string option, bool arg) throw(OptionException) {
  if(arg) {
    if(options::bitblastMode.wasSetByUser()) {
      if(options::bitblastMode() != theory::bv::BITBLAST_MODE_EAGER) {
        throw OptionException("bitblast-aig must be used with eager bitblaster");
      }
    } else {
      theory::bv::BitblastMode mode = stringToBitblastMode("", "eager");
      options::bitblastMode.set(mode);
    }
    if(!options::bitvectorAigSimplifications.wasSetByUser()) {
      options::bitvectorAigSimplifications.set("balance;drw");
    }
  }
}

// theory/uf/options_handlers.h
const std::string OptionsHandler::s_ufssModeHelp = "\
UF strong solver options currently supported by the --uf-ss option:\n\
\n\
full \n\
+ Default, use uf strong solver to find minimal models for uninterpreted sorts.\n\
\n\
no-minimal \n\
+ Use uf strong solver to shrink model sizes, but do no enforce minimality.\n\
\n\
none \n\
+ Do not use uf strong solver to shrink model sizes. \n\
\n\
";

theory::uf::UfssMode OptionsHandler::stringToUfssMode(std::string option, std::string optarg) throw(OptionException) {
  if(optarg ==  "default" || optarg == "full" ) {
    return theory::uf::UF_SS_FULL;
  } else if(optarg == "no-minimal") {
    return theory::uf::UF_SS_NO_MINIMAL;
  } else if(optarg == "none") {
    return theory::uf::UF_SS_NONE;
  } else if(optarg ==  "help") {
    puts(s_ufssModeHelp.c_str());
    exit(1);
  } else {
    throw OptionException(std::string("unknown option for --uf-ss: `") +
                          optarg + "'.  Try --uf-ss help.");
  }
}



// theory/options_handlers.h
const std::string OptionsHandler::s_theoryOfModeHelp = "\
TheoryOf modes currently supported by the --theoryof-mode option:\n\
\n\
type (default) \n\
+ type variables, constants and equalities by type\n\
\n\
term \n\
+ type variables as uninterpreted, equalities by the parametric theory\n\
";

theory::TheoryOfMode OptionsHandler::stringToTheoryOfMode(std::string option, std::string optarg) {
  if(optarg == "type") {
    return theory::THEORY_OF_TYPE_BASED;
  } else if(optarg == "term") {
    return theory::THEORY_OF_TERM_BASED;
  } else if(optarg == "help") {
    puts(s_theoryOfModeHelp.c_str());
    exit(1);
  } else {
    throw OptionException(std::string("unknown option for --theoryof-mode: `") +
                          optarg + "'.  Try --theoryof-mode help.");
  }
}

// theory/options_handlers.h
std::string OptionsHandler::handleUseTheoryList(std::string option, std::string optarg) {
  std::string currentList = options::useTheoryList();
  if(currentList.empty()){
    return optarg;
  } else {
    return currentList +','+ optarg;
  }
}

void OptionsHandler::notifyUseTheoryList(std::string option) {
  d_options->d_useTheoryListListeners.notify();
}



// printer/options_handlers.h
const std::string OptionsHandler::s_modelFormatHelp = "\
Model format modes currently supported by the --model-format option:\n\
\n\
default \n\
+ Print model as expressions in the output language format.\n\
\n\
table\n\
+ Print functional expressions over finite domains in a table format.\n\
";

const std::string OptionsHandler::s_instFormatHelp = "\
Inst format modes currently supported by the --model-format option:\n\
\n\
default \n\
+ Print instantiations as a list in the output language format.\n\
\n\
szs\n\
+ Print instantiations as SZS compliant proof.\n\
";

ModelFormatMode OptionsHandler::stringToModelFormatMode(std::string option, std::string optarg) throw(OptionException) {
  if(optarg == "default") {
    return MODEL_FORMAT_MODE_DEFAULT;
  } else if(optarg == "table") {
    return MODEL_FORMAT_MODE_TABLE;
  } else if(optarg == "help") {
    puts(s_modelFormatHelp.c_str());
    exit(1);
  } else {
    throw OptionException(std::string("unknown option for --model-format: `") +
                          optarg + "'.  Try --model-format help.");
  }
}

InstFormatMode OptionsHandler::stringToInstFormatMode(std::string option, std::string optarg) throw(OptionException) {
  if(optarg == "default") {
    return INST_FORMAT_MODE_DEFAULT;
  } else if(optarg == "szs") {
    return INST_FORMAT_MODE_SZS;
  } else if(optarg == "help") {
    puts(s_instFormatHelp.c_str());
    exit(1);
  } else {
    throw OptionException(std::string("unknown option for --inst-format: `") +
                          optarg + "'.  Try --inst-format help.");
  }
}


// decision/options_handlers.h
const std::string OptionsHandler::s_decisionModeHelp = "\
Decision modes currently supported by the --decision option:\n\
\n\
internal (default)\n\
+ Use the internal decision heuristics of the SAT solver\n\
\n\
justification\n\
+ An ATGP-inspired justification heuristic\n\
\n\
justification-stoponly\n\
+ Use the justification heuristic only to stop early, not for decisions\n\
";

decision::DecisionMode OptionsHandler::stringToDecisionMode(std::string option, std::string optarg) throw(OptionException) {
  options::decisionStopOnly.set(false);

  if(optarg == "internal") {
    return decision::DECISION_STRATEGY_INTERNAL;
  } else if(optarg == "justification") {
    return decision::DECISION_STRATEGY_JUSTIFICATION;
  } else if(optarg == "justification-stoponly") {
    options::decisionStopOnly.set(true);
    return decision::DECISION_STRATEGY_JUSTIFICATION;
  } else if(optarg == "help") {
    puts(s_decisionModeHelp.c_str());
    exit(1);
  } else {
    throw OptionException(std::string("unknown option for --decision: `") +
                          optarg + "'.  Try --decision help.");
  }
}

decision::DecisionWeightInternal OptionsHandler::stringToDecisionWeightInternal(std::string option, std::string optarg) throw(OptionException) {
  if(optarg == "off")
    return decision::DECISION_WEIGHT_INTERNAL_OFF;
  else if(optarg == "max")
    return decision::DECISION_WEIGHT_INTERNAL_MAX;
  else if(optarg == "sum")
    return decision::DECISION_WEIGHT_INTERNAL_SUM;
  else if(optarg == "usr1")
    return decision::DECISION_WEIGHT_INTERNAL_USR1;
  else
    throw OptionException(std::string("--decision-weight-internal must be off, max or sum."));
}


// smt/options_handlers.h
const std::string OptionsHandler::s_simplificationHelp = "\
Simplification modes currently supported by the --simplification option:\n\
\n\
batch (default) \n\
+ save up all ASSERTions; run nonclausal simplification and clausal\n\
  (MiniSat) propagation for all of them only after reaching a querying command\n\
  (CHECKSAT or QUERY or predicate SUBTYPE declaration)\n\
\n\
none\n\
+ do not perform nonclausal simplification\n\
";



SimplificationMode OptionsHandler::stringToSimplificationMode(std::string option, std::string optarg) throw(OptionException) {
  if(optarg == "batch") {
    return SIMPLIFICATION_MODE_BATCH;
  } else if(optarg == "none") {
    return SIMPLIFICATION_MODE_NONE;
  } else if(optarg == "help") {
    puts(s_simplificationHelp.c_str());
    exit(1);
  } else {
    throw OptionException(std::string("unknown option for --simplification: `") +
                          optarg + "'.  Try --simplification help.");
  }
}


void OptionsHandler::setProduceAssertions(std::string option, bool value) throw() {
  options::produceAssertions.set(value);
  options::interactiveMode.set(value);
}


void OptionsHandler::proofEnabledBuild(std::string option, bool value) throw(OptionException) {
#ifndef CVC4_PROOF
  if(value) {
    std::stringstream ss;
    ss << "option `" << option << "' requires a proofs-enabled build of CVC4; this binary was not built with proof support";
    throw OptionException(ss.str());
  }
#endif /* CVC4_PROOF */
}

void OptionsHandler::notifyDumpToFile(std::string option) {
  d_options->d_dumpToFileListeners.notify();
}


void OptionsHandler::notifySetRegularOutputChannel(std::string option) {
  d_options->d_setRegularChannelListeners.notify();
}

void OptionsHandler::notifySetDiagnosticOutputChannel(std::string option) {
  d_options->d_setDiagnosticChannelListeners.notify();
}


std::string OptionsHandler::checkReplayFilename(std::string option, std::string optarg) {
#ifdef CVC4_REPLAY
  if(optarg == "") {
    throw OptionException (std::string("Bad file name for --replay"));
  } else {
    return optarg;
  }
#else /* CVC4_REPLAY */
  throw OptionException("The replay feature was disabled in this build of CVC4.");
#endif /* CVC4_REPLAY */
}

void OptionsHandler::notifySetReplayLogFilename(std::string option) {
  d_options->d_setReplayFilenameListeners.notify();
}

void OptionsHandler::statsEnabledBuild(std::string option, bool value) throw(OptionException) {
#ifndef CVC4_STATISTICS_ON
  if(value) {
    std::stringstream ss;
    ss << "option `" << option << "' requires a statistics-enabled build of CVC4; this binary was not built with statistics support";
    throw OptionException(ss.str());
  }
#endif /* CVC4_STATISTICS_ON */
}

void OptionsHandler::threadN(std::string option) {
  throw OptionException(option + " is not a real option by itself.  Use e.g. --thread0=\"--random-seed=10 --random-freq=0.02\" --thread1=\"--random-seed=20 --random-freq=0.05\"");
}

void OptionsHandler::notifyDumpMode(std::string option) throw(OptionException) {
  d_options->d_setDumpModeListeners.notify();
}


// expr/options_handlers.h
void OptionsHandler::setDefaultExprDepthPredicate(std::string option, int depth) {
  if(depth < -1) {
    throw OptionException("--default-expr-depth requires a positive argument, or -1.");
  }
}

void OptionsHandler::setDefaultDagThreshPredicate(std::string option, int dag) {
  if(dag < 0) {
    throw OptionException("--default-dag-thresh requires a nonnegative argument.");
  }
}

void OptionsHandler::notifySetDefaultExprDepth(std::string option) {
  d_options->d_setDefaultExprDepthListeners.notify();
}

void OptionsHandler::notifySetDefaultDagThresh(std::string option) {
  d_options->d_setDefaultDagThreshListeners.notify();
}

void OptionsHandler::notifySetPrintExprTypes(std::string option) {
  d_options->d_setPrintExprTypesListeners.notify();
}


// main/options_handlers.h
void OptionsHandler::showConfiguration(std::string option) {
  fputs(Configuration::about().c_str(), stdout);
  printf("\n");
  printf("version    : %s\n", Configuration::getVersionString().c_str());
  if(Configuration::isGitBuild()) {
    const char* branchName = Configuration::getGitBranchName();
    if(*branchName == '\0') {
      branchName = "-";
    }
    printf("scm        : git [%s %s%s]\n",
           branchName,
           std::string(Configuration::getGitCommit()).substr(0, 8).c_str(),
           Configuration::hasGitModifications() ?
             " (with modifications)" : "");
  } else if(Configuration::isSubversionBuild()) {
    printf("scm        : svn [%s r%u%s]\n",
           Configuration::getSubversionBranchName(),
           Configuration::getSubversionRevision(),
           Configuration::hasSubversionModifications() ?
             " (with modifications)" : "");
  } else {
    printf("scm        : no\n");
  }
  printf("\n");
  printf("library    : %u.%u.%u\n",
         Configuration::getVersionMajor(),
         Configuration::getVersionMinor(),
         Configuration::getVersionRelease());
  printf("\n");
  printf("debug code : %s\n", Configuration::isDebugBuild() ? "yes" : "no");
  printf("statistics : %s\n", Configuration::isStatisticsBuild() ? "yes" : "no");
  printf("replay     : %s\n", Configuration::isReplayBuild() ? "yes" : "no");
  printf("tracing    : %s\n", Configuration::isTracingBuild() ? "yes" : "no");
  printf("dumping    : %s\n", Configuration::isDumpingBuild() ? "yes" : "no");
  printf("muzzled    : %s\n", Configuration::isMuzzledBuild() ? "yes" : "no");
  printf("assertions : %s\n", Configuration::isAssertionBuild() ? "yes" : "no");
  printf("proof      : %s\n", Configuration::isProofBuild() ? "yes" : "no");
  printf("coverage   : %s\n", Configuration::isCoverageBuild() ? "yes" : "no");
  printf("profiling  : %s\n", Configuration::isProfilingBuild() ? "yes" : "no");
  printf("competition: %s\n", Configuration::isCompetitionBuild() ? "yes" : "no");
  printf("\n");
  printf("cudd       : %s\n", Configuration::isBuiltWithCudd() ? "yes" : "no");
  printf("cln        : %s\n", Configuration::isBuiltWithCln() ? "yes" : "no");
  printf("gmp        : %s\n", Configuration::isBuiltWithGmp() ? "yes" : "no");
  printf("glpk       : %s\n", Configuration::isBuiltWithGlpk() ? "yes" : "no");
  printf("abc        : %s\n", Configuration::isBuiltWithAbc() ? "yes" : "no");
  printf("readline   : %s\n", Configuration::isBuiltWithReadline() ? "yes" : "no");
  printf("tls        : %s\n", Configuration::isBuiltWithTlsSupport() ? "yes" : "no");
  exit(0);
}

void OptionsHandler::showDebugTags(std::string option) {
  if(Configuration::isDebugBuild() && Configuration::isTracingBuild()) {
    printf("available tags:");
    unsigned ntags = Configuration::getNumDebugTags();
    char const* const* tags = Configuration::getDebugTags();
    for(unsigned i = 0; i < ntags; ++ i) {
      printf(" %s", tags[i]);
    }
    printf("\n");
  } else if(! Configuration::isDebugBuild()) {
    throw OptionException("debug tags not available in non-debug builds");
  } else {
    throw OptionException("debug tags not available in non-tracing builds");
  }
  exit(0);
}

void OptionsHandler::showTraceTags(std::string option) {
  if(Configuration::isTracingBuild()) {
    printf("available tags:");
    unsigned ntags = Configuration::getNumTraceTags();
    char const* const* tags = Configuration::getTraceTags();
    for (unsigned i = 0; i < ntags; ++ i) {
      printf(" %s", tags[i]);
    }
    printf("\n");
  } else {
    throw OptionException("trace tags not available in non-tracing build");
  }
  exit(0);
}


OutputLanguage OptionsHandler::stringToOutputLanguage(std::string option, std::string optarg) throw(OptionException) {
  if(optarg == "help") {
    options::languageHelp.set(true);
    return language::output::LANG_AUTO;
  }

  try {
    return language::toOutputLanguage(optarg);
  } catch(OptionException& oe) {
    throw OptionException("Error in " + option + ": " + oe.getMessage() +
                          "\nTry --output-language help");
  }

  Unreachable();
}

InputLanguage OptionsHandler::stringToInputLanguage(std::string option, std::string optarg) throw(OptionException) {
  if(optarg == "help") {
    options::languageHelp.set(true);
    return language::input::LANG_AUTO;
  }

  try {
    return language::toInputLanguage(optarg);
  } catch(OptionException& oe) {
    throw OptionException("Error in " + option + ": " + oe.getMessage() + "\nTry --language help");
  }

  Unreachable();
}

/* options/base_options_handlers.h */
void OptionsHandler::setVerbosity(std::string option, int value) throw(OptionException) {
  if(Configuration::isMuzzledBuild()) {
    DebugChannel.setStream(&CVC4::null_os);
    TraceChannel.setStream(&CVC4::null_os);
    NoticeChannel.setStream(&CVC4::null_os);
    ChatChannel.setStream(&CVC4::null_os);
    MessageChannel.setStream(&CVC4::null_os);
    WarningChannel.setStream(&CVC4::null_os);
  } else {
    if(value < 2) {
      ChatChannel.setStream(&CVC4::null_os);
    } else {
      ChatChannel.setStream(&std::cout);
    }
    if(value < 1) {
      NoticeChannel.setStream(&CVC4::null_os);
    } else {
      NoticeChannel.setStream(&std::cout);
    }
    if(value < 0) {
      MessageChannel.setStream(&CVC4::null_os);
      WarningChannel.setStream(&CVC4::null_os);
    } else {
      MessageChannel.setStream(&std::cout);
      WarningChannel.setStream(&std::cerr);
    }
  }
}

void OptionsHandler::increaseVerbosity(std::string option) {
  options::verbosity.set(options::verbosity() + 1);
  setVerbosity(option, options::verbosity());
}

void OptionsHandler::decreaseVerbosity(std::string option) {
  options::verbosity.set(options::verbosity() - 1);
  setVerbosity(option, options::verbosity());
}


void OptionsHandler::addTraceTag(std::string option, std::string optarg) {
  if(Configuration::isTracingBuild()) {
    if(!Configuration::isTraceTag(optarg.c_str())) {

      if(optarg == "help") {
        printf("available tags:");
        unsigned ntags = Configuration::getNumTraceTags();
        char const* const* tags = Configuration::getTraceTags();
        for(unsigned i = 0; i < ntags; ++ i) {
          printf(" %s", tags[i]);
        }
        printf("\n");
        exit(0);
      }

      throw OptionException(std::string("trace tag ") + optarg +
                            std::string(" not available.") +
                            suggestTags(Configuration::getTraceTags(), optarg) );
    }
  } else {
    throw OptionException("trace tags not available in non-tracing builds");
  }
  Trace.on(optarg);
}

void OptionsHandler::addDebugTag(std::string option, std::string optarg) {
  if(Configuration::isDebugBuild() && Configuration::isTracingBuild()) {
    if(!Configuration::isDebugTag(optarg.c_str()) &&
       !Configuration::isTraceTag(optarg.c_str())) {

      if(optarg == "help") {
        printf("available tags:");
        unsigned ntags = Configuration::getNumDebugTags();
        char const* const* tags = Configuration::getDebugTags();
        for(unsigned i = 0; i < ntags; ++ i) {
          printf(" %s", tags[i]);
        }
        printf("\n");
        exit(0);
      }

      throw OptionException(std::string("debug tag ") + optarg +
                            std::string(" not available.") +
                            suggestTags(Configuration::getDebugTags(), optarg, Configuration::getTraceTags()) );
    }
  } else if(! Configuration::isDebugBuild()) {
    throw OptionException("debug tags not available in non-debug builds");
  } else {
    throw OptionException("debug tags not available in non-tracing builds");
  }
  Debug.on(optarg);
  Trace.on(optarg);
}




std::string OptionsHandler::suggestTags(char const* const* validTags, std::string inputTag,
                                           char const* const* additionalTags)
{
  DidYouMean didYouMean;

  const char* opt;
  for(size_t i = 0; (opt = validTags[i]) != NULL; ++i) {
    didYouMean.addWord(validTags[i]);
  }
  if(additionalTags != NULL) {
    for(size_t i = 0; (opt = additionalTags[i]) != NULL; ++i) {
      didYouMean.addWord(additionalTags[i]);
    }
  }

  return  didYouMean.getMatchAsString(inputTag);
}




}/* CVC4::options namespace */
}/* CVC4 namespace */
