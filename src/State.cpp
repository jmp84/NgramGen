/*
 * State.cpp
 *
 *  Created on: 22 Oct 2012
 *      Author: jmp84
 */

#include "State.h"

#include <iostream>

#include "StateKey.h"
#include "Types.h"

namespace cam {
namespace eng {
namespace gen {

State::State(const StateId stateId, StateKey* stateKey, const Cost cost,
             bool hasInput)
  : stateId_(stateId), stateKey_(stateKey), cost_(cost), hasInput_(hasInput) {}

State::~State() {
  delete stateKey_;
}

bool State::isInitial() const {
  return (stateKey_->coverage_.none());
}

const fst::StdArc::StateId State::stateId() const {
  return stateId_;
}

const Cost State::cost() const {
  return cost_;
}

const Coverage& State::coverage() const {
  return stateKey_->coverage_;
}

const StateKey* State::stateKey() const {
  return stateKey_;
}

const lm::ngram::State& State::getKenlmState() const {
  return stateKey_->kenlmState_;
}

const bool State::hasInput() const {
  return hasInput_;
}

void State::setHasInput(bool hasInput) {
  hasInput_ = hasInput;
}

} // namespace gen
} // namespace eng
} // namespace cam
