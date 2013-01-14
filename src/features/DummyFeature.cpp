/*
 * DummyFeature.cpp
 *
 *  Created on: 11 Jan 2013
 *      Author: jmp84
 */

#include "DummyFeature.h"

namespace cam {
namespace eng {
namespace gen {

/*virtual*/ DummyFeature::~DummyFeature() {}

/*virtual*/ const float DummyFeature::getValue(const Ngram& rule) const {
  return 1;
}

/*static*/ const FeatureCreatorImpl<DummyFeature> DummyFeature::creator("dummy");

} // namespace gen
} // namespace eng
} // namespace cam
