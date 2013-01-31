/*
 * Chop.cpp
 *
 *  Created on: 14 Dec 2012
 *      Author: jmp84
 */

#include "Chop.h"
#include <fstream>
#include <glog/logging.h>
#include "Vocab.h"

namespace cam {
namespace eng {
namespace gen {

Chopper::~Chopper() {}

std::vector<int> Chopper::chop(const std::vector<int>& inputSentence) {
  return std::vector<int>();
}

SillyChopper::~SillyChopper() {}

SillyChopper::SillyChopper(const int maxNumWords) : maxNumWords_(maxNumWords) {}

std::vector<int> SillyChopper::chop(
    const std::vector<int>& inputSentence) {
  std::vector<int> res;
  for (int i = 0; i < inputSentence.size(); ++i) {
    if ((i % maxNumWords_) == 0 && i != 0) {
      res.push_back(i);
    }
  }
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
    const std::vector<int>& inputSentence) {
  std::vector<int> res;
  int numWords = 0;
  for (int i = 0; i < inputSentence.size(); ++i) {
    numWords++;
    if (numWords >= maxNumWords_ || isPunctuation(inputSentence[i])) {
      if (i < inputSentence.size() - 1) {
        res.push_back(i + 1);
      }
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

} // namespace gen
} // namespace eng
} // namespace cam
