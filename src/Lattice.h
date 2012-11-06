/*
 * Lattice.h
 *
 *  Created on: 19 Oct 2012
 *      Author: jmp84
 */

#ifndef LATTICE_H_
#define LATTICE_H_

#include <vector>
#include <lm/model.hh>
#include <fst/fst-decl.h>

// cannot forward declare a typedef
#include "Types.h"

namespace lm {
namespace ngram {
class State;
} // namespace ngram
} // namespace lm

namespace cam {
namespace eng {
namespace gen {

class Column;
class Ngram;
class NgramLoader;
class State;

/**
 * A lattice. States are defined by a history and a coverage vector. Arcs are
 * labeled with n-gram ids.
 */
class Lattice {
public:
  void print();
  void printColumn(int index);

  /**
   * Initializes the lattice. Creates an empty array of size the size of the
   * input. Creates an initial state and add it to the lattice.
   * @param words The input words to be reordered.
   */
  void init(const std::vector<int>& words, const std::string& lmfile);

  /**
   * Extends a column by extending all states in the column with the ngrams
   * provided by an ngram loader.
   * @param ngramLoader The ngram loader that contains a list of ngrams.
   * @param columnIndex The index of the column to be extended.
   */
  void extend(const NgramLoader& ngramLoader, int columnIndex);

  /**
   * Prune the column with index columnIndex. Keeps the n-best states.
   * @param columnIndex The index of the column to be pruned.
   * @param nbest The n-best threshold.
   */
  void pruneNbest(int columnIndex, int nbest);

  /**
   * Prune the column with index columnIndex. Keeps the states whose cost is
   * between the minimum cost and the minimum cost plus a treshold.
   * @param columnIndex The index of the column to be pruned.
   * @param threshold The threshold.
   */
  void pruneThreshold(int columnIndex, Cost threshold);

  /**
   * Converts this lattice to an fst in openFST format.
   * @param length The length of the input from which this lattice was built.
   */
  void convert2openfst(int length, fst::StdVectorFst* res);

private:
  /**
   * Extends a state with an ngram.
   * @param state The state to be extended
   * @param ngram The n-gram used to extend the state
   * @param coverage The coverage of the n-gram
   */
  void extend(const State& state, const std::vector<int>& ngram,
              const Coverage& coverage);

  /**
   * Computes the score of a state extended with an ngram.
   * @param state The state to be extended with an ngram.
   * @param ngram The ngram used to extend the state.
   * @param nextKenlmState The kenlm state we end up in.
   * @return The score
   */
  Cost cost(const State& state, const std::vector<int>& ngram,
            lm::ngram::State* nextKenlmState) const;

  /** Lattice. Indices indicate how many words have been covered. */
  std::vector<Column> lattice_;

  /** Language model in KenLM format. */
  lm::ngram::Model* languageModel_;
};

} // namespace gen
} // namespace eng
} // namespace cam

#endif /* LATTICE_H_ */
