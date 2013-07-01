/*
 * WordCount.h
 *
 *  Created on: 26 Jun 2013
 *      Author: jmp84
 */

#ifndef WORDCOUNT_H_
#define WORDCOUNT_H_

#include "features/Feature.h"
#include "features/FeatureCreatorImpl.h"

namespace cam {
namespace eng {
namespace gen {

/**
 * Word count feature. Returns the size of the ngram.
 */
class WordCount : public Feature {
public:
  virtual ~WordCount();
  virtual const float getValue(const Ngram& rule) const;
private:
  static const FeatureCreatorImpl<WordCount> creator;
};

} // namespace gen
} // namespace eng
} // namespace cam

#endif /* WORDCOUNT_H_ */
