/*
 * Feature.h
 *
 *  Created on: 9 Jan 2013
 *      Author: jmp84
 */

#ifndef FEATURE_H_
#define FEATURE_H_

#include "../Types.h"

namespace cam {
namespace eng {
namespace gen {

/**
 * Base class for feature functions.
 */
class Feature {
public:
  virtual ~Feature() {}
  /**
   * Compute the feature value for a rule. A rule is an n-gram.
   * @param rule The rule.
   * @return The feature value computed on the rule.
   */
  virtual const float getValue(const Ngram& rule) const = 0;
};

} // namespace gen
} // namespace eng
} // namespace cam

#endif /* FEATURE_H_ */
