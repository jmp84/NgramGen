/*
 * RuleCount.cpp
 *
 *  Created on: 11 Jan 2013
 *      Author: jmp84
 */

#include "features/RuleCount.h"

namespace cam {
namespace eng {
namespace gen {

/*virtual*/ RuleCount::~RuleCount() {}

/*virtual*/ const float RuleCount::getValue(const Ngram& rule) const {
  return 1;
}

/*static*/ const FeatureCreatorImpl<RuleCount> RuleCount::creator("rule_count");

} // namespace gen
} // namespace eng
} // namespace cam
