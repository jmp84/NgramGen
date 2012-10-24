/*
 * Ngram.cpp
 *
 *  Created on: 22 Oct 2012
 *      Author: jmp84
 */

#include "Ngram.h"

namespace cam {
namespace eng {
namespace gen {

Ngram::Ngram(const Coverage& coverage) : coverage_(coverage) {}

const Coverage& Ngram::coverage() const {
  return coverage_;
}

const std::vector<int>& Ngram::ngram() const {
  return ngram_;
}

} // namespace gen
} // namespace eng
} // namespace cam
