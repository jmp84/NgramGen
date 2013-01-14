/*
 * RuleCostComputer.cpp
 *
 *  Created on: 10 Jan 2013
 *      Author: jmp84
 */

#include "RuleCostComputer.h"
#include "Feature.h"
#include "FeatureFactory.h"
#include "Weights.h"

namespace cam {
namespace eng {
namespace gen {

const float ruleCost(const Ngram& rule, const Weights& weights,
                     const std::vector<std::string>& featureNames) {
  float res = 0;
  for (int i = 0; i < featureNames.size(); ++i) {
    boost::shared_ptr<Feature> feature =
        FeatureFactory::createFeature(featureNames[i]);
    res += feature->getValue(rule) * weights.getWeight(featureNames[i]);
  }
  return res;
}

} // namespace gen
} // namespace eng
} // namespace cam
