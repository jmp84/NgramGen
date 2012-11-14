/*
 * NgramLoader.h
 *
 *  Created on: 26 Oct 2012
 *      Author: jmp84
 */

#ifndef NGRAMLOADER_H_
#define NGRAMLOADER_H_

#include <map>
#include <vector>

#include "Types.h"

namespace cam {
namespace eng {
namespace gen {

/**
 * Loads ngrams from an ngram file. The format of the file is as follows:
 * X X1_X2 X1_X2
 * X X1_X2 X2_X1
 * X 00011111 6895_70_1696_6_2
 * X 00101111 5_6895_1696_6_2
 * X 00110111 5_70_1696_6_2
 * X 00111110 5_6895_70_1696_6
 * X 01010111 6316_70_1696_6_2
 */
class NgramLoader {
public:
  /**
   * Reads a file containing n-grams and coverages and loads them.
   * @param fileName
   */
  void loadNgram(const std::string& fileName);

  /**
   * Getter.
   * @return The ngrams.
   */
  const std::map<Ngram, std::vector<Coverage> >& ngrams() const;

private:
  /**
   * Map between an n-gram (sequence of words) and coverages. There may be
   * multiple coverages if a word in the input is repeated.
   */
  std::map<Ngram, std::vector<Coverage> > ngrams_;

};

} // namespace gen
} // namespace eng
} // namespace cam

#endif /* NGRAMLOADER_H_ */
