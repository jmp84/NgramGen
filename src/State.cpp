/*
 * State.cpp
 *
 *  Created on: 22 Oct 2012
 *      Author: jmp84
 */

#include <iostream>

#include "Ngram.h"
#include "State.h"
#include "Types.h"

namespace cam {
namespace eng {
namespace gen {

State::State(StateKey* stateKey, const Cost cost,
             const std::vector<Arc>& incomingArcs) :
                 stateKey_(stateKey), cost_(cost), incomingArcs_(incomingArcs) {}

bool State::operator<(const State& other) const {
  return (cost_ < other.cost_);
}

bool State::canApply(const std::vector<int>& ngram,
                     const Coverage& coverage) const {
  // check that the ngram has a disjoint coverage with the current coverage
  int ol = overlap(coverage);
  if (ol > 0) {
    return false;
  }
  // check that if the ngram starts with start-of-sentence, then the current
  // state is initial (that is, has an empty coverage)
  if (ngram[0] == 1 && stateKey_->coverage_.any()) {
    return false;
  }
  // checks that if the ngram ends with end-of-sentence, then the resulting
  // coverage will cover all words
  int sizeNextCoverage =
      stateKey_->coverage_.count() + coverage.count() - ol;
  if (ngram[ngram.size() - 1] == 2 && sizeNextCoverage < coverage.size()) {
    return false;
  }
  return true;
}

int State::overlap(const Coverage& coverage) const {
  Coverage intersection =
      stateKey_->coverage_ & coverage;
  return intersection.count();
}

Cost State::cost() const {
  return cost_;
}

void State::setCost(const Cost cost) {
  cost_ = cost;
}

const Coverage& State::coverage() const {
  return stateKey_->coverage_;
}

const StateKey* State::stateKey() const {
  return stateKey_;
}

lm::ngram::State State::getKenlmState() const {
  return stateKey_->kenlmState_;
}

const std::vector<Arc>& State::incomingArcs() const {
  return incomingArcs_;
}

void State::addArc(const Arc& arc) {
  incomingArcs_.push_back(arc);
}

} // namespace gen
} // namespace eng
} // namespace cam
