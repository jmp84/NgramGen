/*
 * FeatureFactory.h
 *
 *  Created on: 9 Jan 2013
 *      Author: jmp84
 */

#ifndef FEATUREFACTORY_H_
#define FEATUREFACTORY_H_

#include <map>
#include <boost/smart_ptr.hpp>

namespace cam {
namespace eng {
namespace gen {

class Feature;
class FeatureCreator;

/**
 * Factory holding a map between feature names and feature creator pointers.
 * This is part of the feature pattern (or the registrar/exemplar pattern). All
 * methods in the factory are static, making it effectively a singleton.
 */
class FeatureFactory {
public:
  /**
   * Creates a feature object.
   * @param name The feature name.
   * @return A pointer to the feature created.
   */
  static boost::shared_ptr<Feature> createFeature(const std::string& name);

  /**
   * Register a feature creator. Simply maps the feature name to a pointer
   * to a feature creator. Cannot use the name register because it is reserved.
   * @param name The feature name.
   * @param creator A pointer to the feature creator. We don't use smart
   * pointers here because this function is called with a this object.
   */
  static void registre(const std::string& name, FeatureCreator* creator);

private:
  /**
   * Get the table holding a mapping between feature names and feature creators.
   * @return A reference to the table.
   */
  static std::map<std::string, FeatureCreator*>& getTable();
};

} // namespace gen
} // namespace eng
} // namespace cam

#endif /* FEATUREFACTORY_H_ */
