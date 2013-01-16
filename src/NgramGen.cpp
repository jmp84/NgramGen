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

#include "features/Weights.h"
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
DEFINE_string(fstoutput, "", "Directory name for the fst outputs.");
DEFINE_string(range, "1", "Range of items to be processed");
DEFINE_int32(overlap, 0, "Maximum overlap allowed when extending a state.");
DEFINE_int32(prune_nbest, 0, "N-best pruning: number of states kept in a"
             " column");
DEFINE_double(prune_threshold, 0, "Threshold pruning: add this threshold to "
              " the lowest cost in a column to define what states are kept.");
DEFINE_double(dump_prune, 0, "Pruning parameter for dumping lattices. If set to"
              " a value greater than 0, then apply fstprune --weight= before"
              " writing.");
DEFINE_bool(add_input, false, "If true, add the input sentence to the output "
            "lattice, this ensures that we at least regenerate the input.");
DEFINE_bool(when_lost_input, false, "If true, looks for when the input was lost"
            " as a hypothesis");
DEFINE_string(features, "", "Comma separated list of features");
DEFINE_string(weights, "", "Comma separated list of feature weights. "
              "The format is featureName1=weight1,featureName2=weight2 etc.");
DEFINE_string(task, "decode", "Task: either 'decode' or 'tune'. If the task is "
              "decode, the output is a StdVectorFst. If the task is tune, then "
              "the output is a fst with sparse weights.");

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

/**
 * Parses comma separated feature names into a vector of feature names.
 * @param featureNames The comma separated feature names.
 * @param features The output list of feature names.
 */
void parseFeatures(const std::string& featureNames,
                   std::vector<std::string>* features) {
  // boost split for empty strings returns a vector of size 1 so we need to take
  // care of that case
  if (featureNames.empty()) {
    features->clear();
  }
  else {
    boost::split(*features, featureNames, boost::is_any_of(","));
  }
}

/**
 * Parses a comma separated list of pair featureName=featureWeight into a
 * Weight object. The format is featureName1=weight1,featureName2=weight2 etc."
 * @param featureWeights The comma separated list of featureName=featureWeight
 * pairs
 * @param weights The output Weight object.
 */
void parseWeights(const std::string& featureWeights, Weights* weights) {
  if (featureWeights.empty()) {
    weights->clear();
    return;
  }
  std::vector<std::string> listPairFeatureNameFeatureWeight;
  boost::split(
      listPairFeatureNameFeatureWeight, featureWeights, boost::is_any_of(","));
  for (int i = 0; i < listPairFeatureNameFeatureWeight.size(); ++i) {
    std::vector<std::string> pairFeatureNameFeatureWeight;
    boost::split(pairFeatureNameFeatureWeight,
                 listPairFeatureNameFeatureWeight[i], boost::is_any_of("="));
    CHECK_EQ(2, pairFeatureNameFeatureWeight.size()) <<
        "Malformed feature weight pair: " <<
        listPairFeatureNameFeatureWeight[i] <<
        " in feature weight string: " << featureWeights;
    weights->addWeight(
        pairFeatureNameFeatureWeight[0],
        boost::lexical_cast<float>(pairFeatureNameFeatureWeight[1]));
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
  CHECK(FLAGS_task == "decode" || FLAGS_task == "tune") << "Unknown task: " <<
      FLAGS_task << ". The task can only be 'decode' or 'tune'";
}

template <class Arc>
void decode(const vector<int>& inputWords, const int id,
            const NgramLoader& ngramLoader, Lattice<Arc>* lattice) {
  for (int i = 0; i < inputWords.size(); ++i) {
    lattice->extend(ngramLoader, i, FLAGS_prune_nbest, FLAGS_prune_threshold,
                    FLAGS_overlap);
  }
  lattice->markFinalStates(inputWords.size());
  if (FLAGS_add_input) {
    lattice->addInput();
  }
  if (FLAGS_when_lost_input) {
    lattice->whenLostInput();
  }
  lattice->compactFst(FLAGS_dump_prune);
  std::ostringstream output;
  output << FLAGS_fstoutput << "/" << id << ".fst";
  lattice->write(output.str());
}

} // namespace gen
} // namespace eng
} // namespace cam

/**
 * This is the main program.
 */
int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  using namespace cam::eng::gen;
  checkArgs(argc, argv);
  std::vector<std::vector<int> > inputWords;
  parseInput(FLAGS_sentenceFile, &inputWords);
  std::vector<std::string> features;
  parseFeatures(FLAGS_features, &features);
  Weights weights;
  parseWeights(FLAGS_weights, &weights);
  for (boost::scoped_ptr<IntegerRangeInterface> ir(
      IntegerRangeInterface::initFactory(FLAGS_range));
      !ir->done(); ir->next()) {
    int id = ir->get();
    LOG(INFO) << "Processing sentence number " << id;
    NgramLoader ngramLoader;
    std::ostringstream ngrams;
    ngrams << FLAGS_ngrams << "/"<< id << ".r";
    ngramLoader.loadNgram(ngrams.str());
    std::ostringstream lm;
    lm << FLAGS_lm << "/" << id << "/lm.4.gz";
    if (FLAGS_task == "decode") {
      decode(inputWords[id - 1], id, ngramLoader,
             new Lattice<fst::StdArc>(
                 inputWords[id - 1], lm.str(), features, weights));
    } else if (FLAGS_task == "tune") {
      LOG(FATAL) << "Task " << FLAGS_task << " not implemented yet!";
    }
  }
}
