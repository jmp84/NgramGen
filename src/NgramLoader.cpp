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

#include "Util.h"

namespace cam {
namespace eng {
namespace gen {

NgramLoader::NgramLoader(const std::vector<int>& inputSentence) :
    inputSentence_(inputSentence) {}

void NgramLoader::loadNgram(const std::string& fileName,
                            const std::vector<int>& splitPositions,
                            const std::vector<bool>& chunksToReorder) {
  // ngrams_ has a size which is the number of chunks, splitPositions as well.
  // even with one chunk, splitPositions contains one element which is the
  // input sentence size.
  ngrams_.resize(splitPositions.size());
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
    Coverage coverage;
    positionList2Coverage(parts[1], &coverage);
    // chunkId is the chunk id where the coverage should belong. For example, if
    // there is no split, then all coverages belong to chunkId zero. For an
    // input "a b c d" and split positions <2>, then a coverage 1100 belongs to
    // chunkId zero, a coverage 0011 belongs to chunkId one, and a coverage 0110
    // belongs nowhere (by convention, chunkId minus one).
    int chunkId = getChunkId(coverage, splitPositions);
    // if chunkId is negative (meaning the n-gram doesn't belong to any
    // specific chunk) or if the chunk is not supposed to be reordered, then we
    // don't load any n-gram for that chunk
    if (chunkId < 0 ||
        (chunkId < chunksToReorder.size() && !chunksToReorder[chunkId])) {
      continue;
    }
    boost::split(ngramString, parts[2], boost::is_any_of("_"));
    ngramInt.resize(ngramString.size());
    std::transform(ngramString.begin(), ngramString.end(), ngramInt.begin(),
                   boost::lexical_cast<int, std::string>);
    ngrams_[chunkId][ngramInt].push_back(coverage);
  }
  // for chunks that are not supposed to be reordered, load the input chunk as
  // unique n-gram for that chunk.
  for (int chunkId = 0; chunkId < chunksToReorder.size(); ++chunkId) {
    if (!chunksToReorder[chunkId]) {
      std::stringstream bits;
      for (int i = 0; i < (chunkId > 0 ? splitPositions[chunkId - 1] : 0);
          ++i) {
        bits << "0";
      }
      for (int i = (chunkId > 0 ? splitPositions[chunkId - 1] : 0);
          i < splitPositions[chunkId]; ++i) {
        bits << "1";
      }
      for (int i = splitPositions[chunkId]; i < inputSentence_.size(); ++i) {
        bits << "0";
      }
      Coverage chunkCoverage(bits.str());
      std::vector<int> chunk;
      for (int i = (chunkId > 0 ? splitPositions[chunkId - 1] : 0);
          i < splitPositions[chunkId]; ++i) {
        chunk.push_back(inputSentence_[i]);
      }
      ngrams_[chunkId].clear();
      ngrams_[chunkId][chunk].push_back(chunkCoverage);
    }
  }
}

const std::map<Ngram, std::vector<Coverage> >& NgramLoader::ngrams(
    const int chunkId) const {
  CHECK_LT(chunkId, ngrams_.size()) << "Invalid chunk id " << chunkId << ". "
      "Must be less than the size of the number of chunks: " << ngrams_.size();
  return ngrams_[chunkId];
}

void NgramLoader::positionList2Coverage(const std::string& positionList,
                                        Coverage* coverage) {
  coverage->resize(inputSentence_.size());
  std::vector<std::string> stringPositions;
  boost::split(stringPositions, positionList, boost::is_any_of("_"));
  std::vector<int> positions(stringPositions.size());
  std::transform(stringPositions.begin(), stringPositions.end(),
                 positions.begin(), boost::lexical_cast<int, std::string>);
  // here we emulate the behaviour of the boost::dynamic_bitset constructor
  // from a string that fills backwards.
  for (int i = 0; i < positions.size(); ++i) {
    coverage->set(inputSentence_.size() - 1 - positions[i]);
  }
}

int NgramLoader::getChunkId(const Coverage& coverage,
                            const std::vector<int>& splitPositions) {
  // if there is no split, then all coverages belong to chunkId zero.
  if (splitPositions.size() == 1) {
    CHECK_EQ(splitPositions[0], inputSentence_.size()) << "Split positions "
        "contains only one element which should be equal to the input sentence "
        "size";
    return 0;
  }
  int previousChunkId = -1;
  int chunkId = -1;
  for (int i = 0; i < coverage.size(); ++i) {
    if (coverage.test(i)) {
      chunkId = -1;
      // we need to use iBackwards because bitset, when constructing from
      // a string, stores backwards.
      int iBackwards = coverage.size() - 1 - i;
      for (int j = 0; j < splitPositions.size(); ++j) {
        if (iBackwards < splitPositions[j]) {
          chunkId = j;
          break;
        }
      }
      CHECK_NE(chunkId, -1) <<
          "Did not found any chunk id for coverage " << coverage << " and split"
          " positions " << toString<int>(splitPositions);
      // coverage belongs to two different chunks.
      if (previousChunkId != -1 && chunkId != previousChunkId ) {
        return -1;
      }
      previousChunkId = chunkId;
    }
  }
  return chunkId;
}

} // namespace gen
} // namespace eng
} // namespace cam
