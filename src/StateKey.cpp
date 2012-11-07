/*
 * StateKey.cpp
 *
 *  Created on: 22 Oct 2012
 *      Author: jmp84
 */

#include <iostream>

#include "StateKey.h"

namespace cam {
namespace eng {
namespace gen {

bool StateKey::operator<(const StateKey& other) const {
  if (coverage_ < other.coverage_) {
    return true;
  }
  if (coverage_ > other.coverage_) {
    return false;
  }
  return (kenlmState_ < other.kenlmState_);
}

const Coverage& StateKey::coverage() const {
  return coverage_;
}

StateKey::StateKey(const Coverage& coverage,
                   const lm::ngram::State& kenlmState) :
                       coverage_(coverage), kenlmState_(kenlmState) {}

} // namespace gen
} // namespace eng
} // namespace cam
