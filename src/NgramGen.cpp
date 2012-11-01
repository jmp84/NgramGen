/*
 ============================================================================
 Name        : NgramGen.cpp
 Author      : Juan Pino
 Version     :
 Copyright   : DWTFYW
 Description : Ngram generation
 ============================================================================
 */

#include <fstream>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

//#include <gflags/gflags.h>
//#include <glog/logging.h>
#include <fst/fstlib.h>

#include "Column.h"
#include "Lattice.h"
#include "NgramLoader.h"
#include "Ngram.h"
#include "State.h"
#include "StateKey.h"

DEFINE_string(sentenceFile, "",
              "Name of a file containing a sentence to be reordered");
DEFINE_string(ngrams, "", "Name of a file containing ngrams and coverages "
              "applicable to the input words");
DEFINE_string(lm, "", "Language model file, in arpa or kenlm format.");
DEFINE_bool(prune, false, "Should we prune or not ?");
DEFINE_string(fstoutput, "", "File name for the fst output.");

namespace cam {
namespace eng {
namespace gen {

/**
 * Tokenizes the input using whitespace and
 * @param words
 */
void parseInput(const std::string& fileName, std::vector<int>* words) {
  std::ifstream file(fileName.c_str());
  //CHECK(file.is_open()) << "Cannot open file " << fileName;
  std::string line;
  int lineCount = 0;
  while (std::getline(file, line)) {
    lineCount++;
    //CHECK(lineCount == 1) << fileName << " must contain only one line which "
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
int main(int argc, char** argv) {
  using namespace cam::eng::gen;
  std::string usage = "Generates a lattice of reordered sentences.\n\n Usage: ";
  usage += argv[0];
  usage += "argv[0] --sentenceFile=sentenceFile --ngrams=ngramFile "
      "--lm=lm --fstoutput=fstoutput [--prune=[false]]\n";
  SET_FLAGS(usage.c_str(), &argc, &argv, true);
  if (argc > 1) {
    ShowUsage();
  }
  std::vector<int> inputWords;
  parseInput(FLAGS_sentenceFile, &inputWords);
  NgramLoader ngramLoader;
  ngramLoader.loadNgram(FLAGS_ngrams);
  Lattice lattice;
  lattice.init(inputWords, FLAGS_lm);
  for (int i = 0; i < inputWords.size(); i++) {
    if (FLAGS_prune) {
      lattice.prune(i, 50);
    }
    lattice.extend(ngramLoader, i);
  }
  fst::StdVectorFst result;
  lattice.convert2openfst(inputWords.size(), &result);
  result.Write(FLAGS_fstoutput);
}
