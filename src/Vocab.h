/*
 * Vocab.h
 *
 *  Created on: 14 Dec 2012
 *      Author: jmp84
 */

#ifndef VOCAB_H_
#define VOCAB_H_

#include <string>
#include <vector>

namespace cam {
namespace eng {
namespace gen {

/**
 * Deals with mapping between words and ids.
 */
class Vocab {
public:
  /**
   * Loads the word/id mapping from a file.
   * @param fileName The file name.
   */
  void loadVocab(const std::string& fileName);

  /**
   * Looks up a word
   * @param wordId The word id to be looked up.
   * @return The word corresponding to the word id.
   */
  const std::string& getWord(const int wordId) const;

private:
  /** Stores the word/id mapping. Indices are ids, elements are words. */
  std::vector<std::string> vocab_;
};

} // namespace gen
} // namespace eng
} // namespace cam

#endif /* VOCAB_H_ */
