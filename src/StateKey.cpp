/*
 * StateKey.cpp
 *
 *  Created on: 22 Oct 2012
 *      Author: jmp84
 */

#include "StateKey.h"

namespace cam {
namespace eng {
namespace gen {

StateKey::StateKey(const Coverage& coverage,
                   const lm::ngram::State& kenlmState) :
                       coverage_(coverage), kenlmState_(kenlmState) {}

} // namespace gen
} // namespace eng
} // namespace cam
