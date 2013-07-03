/*
 * Feature.cpp
 *
 *  Created on: 3 Jul 2013
 *      Author: jmp84
 */

#include "features/Feature.h"

namespace cam {
namespace eng {
namespace gen {

/*virtual*/ const float Feature::getValueDeletion(const Ngram& rule) const {
  return 0;
}

} // namespace gen
} // namespace eng
} // namespace cam
