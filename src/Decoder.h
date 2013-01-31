/*
 * Decoder.h
 *
 *  Created on: 28 Jan 2013
 *      Author: jmp84
 */

#ifndef DECODER_H_
#define DECODER_H_

#include "Lattice.h"

namespace cam {
namespace eng {
namespace gen {

class Chopper;

/**
 * Manages decoding.
 */
class Decoder {
public:
  /**
   * Constructor. Arguments are command line flags.
   * @param sentenceFile The input sentence file.
   * @param ngrams The ngram directory.
   * @param lm The lm directory.
   * @param fstOutput The fst output directory.
   * @param range The range of sentences to decode.
   * @param overlap The allowed overlap between n-grams.
   * @param pruneNbest The maximum number of states in a column.
   * @param pruneThreshold The threshold for beam pruning.
   * @param dumpPrune The pruning parameter to use prior to dumping the fst.
   * @param addInput Determines if the input should be added to the output
   * lattice.
   * @param whenLostInput Whether we should detect when the input was lost.
   * @param features Comma-separated feature names.
   * @param weights Feature weights in format name1=weight1,name2=weight2,etc.
   * @param task Decoding or tuning.
   * @param chop Chopping strategy.
   * @param maxChop Maximum number of words per chunk when chopping.
   * @param punctuation Punctuation file for chopping with punctuation.
   * @param wordmap Wordmap file.
   */
  Decoder(
      const std::string& sentenceFile, const std::string& ngrams,
      const std::string& lm, const std::string& fstOutput,
      const std::string& range, const int overlap, const int pruneNbest,
      const float pruneThreshold, const float dumpPrune, const bool addInput,
      const bool whenLostInput, const std::string& features,
      const std::string& weights, const std::string& task,
      const std::string& chop, const int maxChop,
      const std::string& punctuation, const std::string& wordmap);

  /**
   * Decodes everything.
   */
  void decode() const;

private:
  /**
   * Reads a file line by line, each line is tokenized by whitespace.
   * @param fileName The input file name.
   */
  void parseInput(const std::string& fileName);

  /**
   * Parses comma separated feature names into a vector of feature names.
   * @param featureNames The comma separated feature names.
   */
  void parseFeatures(const std::string& featureNames);

  /**
   * Parses a comma separated list of pair featureName=featureWeight into a
   * Weight object. The format is featureName1=weight1,featureName2=weight2 etc."
   * @param featureWeights The comma separated list of featureName=featureWeight
   * pairs
   */
  void parseWeights(const std::string& featureWeights);

  /**
   * Decodes a specific sentence. Possibly chops the input and decodes the
   * chunks separately.
   * @param inputSentence The input sentence to decode.
   * @param id The id of the sentence (coming from a range).
   */
  void decode(const std::vector<int>& inputSentence, const int id) const;

  /**
   * Decodes a sequence of words. Could be a sentence or a chunk.
   * @param inputWords The input words to decode.
   * @param ngramLoader Object containing the list of n-grams relevant to the
   * input words.
   * @param lattice The output lattice.
   */
  template <class Arc>
  void decode(
      const std::vector<int>& inputSentence,
      const std::vector<int>& splitPositions, const NgramLoader& ngramLoader,
      const int id, Lattice<Arc>* lattice) const;

  /** Input sentences to decode. */
  std::vector<std::vector<int> > inputSentences_;
  /** Feature names. */
  std::vector<std::string> features_;
  /** Feature weights. */
  Weights weights_;
  /** N-gram directory. */
  std::string ngrams_;
  /** Language model directory. */
  std::string lm_;
  /** Fst output directory. */
  std::string fstOutput_;
  /** Range of sentences to decode. */
  std::string range_;
  /** Maximum overlap between n-grams. */
  int overlap_;
  /** Maximum number of states in a column. */
  int pruneNbest_;
  /** Threshold for beam pruning. */
  float pruneThreshold_;
  /** Pruning parameter for pruning output fst before dumping to disk. */
  float dumpPrune_;
  /** Determines if the input should be added to the output. */
  bool addInput_;
  /** Whether we should detect when the input was lost. */
  bool whenLostInput_;
  /** Decoding or tuning. */
  std::string task_;
  /** Chopper interface. */
  boost::shared_ptr<Chopper> chopper_;
};

template <class Arc>
void Decoder::decode(
    const std::vector<int>& inputSentence,
    const std::vector<int>& splitPositions, const NgramLoader& ngramLoader,
    const int id, Lattice<Arc>* lattice) const {
  int chunkId = 0;
  int splitPosition = splitPositions.empty() ? inputSentence.size() :
      splitPositions[chunkId];
  for (int i = 0; i < inputSentence.size(); ++i) {
    if (i >= splitPosition) {
      ++chunkId;
      splitPosition = chunkId < splitPositions.size() ?
          splitPositions[chunkId] : inputSentence.size();
    }
    lattice->extend(
        ngramLoader, i, pruneNbest_, pruneThreshold_, overlap_, chunkId);
  }
  lattice->markFinalStates(inputSentence.size());
  if (addInput_) {
    lattice->addInput();
  }
  if (whenLostInput_) {
    lattice->whenLostInput();
  }
  lattice->compactFst(dumpPrune_);
  std::ostringstream output;
  output << fstOutput_ << "/" << id << ".fst";
  lattice->write(output.str());
}

} // namespace gen
} // namespace eng
} // namespace cam

#endif /* DECODER_H_ */
