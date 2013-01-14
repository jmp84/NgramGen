/*
 * FeatureCreator.cpp
 *
 *  Created on: 9 Jan 2013
 *      Author: jmp84
 */

#include "FeatureCreator.h"
#include "FeatureFactory.h"

namespace cam {
namespace eng {
namespace gen {

FeatureCreator::FeatureCreator(const std::string& name) {
  FeatureFactory::registre(name, this);
}

FeatureCreator::~FeatureCreator() {}

} // namespace gen
} // namespace eng
} // namespace cam
