/*
 * Util.cpp
 *
 *  Created on: 17 Jan 2013
 *      Author: jmp84
 */

#include "Util.h"

namespace cam {
namespace eng {
namespace gen {

const lm::WordIndex index(
    const lm::ngram::Vocabulary& vocab, int id) {
  if (id == ENDSENTENCE) {
    return vocab.Index("</s>");
  }
  if (id == STARTSENTENCE) {
    return vocab.Index("<s>");
  }
  return vocab.Index(boost::lexical_cast<std::string>(id));
}

} // namespace gen
} // namespace eng
} // namespace cam
