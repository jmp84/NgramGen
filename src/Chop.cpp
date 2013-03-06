/*
 * Chop.cpp
 *
 *  Created on: 14 Dec 2012
 *      Author: jmp84
 */

#include "Chop.h"
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <fstream>
#include <glog/logging.h>
#include "Vocab.h"

namespace cam {
namespace eng {
namespace gen {

Chopper::~Chopper() {}

std::vector<int> Chopper::chop(
    const std::vector<int>& inputSentence, const int id) {
  return std::vector<int>(1, inputSentence.size());
}

SillyChopper::~SillyChopper() {}

SillyChopper::SillyChopper(const int maxNumWords) : maxNumWords_(maxNumWords) {}

std::vector<int> SillyChopper::chop(
    const std::vector<int>& inputSentence, const int id) {
  std::vector<int> res;
  for (int i = 0; i < inputSentence.size(); ++i) {
    if ((i % maxNumWords_) == 0 && i != 0) {
      res.push_back(i);
    }
  }
  res.push_back(inputSentence.size());
  return res;
}

PunctuationChopper::~PunctuationChopper() {}

PunctuationChopper::PunctuationChopper(
    const int maxNumWords, const std::string& punctuationFile,
    const std::string& vocabFile) :
        SillyChopper(maxNumWords), wordmap_(new Vocab()) {
  loadPunctuation(punctuationFile);
  wordmap_->loadVocab(vocabFile);
}

std::vector<int> PunctuationChopper::chop(
    const std::vector<int>& inputSentence, const int id) {
  std::vector<int> res;
  int numWords = 0;
  for (int i = 0; i < inputSentence.size(); ++i) {
    numWords++;
    if (numWords >= maxNumWords_ || isPunctuation(inputSentence[i]) ||
        i == inputSentence.size() - 1) {
      res.push_back(i + 1);
      numWords = 0;
    }
  }
  return res;
}

void PunctuationChopper::loadPunctuation(const std::string& fileName) {
  std::ifstream file(fileName.c_str());
  CHECK(file.good()) << "Cannot open file " << fileName;
  std::string line;
  while (getline(file, line)) {
    punctuationSymbols_.insert(line);
  }
}

bool PunctuationChopper::isPunctuation(const int wordId) const {
  return (punctuationSymbols_.find(wordmap_->getWord(wordId)) !=
      punctuationSymbols_.end());
}

ChopperFromFile::ChopperFromFile(const std::string& chopFile) {
  std::ifstream file(chopFile.c_str());
  CHECK(file.good()) << "Cannot open file " << chopFile;
  std::string line;
  while (getline(file, line)) {
    std::vector<std::string> splitsString;
    boost::split(splitsString, line, boost::is_any_of(" "));
    std::vector<int> splits(splitsString.size());
    std::transform(splitsString.begin(), splitsString.end(), splits.begin(),
                   boost::lexical_cast<int, std::string>);
    splits_.push_back(splits);
  }
}

ChopperFromFile::~ChopperFromFile() {}

std::vector<int> ChopperFromFile::chop(
    const std::vector<int>& inputSentence, const int id) {
  CHECK_LT(id - 1, splits_.size()) << "Sentence id " << id << " out of range.";
  CHECK_GE(id - 1, 0) << "Sentence id " << id << " out of range.";
  return splits_[id - 1];
}

} // namespace gen
} // namespace eng
} // namespace cam
