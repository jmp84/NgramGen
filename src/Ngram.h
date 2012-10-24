/*
 * Ngram.h
 *
 *  Created on: 19 Oct 2012
 *      Author: jmp84
 */

#ifndef NGRAM_H_
#define NGRAM_H_

#include "Types.h"

namespace cam {
namespace eng {
namespace gen {

/**
 * An n-gram and its coverage vector of an input multiset.
 */
class Ngram {
public:
  /**
   * Constructor.
   * @param coverage
   */
  Ngram(const Coverage& coverage);

  /**
   * Getter.
   * @return the coverage.
   */
  const Coverage& coverage() const;

  /**
   * Getter.
   * @return the ngram.
   */
  const std::vector<int>& ngram() const;

private:
  /** N-gram. */
  std::vector<int> ngram_;
  /** Coverage vector. */
  Coverage coverage_;
};

} // namespace gen
} // namespace eng
} // namespace cam

#endif /* NGRAM_H_ */
