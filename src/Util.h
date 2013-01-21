/*
 * Util.h
 *
 *  Created on: 17 Jan 2013
 *      Author: jmp84
 */

#ifndef UTIL_H_
#define UTIL_H_

#include <boost/lexical_cast.hpp>
#include <lm/model.hh>

/**
 * Utility functions.
 */
namespace cam {
namespace eng {
namespace gen {

// start and end-of-sentence symbols
enum {
  STARTSENTENCE = 1,
  ENDSENTENCE = 2
};

/**
 * Utility to compute the index in kenlm vocab from an integer id. This is a
 * bit inefficient. It should be possible to reuse the id directly.
 * @param vocab The kenlm language model internal vocabulary
 * @param id The id corresponding to a word via our own wordmap
 * @return The index in kenlm vocab corresponding to the id.
 */
const lm::WordIndex index(const lm::ngram::Vocabulary& vocab, int id);

} // namespace gen
} // namespace eng
} // namespace cam

#endif /* UTIL_H_ */
