/*
 * Arc.cpp
 *
 *  Created on: 24 Oct 2012
 *      Author: jmp84
 */

#include "Arc.h"

namespace cam {
namespace eng {
namespace gen {

Arc::Arc(State* origin, std::vector<int>* ngram, const Cost cost) :
    origin_(origin), ngram_(ngram), cost_(cost) {}

const State* Arc::state() const {
  return origin_;
}

const std::vector<int>* Arc::ngram() const {
  return ngram_;
}

const Cost Arc::cost() const {
  return cost_;
}

} // namespace gen
} // namespace eng
} // namespace cam
