/*
 * RuleCostComputer.cpp
 *
 *  Created on: 10 Jan 2013
 *      Author: jmp84
 */

#include "features/RuleCostComputer.h"

#include <glog/logging.h>

#include "Util.h"
#include "State.h"

namespace cam {
namespace eng {
namespace gen {

Cost lmCost(const State& state, const Ngram& rule,
            lm::ngram::Model* languageModel,
            lm::ngram::State* nextKenlmState) {
  Cost res = 0;
  lm::ngram::State startKenlmStateTemp;
  const lm::ngram::Vocabulary& vocab = languageModel->GetVocabulary();
  for (int i = 0; i < rule.size(); i++) {
    if (rule[i] == STARTSENTENCE) {
      CHECK_EQ(0, i) << "Ngram with a start-of-sentence marker in the middle.";
      startKenlmStateTemp = languageModel->BeginSentenceState();
      continue;
    }
    if (i == 0) {
      startKenlmStateTemp = state.getKenlmState();
    } else if (i > 1 || rule[0] != 1) {
      startKenlmStateTemp = *nextKenlmState;
    }
    // else startKenlmStateTemp has been set to
    // languageModel_->BeginSentenceState()
    res += languageModel->Score(
        startKenlmStateTemp, index(languageModel->GetVocabulary(), rule[i]),
        *nextKenlmState);
  }
  return res * (-log(10));
}

} // namespace gen
} // namespace eng
} // namespace cam
