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
   * Constructor.
   * @param inputSentence The input sentence to be reordered.
   */
  NgramLoader(const std::vector<int>& inputSentence);

  /**
   * Reads a file containing n-grams and coverages and loads them.
   * @param fileName The file name.
   * @param splitPositions The zero-based positions in the input that indicate
   * where to split it. Position p means that a new chunk must start at position
   * p. N-grams with coverage relevant to a specific chunk will be loaded for
   * that chunk.
   */
  void loadNgram(
      const std::string& fileName, const std::vector<int>& splitPositions,
      const std::vector<bool>& chunksToReorder);

  /**
   * Gets the n-grams for a specific zero-based chunk id.
   * @param chunkId The zero-based chunk id.
   * @return The n-grams for a specific zero-based chunk id.
   */
  const std::map<Ngram, std::vector<Coverage> >& ngrams(
      const int chunkId) const;

private:
  /**
   * Gets the chunk id where the coverage should belong. For example, if there
   * is no split, then all coverages belong to chunkId zero. For an input
   * "a b c d" and split positions <2>, then a coverage 1100 belongs to chunkId
   * zero, a coverage 0011 belongs to chunkId one, and a coverage 0110 belongs
   * nowhere (by convention, chunkId minus one).
   * @param coverage The coverage.
   * @param splitPositions The split positions (zero-based, position p indicates
   * that a new chunk should be started).
   * @return The chunk id that the coverage belongs two. Minus one if the
   * coverage overlaps multiple chunks.
   */
  int getChunkId(const Coverage& coverage,
                 const std::vector<int>& splitPositions);

  /**
   * List of maps between an n-gram (sequence of words) and coverages.
   * There may be multiple coverages if a word in the input is repeated.
   * The list may have more than one element if the input is chopped.
   */
  std::vector<std::map<Ngram, std::vector<Coverage> > > ngrams_;

  /** Input sentence to be reordered. */
  std::vector<int> inputSentence_;

};

} // namespace gen
} // namespace eng
} // namespace cam

#endif /* NGRAMLOADER_H_ */
