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
#include <boost/smart_ptr.hpp>
#include <gflags/gflags.h>
#include <glog/logging.h>
#include <fst/fstlib.h>

#include "Column.h"
#include "Lattice.h"
#include "NgramLoader.h"
#include "Range.h"
#include "State.h"
#include "StateKey.h"

DEFINE_string(sentenceFile, "",
              "Name of a file containing sentences to be reordered");
DEFINE_string(ngrams, "", "Name of a directory containing ngram and coverage "
              "files applicable to the input words");
DEFINE_string(lm, "", "Language model file directory, in arpa or kenlm format.");
DEFINE_int32(prune_nbest, 0, "N-best pruning: number of states kept in a"
             " column");
DEFINE_double(prune_threshold, 0, "Threshold pruning: add this threshold to "
              " the lowest cost in a column to define what states are kept.");
DEFINE_string(fstoutput, "", "Directory name for the fst outputs.");
DEFINE_string(range, "1", "Range of items to be processed");

namespace cam {
namespace eng {
namespace gen {

/**
 * Reads a file line by line, each line is tokenized by whitespace and the
 * sequence of tokens is added as an element to the result.
 * @param fileName The input file name.
 * @param words A vector of vector of integers representing an array of word
 * sequences.
 */
void parseInput(const std::string& fileName,
                std::vector<std::vector<int> >* words) {
  std::ifstream file(fileName.c_str());
  CHECK(file.is_open()) << "Cannot open file " << fileName;
  std::string line;
  while (std::getline(file, line)) {
    std::vector<std::string> stringWords;
    boost::split(stringWords, line, boost::is_any_of(" "));
    std::vector<int> sequence(stringWords.size());
    std::transform(stringWords.begin(), stringWords.end(), sequence.begin(),
                 boost::lexical_cast<int, std::string>);
    words->push_back(sequence);
  }
}

void checkArgs(int argc, char** argv) {
  std::string usage = "Generates a lattice of reordered sentences.\n\n Usage: ";
  usage += argv[0];
  usage += "argv[0] --sentenceFile=sentenceFile --ngrams=ngramDirectory "
      "--lm=lmDirectory --fstoutput=fstoutputDirectory "
      "[--prune_nbest=<integer> | --prune_threshold=<double>]\n";
  google::ParseCommandLineFlags(&argc, &argv, true);
  CHECK_NE("", FLAGS_sentenceFile) << "Missing input --sentenceFile" <<
      std::endl << usage;
  CHECK_NE("", FLAGS_ngrams) << "Missing ngrams --ngram" << std::endl << usage;
  CHECK_NE("", FLAGS_lm) << "Missing language model directory --lm" <<
      std::endl << usage;
  CHECK_NE("", FLAGS_fstoutput) << "Missing output directory --fstoutput" <<
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
  std::vector<std::vector<int> > inputWords;
  parseInput(FLAGS_sentenceFile, &inputWords);
  for (boost::scoped_ptr<IntegerRangeInterface> ir(
      IntegerRangeInterface::initFactory(FLAGS_range));
      !ir->done(); ir->next()) {
    int id = ir->get();
    NgramLoader ngramLoader;
    std::ostringstream ngrams;
    ngrams << FLAGS_ngrams << "/"<< id << ".r";
    ngramLoader.loadNgram(ngrams.str());
    Lattice lattice;
    std::ostringstream lm;
    lm << FLAGS_lm << "/" << id << "/lm.4.gz";
    lattice.init(inputWords[id - 1], lm.str());
    for (int i = 0; i < inputWords[id - 1].size(); ++i) {
      if (FLAGS_prune_nbest != 0) {
        lattice.pruneNbest(i, FLAGS_prune_nbest);
      } else if (FLAGS_prune_threshold != 0) {
        lattice.pruneThreshold(i, FLAGS_prune_threshold);
      }
      lattice.extend(ngramLoader, i);
    }
    fst::StdVectorFst result;
    lattice.convert2openfst(inputWords.size(), &result);
    std::ostringstream output;
    output << FLAGS_fstoutput << "/" << id << ".fst";
    result.Write(output.str());
  }
}
