/*
 * WordCount.cpp
 *
 *  Created on: 26 Jun 2013
 *      Author: jmp84
 */

#include "features/WordCount.h"

namespace cam {
namespace eng {
namespace gen {

/*virtual*/ WordCount::~WordCount() {}

/*virtual*/ const float WordCount::getValue(const Ngram& rule) const {
  return rule.size();
}

/*static*/ const FeatureCreatorImpl<WordCount> WordCount::creator("word_count");

} // namespace gen
} // namespace eng
} // namespace cam
