/*
 * Lattice.h
 *
 *  Created on: 19 Oct 2012
 *      Author: jmp84
 */

#ifndef LATTICE_H_
#define LATTICE_H_

#include <vector>

// cannot forward declare a typedef
#include "Types.h"

class lm::ngram::Model;

namespace cam {
namespace eng {
namespace gen {

class Column;
class Ngram;
class State;

/**
 * A lattice. States are defined by a history and a coverage vector. Arcs are
 * labeled with n-gram ids.
 */
class Lattice {
public:
  /**
   * Initializes the lattice. Creates an empty array of size the size of the
   * input. Creates an initial state and add it to the lattice.
   * @param multiset The input multiset
   */
  void init(const Multiset& multiset);

  /**
   * Extends a state with an ngram.
   * @param state The state to be extended
   * @param ngram The ngram used to extend the state
   */
  void extend(const State& state, const Ngram& ngram);

  /**
   * Prune the column with index columnIndex. Keeps the n-best states.
   * @param columnIndex The index of the column to be pruned.
   * @param nbest The n-best threshold.
   */
  void prune(int columnIndex, int nbest);

  /**
   * Converts this lattice to an fst.
   * @param length The length of the input from which this lattice was built.
   */
  void convert2fst(int length);

private:
  /**
   * Computes the score of a state extended with an ngram.
   * @param state The state to be extended with an ngram.
   * @param ngram The ngram used to extend the state.
   * @param nextKenlmState The kenlm state we end up in.
   * @return The score
   */
  Cost cost(const State& state, const Ngram& ngram,
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
