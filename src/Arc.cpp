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


Arc::Arc(State* origin, std::vector<int>* ngram) :
    origin_(origin), ngram_(ngram) {}

const State* Arc::state() const {
  return origin_;
}

const std::vector<int>* Arc::ngram() const {
  return ngram_;
}

} // namespace gen
} // namespace eng
} // namespace cam
