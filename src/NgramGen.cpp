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

#include <gflags/gflags.h>
#include <glog/logging.h>
#include <fst/fstlib.h>

#include "Column.h"
#include "Lattice.h"
#include "NgramLoader.h"
#include "State.h"
#include "StateKey.h"

DEFINE_string(sentenceFile, "",
              "Name of a file containing a sentence to be reordered");
DEFINE_string(ngrams, "", "Name of a file containing ngrams and coverages "
              "applicable to the input words");
DEFINE_string(lm, "", "Language model file, in arpa or kenlm format.");
DEFINE_int32(prune_nbest, 0, "N-best pruning: number of states kept in a"
             " column");
DEFINE_double(prune_threshold, 0, "Threshold pruning: add this threshold to "
              " the lowest cost in a column to define what states are kept.");
DEFINE_string(fstoutput, "", "File name for the fst output.");

namespace cam {
namespace eng {
namespace gen {

/**
 * Tokenizes the input using whitespace.
 * @param words
 */
void parseInput(const std::string& fileName, std::vector<int>* words) {
  std::ifstream file(fileName.c_str());
  CHECK(file.is_open()) << "Cannot open file " << fileName << ": " << file;
  std::string line;
  int lineCount = 0;
  while (std::getline(file, line)) {
    lineCount++;
    CHECK_EQ(1, lineCount) << fileName << " must contain only one line which "
        "is the sentence to reorder";
    std::vector<std::string> stringWords;
    boost::split(stringWords, line, boost::is_any_of(" "));
    words->resize(stringWords.size());
    std::transform(stringWords.begin(), stringWords.end(), words->begin(),
                   boost::lexical_cast<int, std::string>);
  }
}

void checkArgs(int argc, char** argv) {
  std::string usage = "Generates a lattice of reordered sentences.\n\n Usage: ";
  usage += argv[0];
  usage += "argv[0] --sentenceFile=sentenceFile --ngrams=ngramFile "
      "--lm=lm --fstoutput=fstoutput "
      "[--prune_nbest=<integer> | --prune_threshold=<double>]\n";
  google::ParseCommandLineFlags(&argc, &argv, true);
  CHECK_NE("", FLAGS_sentenceFile) << "Missing input --sentenceFile" <<
      std::endl << usage;
  CHECK_NE("", FLAGS_ngrams) << "Missing ngrams --ngram" << std::endl << usage;
  CHECK_NE("", FLAGS_lm) << "Missing language model --lm" << std::endl << usage;
  CHECK_NE("", FLAGS_fstoutput) << "Missing output --fstoutput" <<
      std::endl << usage;
  CHECK((FLAGS_prune_nbest == 0 && FLAGS_prune_threshold == 0) ||
        (FLAGS_prune_nbest != 0 && FLAGS_prune_threshold == 0) ||
        (FLAGS_prune_nbest == 0 && FLAGS_prune_threshold != 0)) << "Only one "
            "threshold strategy is allowed: --prune_nbest or --prune_threshold";
}

} // namespace gen
} // namespace eng
} // namespace cam

/**
 * This is the main program.
 */
int main(int argc, char** argv) {
  using namespace cam::eng::gen;
  checkArgs(argc, argv);
  std::vector<int> inputWords;
  parseInput(FLAGS_sentenceFile, &inputWords);
  NgramLoader ngramLoader;
  ngramLoader.loadNgram(FLAGS_ngrams);
  Lattice lattice;
  lattice.init(inputWords, FLAGS_lm);
  for (int i = 0; i < inputWords.size(); i++) {
    if (FLAGS_prune_nbest != 0) {
      lattice.pruneNbest(i, FLAGS_prune_nbest);
    } else if (FLAGS_prune_threshold != 0) {
      lattice.pruneThreshold(i, FLAGS_prune_threshold);
    }
    lattice.extend(ngramLoader, i);
  }
  fst::StdVectorFst result;
  lattice.convert2openfst(inputWords.size(), &result);
  result.Write(FLAGS_fstoutput);
}
