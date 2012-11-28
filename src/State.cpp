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

State::State(StateId stateId, StateKey* stateKey, const Cost cost)
  : stateId_(stateId), stateKey_(stateKey), cost_(cost) {}

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

} // namespace gen
} // namespace eng
} // namespace cam
