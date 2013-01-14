/*
 * FeatureCreatorImpl.h
 *
 *  Created on: 9 Jan 2013
 *      Author: jmp84
 */

#ifndef FEATURECREATORIMPL_H_
#define FEATURECREATORIMPL_H_

#include "FeatureCreator.h"

namespace cam {
namespace eng {
namespace gen {

/**
 * Feature creator template class. The template is a feature class. This is part
 * of the feature pattern (or registrar/exemplar pattern).
 */
template <class T>
class FeatureCreatorImpl : public FeatureCreator {
public:
  /**
   * Constructor. Calls the parent constructor for registration, which register
   * the "this" pointer.
   * @param name The feature name.
   */
  FeatureCreatorImpl(const std::string& name);

  /**
   * Creates a feature.
   * @return The feature object.
   */
  virtual boost::shared_ptr<Feature> createFeature() const;
};

template <class T>
FeatureCreatorImpl<T>::FeatureCreatorImpl(const std::string& name) :
  FeatureCreator(name) {}

template <class T>
boost::shared_ptr<Feature> FeatureCreatorImpl<T>::createFeature() const {
  return boost::shared_ptr<Feature>(new T);
}

} // namespace gen
} // namespace eng
} // namespace cam

#endif /* FEATURECREATORIMPL_H_ */
