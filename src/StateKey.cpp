/*
 * StateKey.cpp
 *
 *  Created on: 22 Oct 2012
 *      Author: jmp84
 */

#include "StateKey.h"

#include <iostream>
#include <boost/functional/hash.hpp>

namespace boost {
template <typename B, typename A>
std::size_t hash_value(const dynamic_bitset<B, A>& bs) {
  return hash_value(bs.m_bits);
}
} // namespace boost

namespace cam {
namespace eng {
namespace gen {

StateKey::StateKey(const Coverage& coverage,
                   const lm::ngram::State& kenlmState) :
                       coverage_(coverage), kenlmState_(kenlmState) {}

bool StateKey::operator==(const StateKey& other) const {
  return ((coverage_ == other.coverage_) && (kenlmState_ == other.kenlmState_));
}

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

std::size_t hash_value(const StateKey& stateKey) {
  std::size_t seed = 0;
  boost::hash_combine(seed, stateKey.coverage_);
  boost::hash_combine(seed, stateKey.kenlmState_);
  return seed;
}

} // namespace gen
} // namespace eng
} // namespace cam
