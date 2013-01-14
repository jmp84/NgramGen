/*
 * FeatureCreator.h
 *
 *  Created on: 9 Jan 2013
 *      Author: jmp84
 */

#ifndef FEATURECREATOR_H_
#define FEATURECREATOR_H_

#include <string>
#include <boost/smart_ptr.hpp>

namespace cam {
namespace eng {
namespace gen {

class Feature;

/**
 * Base class for a feature creator. This class is part of what I call the
 * feature pattern. I am not sure what it is called officially, maybe registrar
 * pattern or examplar pattern. The problem is that there are many classes
 * (here: features) implementing an interface and we would like to access a
 * subset of them (the selected features) and possibly add new features in the
 * future. The first solution is to have a map that hard codes the mapping
 * between a feature name and a feature class. With this pattern, each feature
 * class automatically registers a feature creator into a map at runtime. See
 * http://blog.fourthwoods.com/2011/06/04/factory-design-pattern-in-c/ and
 * http://www.drdobbs.com/cpp/self-registering-objects-in-c/184410633
 */
class FeatureCreator {
public:
  /**
   * Constructor. This constructor is called by the subclass FeatureCreatorImpl.
   * It registers itself into a static map.
   * @param name The feature name. The name is used as a key in the static map
   * holding the creators.
   */
  FeatureCreator(const std::string& name);

  /**
   * Destructor.
   */
  virtual ~FeatureCreator();

  /**
   * Create a feature object. Implemented in the FeatureCreatorImpl class.
   * @return A pointer to the feature.
   */
  virtual boost::shared_ptr<Feature> createFeature() const = 0;
};

} // namespace gen
} // namespace eng
} // namespace cam

#endif /* FEATURECREATOR_H_ */
