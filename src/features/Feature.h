/*
 * Feature.h
 *
 *  Created on: 9 Jan 2013
 *      Author: jmp84
 */

#ifndef FEATURE_H_
#define FEATURE_H_

#include "Types.h"

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

  /**
   * Compute the deletion feature value for a rule. This value is zero for all
   * features but the deletion feature, so the default implementation returns
   * zero. This method is overriden in the Deletion class.
   * @param rule The rule.
   * @return The deletion feature value computed on the rule.
   */
  virtual const float getValueDeletion(const Ngram& rule) const;
};

} // namespace gen
} // namespace eng
} // namespace cam

#endif /* FEATURE_H_ */
