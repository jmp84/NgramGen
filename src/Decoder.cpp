/*
 * Decoder.cpp
 *
 *  Created on: 30 Jan 2013
 *      Author: jmp84
 */

#include "Decoder.h"
#include "Chop.h"
#include "Range.h"

namespace cam {
namespace eng {
namespace gen {

Decoder::Decoder(const std::string& sentenceFile, const std::string& ngrams,
                 const std::string& lm, const std::string& fstOutput,
                 const std::string& range, const int overlap,
                 const int pruneNbest, const float pruneThreshold,
                 const float dumpPrune, const bool addInput,
                 const bool whenLostInput, const std::string& features,
                 const std::string& weights, const std::string& task,
                 const std::string& chop, const int maxChop,
                 const std::string& punctuation, const std::string& wordmap,
                 const std::string& chopFile, const std::string& constraints,
                 const std::string& constraintsFile) :
                   ngrams_(ngrams), lm_(lm), fstOutput_(fstOutput),
                   range_(range), overlap_(overlap), pruneNbest_(pruneNbest),
                   pruneThreshold_(pruneThreshold), dumpPrune_(dumpPrune),
                   addInput_(addInput), whenLostInput_(whenLostInput),
                   task_(task) {
  parseInput(sentenceFile);
  parseFeatures(features);
  parseWeights(weights);
  if (chop == "silly") {
    chopper_.reset(new SillyChopper(maxChop));
  } else if (chop == "punctuation") {
    chopper_.reset(new PunctuationChopper(maxChop, punctuation, wordmap));
  } else if (chop == "from_file") {
    chopper_.reset(new ChopperFromFile(chopFile));
  } else {
    chopper_.reset(new Chopper());
  }
  if (constraints == "chunk") {
    constraints_.reset(new ChunkConstraints(constraintsFile));
  } else {
    constraints_.reset(new Constraints());
  }
}

void Decoder::decode() const {
  for (boost::scoped_ptr<IntegerRangeInterface> ir(
      IntegerRangeInterface::initFactory(range_)); !ir->done(); ir->next()) {
    int id = ir->get();
    LOG(INFO)<< "Processing sentence number " << id;
    decode(inputSentences_[id - 1], id);
  }
}

void Decoder::decode(
    const std::vector<int>& inputSentence, const int id) const {
  std::vector<int> splitPositions = chopper_->chop(inputSentence, id);
  std::vector<bool> chunksToReorder = constraints_->constrain(id);
  NgramLoader ngramLoader(inputSentence);
  std::ostringstream ngramFile;
  ngramFile << ngrams_ << "/" << id << ".r.gz";
  ngramLoader.loadNgram(ngramFile.str(), splitPositions, chunksToReorder);
  std::ostringstream lmFile;
  lmFile << lm_ << "/" << id << "/lm.4.gz";
  boost::shared_ptr<lm::ngram::Model> languageModel(
      new lm::ngram::Model(lmFile.str().c_str()));
  if (task_ == "decode") {
    decode<fst::StdArc>(inputSentence, splitPositions, ngramLoader, id,
                        new Lattice<fst::StdArc>(
                            inputSentence, languageModel, features_, weights_));
  } else if (task_ == "tune") {
    decode<TupleArc32>(inputSentence, splitPositions, ngramLoader, id,
                       new Lattice<TupleArc32>(
                           inputSentence, languageModel, features_, weights_));
  }
}

void Decoder::parseInput(const std::string& fileName) {
  inputSentences_.clear();
  std::ifstream file(fileName.c_str());
  CHECK(file.is_open()) << "Cannot open file " << fileName;
  std::string line;
  while (std::getline(file, line)) {
    std::vector<std::string> stringWords;
    boost::split(stringWords, line, boost::is_any_of(" "));
    std::vector<int> sequence(stringWords.size());
    std::transform(stringWords.begin(), stringWords.end(), sequence.begin(),
                   boost::lexical_cast<int, std::string>);
    inputSentences_.push_back(sequence);
  }
}

void Decoder::parseFeatures(const std::string& featureNames) {
  // boost split for empty strings returns a vector of size 1 so we need to take
  // care of that case
  if (featureNames.empty()) {
    features_.clear();
  } else {
    boost::split(features_, featureNames, boost::is_any_of(","));
  }
}

void Decoder::parseWeights(const std::string& featureWeights) {
  if (featureWeights.empty()) {
    weights_.clear();
    return;
  }
  std::vector<std::string> listPairFeatureNameFeatureWeight;
  boost::split(listPairFeatureNameFeatureWeight, featureWeights,
               boost::is_any_of(","));
  // +1 is for the language model.
  std::vector<float> weightsForParams(
      listPairFeatureNameFeatureWeight.size() + 1);
  weightsForParams[0] = 1;
  for (int i = 0; i < listPairFeatureNameFeatureWeight.size(); ++i) {
    std::vector<std::string> pairFeatureNameFeatureWeight;
    boost::split(pairFeatureNameFeatureWeight,
                 listPairFeatureNameFeatureWeight[i], boost::is_any_of("="));
    CHECK_EQ(2, pairFeatureNameFeatureWeight.size())<<
    "Malformed feature weight pair: " <<
    listPairFeatureNameFeatureWeight[i] <<
    " in feature weight string: " << featureWeights;
    float featureWeight =
        boost::lexical_cast<float>(pairFeatureNameFeatureWeight[1]);
    weights_.addWeight(pairFeatureNameFeatureWeight[0], featureWeight);
    weightsForParams[i + 1] = featureWeight;
  }
  // this overrides setting the weights from an environment variable.
  fst::TropicalSparseTupleWeight<float>::Params() = weightsForParams;
}

} // namespace gen
} // namespace eng
} // namespace cam
