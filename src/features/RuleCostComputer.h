/*
 * RuleCostComputer.h
 *
 *  Created on: 10 Jan 2013
 *      Author: jmp84
 */

#ifndef RULECOSTCOMPUTER_H_
#define RULECOSTCOMPUTER_H_

#include "../Types.h"

namespace cam {
namespace eng {
namespace gen {

struct Weights;

/**
 * Computes the cost of a rule by doing the scalar product between the rule
 * features and the feature weights. Language model and other context dependent
 * features are not taken into account here.
 * @param rule The n-gram rule.
 * @param weights The feature weights.
 * @param featureNames The feature names.
 * @return The cost if the rule.
 */
const float ruleCost(const Ngram& rule, const Weights& weights,
                     const std::vector<std::string>& featureNames);

} // namespace gen
} // namespace eng
} // namespace cam

#endif /* RULECOSTCOMPUTER_H_ */
