/*
 ============================================================================
 Name        : NgramGen.cpp
 Author      : Juan Pino
 Version     :
 Copyright   : DWTFYW
 Description : Hello World in C++,
 ============================================================================
 */

#include <fstream>
#include <boost/algorithm/string.hpp>
#include <gflags/gflags.h>
#include <glog/logging.h>

#include "Lattice.h"

DEFINE_string(words, "",
              "Name of a file containing a sentence to be reordered");
DEFINE_string(ngrams, "", "Name of a file containing ngrams and coverages "
              "applicable to the input words");
DEFINE_string(lm, "", "Language model file, in arpa or kenlm format.");
DEFINE_bool(prune, false, "Should we prune or not ?");

namespace cam {
namespace eng {
namespace gen {

/**
 * Tokenizes the input using whitespace and
 * @param words
 */
void parseInput(const std::string& fileName, std::vector<int>* words) {
  std::ifstream file(fileName);
  CHECK(file.is_open()) << "Cannot open file " << fileName;
  std::string line;
  int lineCount = 0;
  while (std::getline(file, line)) {
    lineCount++;
    CHECK(lineCount == 1) << fileName << " must contain only one line which "
        "is the sentence to reorder";
    std::vector<std::string> stringWords;
    boost::split(stringWords, line, boost::is_any_of(" "));
    words->resize(stringWords.size());
    for (int i = 0; i < stringWords.size(); i++) {
      (*words)[i] = boost::lexical_cast<int>(stringWords[i]);
    }
  }
}

}
}
}

/**
 * This is the main program.
 */
int main(void) {
  using namespace cam::eng::gen;
  std::vector<int> inputWords;
  parseInput(FLAGS_words, &inputWords);
  Lattice lattice;
  lattice.init(inputWords);
  for (int i = 1; i <= inputWords.size(); i++) {
    if (FLAGS_prune) {
      lattice.prune(i, 50);
    }
    lattice.extend();
  }
}
