/*
 * Weights.cpp
 *
 *  Created on: 10 Jan 2013
 *      Author: jmp84
 */

#include "Weights.h"
#include <glog/logging.h>

namespace cam {
namespace eng {
namespace gen {

const float Weights::getWeight(const std::string& name) const {
  if (name == "lm") {
    return 1;
  }
  std::map<std::string, float>::const_iterator findWeight =
      weights_.find(name);
  CHECK(findWeight != weights_.end()) <<
      "Weight not found for feature: " << name;
  return findWeight->second;
}

void Weights::addWeight(const std::string& name, const float weight) {
  std::map<std::string, float>::const_iterator findName =
      weights_.find(name);
  CHECK(findName == weights_.end()) << "Feature " << name << " already has a "
      "weight.";
  weights_[name] = weight;
}

void Weights::clear() {
  weights_.clear();
}

} // namespace gen
} // namespace eng
} // namespace cam
