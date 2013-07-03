/*
 * Deletion.h
 *
 *  Created on: 3 Jul 2013
 *      Author: jmp84
 */

#ifndef DELETION_H_
#define DELETION_H_

#include "features/Feature.h"
#include "features/FeatureCreatorImpl.h"

namespace cam {
namespace eng {
namespace gen {

/**
 * Deletion feature. Returns 1.
 */
class Deletion : public Feature {
public:
  virtual ~Deletion();
  virtual const float getValue(const Ngram& rule) const;
  virtual const float getValueDeletion(const Ngram& rule) const /*override*/;
private:
  static const FeatureCreatorImpl<Deletion> creator;
};

} // namespace gen
} // namespace eng
} // namespace cam

#endif /* DELETION_H_ */
