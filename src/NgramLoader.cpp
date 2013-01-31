/*
 * NgramLoader.cpp
 *
 *  Created on: 26 Oct 2012
 *      Author: jmp84
 */

#include "NgramLoader.h"

#include <fstream>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <glog/logging.h>

namespace cam {
namespace eng {
namespace gen {

void NgramLoader::loadNgram(const std::string& fileName,
                            const std::vector<int>& splitPositions) {
  // the "+ 1" is because splitPositions indicates positions where to split
  // whereas ngrams_ has a size which is the number of chunks.
  ngrams_.resize(splitPositions.size() + 1);
  std::ifstream file(fileName.c_str());
  CHECK(file.is_open()) << "Cannot open file " << fileName;
  std::string line;
  // skip the first two lines which are the ITG rules
  std::getline(file, line);
  std::getline(file, line);
  std::vector<std::string> parts;
  std::vector<std::string> ngramString;
  Ngram ngramInt;
  while (std::getline(file, line)) {
    boost::split(parts, line, boost::is_any_of(" "));
    CHECK_LE(3, parts.size()) << "Wrong format, should have at least 3 parts: "
        << line;
    Coverage coverage(parts[1]);
    // chunkId is the chunk id where the coverage should belong. For example, if
    // there is no split, then all coverages belong to chunkId zero. For an
    // input "a b c d" and split positions <2>, then a coverage 1100 belongs to
    // chunkId zero, a coverage 0011 belongs to chunkId one, and a coverage 0110
    // belongs nowhere (by convention, chunkId minus one).
    int chunkId = getChunkId(coverage, splitPositions);
    if (chunkId >= 0) {
      boost::split(ngramString, parts[2], boost::is_any_of("_"));
      ngramInt.resize(ngramString.size());
      std::transform(ngramString.begin(), ngramString.end(), ngramInt.begin(),
                     boost::lexical_cast<int, std::string>);
      ngrams_[chunkId][ngramInt].push_back(coverage);
    }
  }
}

const std::map<Ngram, std::vector<Coverage> >& NgramLoader::ngrams(
    const int chunkId) const {
  CHECK_LT(chunkId, ngrams_.size()) << "Invalid chunk id " << chunkId << ". "
      "Must be less than the size of the number of chunks: " << ngrams_.size();
  return ngrams_[chunkId];
}

int NgramLoader::getChunkId(const Coverage& coverage,
                            const std::vector<int>& splitPositions) {
  // if there is no split, then all coverages belong to chunkId zero.
  if (splitPositions.empty()) {
    return 0;
  }
  int previousChunkId = -1;
  int chunkId = -1;
  for (int i = 0; i < coverage.size(); ++i) {
    if (coverage.test(i)) {
      // we need to use iBackwards because bitset, when constructing from
      // a string, stores backwards.
      int iBackwards = coverage.size() - 1 - i;
      bool foundChunkId = false;
      for (int j = 0; j < splitPositions.size(); ++j) {
        if (iBackwards < splitPositions[j]) {
          chunkId = j;
          foundChunkId = true;
          break;
        }
      }
      if (!foundChunkId) {
        chunkId = splitPositions.size();
      }
      // coverage belongs to two different chunks.
      if (previousChunkId != -1 && chunkId != previousChunkId ) {
        return -1;
      }
      previousChunkId = chunkId;
    }
  }
  CHECK_NE(-1, chunkId) << "The chunk id should not be -1 at this point: if the"
      " coverage " << coverage << " covered different chunks, then -1 should "
      "have been returned before.";
  return chunkId;
}

} // namespace gen
} // namespace eng
} // namespace cam
