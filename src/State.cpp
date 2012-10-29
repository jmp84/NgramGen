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

State::State(StateKey* stateKey) : stateKey_(stateKey), cost_(0) {}

State::State(StateKey* stateKey, Cost cost) :
    stateKey_(stateKey), cost_(cost) {}

bool State::operator<(const State& other) const {
  return (cost_ < other.cost_);
}

int State::overlap(const Ngram& ngram) const {
  Coverage intersection =
      stateKey_->coverage_ & ngram.coverage();
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
