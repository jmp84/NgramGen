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


Arc::Arc(const State* origin, const Ngram* ngram) :
    origin_(const_cast<State*>(origin)), ngram_(const_cast<Ngram*>(ngram)) {}

const Ngram* Arc::ngram() const {
  return ngram_;
}

} // namespace gen
} // namespace eng
} // namespace cam
