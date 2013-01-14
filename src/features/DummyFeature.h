/*
 * DummyFeature.h
 *
 *  Created on: 9 Jan 2013
 *      Author: jmp84
 */

#ifndef DUMMYFEATURE_H_
#define DUMMYFEATURE_H_

#include "Feature.h"
#include "FeatureCreatorImpl.h"

namespace cam {
namespace eng {
namespace gen {

/**
 * Dummy feature to test the feature framework.
 */
class DummyFeature : public Feature {
public:
  virtual ~DummyFeature();
  virtual const float getValue(const Ngram& rule) const;
private:
  static const FeatureCreatorImpl<DummyFeature> creator;
};

} // namespace gen
} // namespace eng
} // namespace cam

#endif /* DUMMYFEATURE_H_ */
