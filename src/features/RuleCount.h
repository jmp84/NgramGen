/*
 * RuleCount.h
 *
 *  Created on: 9 Jan 2013
 *      Author: jmp84
 */

#ifndef RULECOUNT_H_
#define RULECOUNT_H_

#include "features/Feature.h"
#include "features/FeatureCreatorImpl.h"

namespace cam {
namespace eng {
namespace gen {

/**
 * Rule count feature. Simply returns 1.
 */
class RuleCount : public Feature {
public:
  virtual ~RuleCount();
  virtual const float getValue(const Ngram& rule) const;
private:
  static const FeatureCreatorImpl<RuleCount> creator;
};

} // namespace gen
} // namespace eng
} // namespace cam

#endif /* RULECOUNT_H_ */
