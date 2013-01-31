/*
 * Vocab.cpp
 *
 *  Created on: 14 Dec 2012
 *      Author: jmp84
 */

#include "Vocab.h"
#include <fstream>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <glog/logging.h>

namespace cam {
namespace eng {
namespace gen {

void Vocab::loadVocab(const std::string& fileName) {
  std::ifstream file(fileName.c_str());
  CHECK(file.good()) << "Cannot open file " << fileName;
  std::string line;
  while (getline(file, line)) {
    std::vector<std::string> parts;
    boost::split(parts, line, boost::is_any_of("\t"));
    CHECK_EQ(2, parts.size()) << "Wrong format in wordmap, expecting "
        "\"id<tab>word\", got this instead: " << line;
    int id = boost::lexical_cast<int>(parts[0]);
    std::string word = parts[1];
    if (id + 1 > vocab_.size()) {
      vocab_.resize(id + 1);
    }
    if (!vocab_[id].empty()) {
      if (vocab_[id] == word) {
        LOG(WARNING) << "Duplicate entry in the wordmap: " << line;
      } else {
        LOG(FATAL) << "Inconsistent wordmap: word id " << id << " associated "
            "with " << vocab_[id] << " and with " << word;
      }
    }
    vocab_[id] = word;
  }
  file.close();
}

const std::string& Vocab::getWord(const int wordId) const {
  CHECK_LT(wordId, vocab_.size()) << "Out of range word id: " << wordId;
  CHECK_NE("", vocab_[wordId]) << "Empty word, invalid word id: " << wordId;
  return vocab_[wordId];
}

} // namespace gen
} // namespace eng
} // namespace cam
