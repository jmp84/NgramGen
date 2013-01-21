/*
 * FeatureFactory.cpp
 *
 *  Created on: 9 Jan 2013
 *      Author: jmp84
 */

#include <glog/logging.h>
#include "features/FeatureFactory.h"
#include "features/FeatureCreator.h"

namespace cam {
namespace eng {
namespace gen {

/*static*/ boost::shared_ptr<Feature> FeatureFactory::createFeature(
    const std::string& name) {
  std::map<std::string, FeatureCreator*>::const_iterator findName =
      getTable().find(name);
  CHECK(findName != getTable().end()) << "Feature not found: " << name;
  return findName->second->createFeature();
}

/*static*/ void FeatureFactory::registre(
    const std::string& name, FeatureCreator* creator) {
  getTable()[name] = creator;
}

/*static*/ std::map<std::string, FeatureCreator*>& FeatureFactory::getTable() {
  // line below called only once.
  static std::map<std::string, FeatureCreator*> table;
  return table;
}

} // namespace gen
} // namespace eng
} // namespace cam
