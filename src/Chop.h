/*
 * Chop.h
 *
 *  Created on: 14 Dec 2012
 *      Author: jmp84
 */

#ifndef CHOP_H_
#define CHOP_H_

#include <vector>
#include <set>
#include <string>
#include <boost/smart_ptr.hpp>

/**
 * Classes for chopping an input into multiple chunks. This is used to make
 * decoding time and memory reasonable.
 */
namespace cam {
namespace eng {
namespace gen {

class Vocab;

/**
 * Base class for chopping. Doesn't chop the input.
 */
class Chopper {
public:
  /**
   * Destructor.
   */
  virtual ~Chopper();

  /**
   * Chops an input sentence into only one chunk.
   * @param inputSentence The input sentence to be chopped.
   * @param id The sentence id.
   * @return A vector containing one element which is the size of the input
   * sentence.
   */
  virtual std::vector<int> chop(
      const std::vector<int>& inputSentence, const int id);
};

/**
 * Chops every N words no matter what.
 */
class SillyChopper : public Chopper {
public:
  /**
   * Constructor.
   * @param maxNumWords The maximum number of words in a chunk.
   */
  explicit SillyChopper(const int maxNumWords);

  virtual ~SillyChopper();

  /**
   * Chops the input after seeing a maximum number of words.
   * @param inputSentence The input sentence to chop.
   * @return The positions where to chop the input. The positions are
   * zero-based. A position p indicates that the word in zero-based position
   * p starts a new chunk. For example, if the input is "a b c" and the
   * maximum number of words per chunk is 2, then the result will be a vector
   * <2, 3>. If the input is "a b c d e", the the result is <2, 4, 5>.
   */
  virtual std::vector<int> chop(
      const std::vector<int>& inputSentence, const int id);

protected:
  /** Maximum number of words in a chunk. */
  int maxNumWords_;
};

/**
 * Chops with punctuation and a maximum number of words per chunk.
 */
class PunctuationChopper : public SillyChopper {
public:
  /**
   * Constructor.
   * @param maxNumWords Maximum number of words in a chunk.
   * @param punctuationFile A file containing punctuation symbols (one per
   * line), which will guide chopping.
   * @param wordmap A word map file. Words ids can be converted to words and
   * this way we can check if they are punctuation symbols.
   */
  PunctuationChopper(const int maxNumWords, const std::string& punctuationFile,
                     const std::string& wordmap);

  /**
   * Destructor.
   */
  virtual ~PunctuationChopper();

  /**
   * Chops the input into chunks when seeing a punctuation symbol or after
   * seeing a maximum number of words.
   * @param inputSentence The input sentence to be chopped.
   * @return The positions where to chop the input. The positions are zero-based
   * and indicate where to start a new chunk.
   */
  virtual std::vector<int> chop(
      const std::vector<int>& inputSentence, const int id);

private:
  /**
   * Loads the punctuation symbol file.
   * @param fileName The file name.
   */
  void loadPunctuation(const std::string& fileName);

  /**
   * Checks if a word id (in the input) is a punctuation symbol.
   * @param wordId The word id.
   * @return True if the word id is the id of a punctuation symbol.
   */
  bool isPunctuation(const int wordId) const;

  /** List of punctuation symbols. */
  std::set<std::string> punctuationSymbols_;

  /** Word map. */
  boost::shared_ptr<Vocab> wordmap_;
};

/**
 * Chopper that reads chopping information from a file.
 */
class ChopperFromFile : public Chopper {
public:
  /**
   * Constructor.
   * @param chopFile File containing chopping information. The format is one
   * chopping information per sentence. Each line contains space separated
   * integers, each integer indicates the word index where to start the next
   * chunk, for example "3 5" means there are two chunks, the first chunk from
   * word 0 to word 2, and the second chunk from word 3 to word 4. 5 is the
   * input sentence size.
   */
  ChopperFromFile(const std::string& chopFile);

  /**
   * Destructor.
   */
  virtual ~ChopperFromFile();

  /**
   * Chops the input into chunks based on the info given in a chop file.
   * @param inputSentence The input sentence to be chopped.
   * @return The positions where to chop the input. The positions are zero-based
   * and indicate where to start a new chunk.
   */
  virtual std::vector<int> chop(
      const std::vector<int>& inputSentence, const int id);

private:
  std::vector<std::vector<int> > splits_;
};


} // namespace gen
} // namespace eng
} // namespace cam

#endif /* CHOP_H_ */
