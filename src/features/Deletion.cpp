/*
 * Deletion.cpp
 *
 *  Created on: 3 Jul 2013
 *      Author: jmp84
 */

#include "features/Deletion.h"

namespace cam {
namespace eng {
namespace gen {

/*virtual*/ Deletion::~Deletion() {}

/*virtual*/ const float Deletion::getValue(const Ngram& rule) const {
  return 0;
}

/*virtual*/ const float Deletion::getValueDeletion(const Ngram& rule) const /*override*/ {
  return 1;
}

/*static*/ const FeatureCreatorImpl<Deletion> Deletion::creator("deletion");

} // namespace gen
} // namespace eng
} // namespace cam
