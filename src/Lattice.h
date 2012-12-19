/*
 * Lattice.h
 *
 *  Created on: 19 Oct 2012
 *      Author: jmp84
 */

#ifndef LATTICE_H_
#define LATTICE_H_

#include <vector>
#include <boost/smart_ptr.hpp>
#include <fst/fstlib.h>
#include <lm/model.hh>

#include "Column.h"
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
class NgramLoader;
class State;

/**
 * A lattice of generation hypotheses. States are defined by a history and a
 * coverage vector. Arcs are labeled with n-grams. States are kept track of in
 * columns that represent how many words have been covered for pruning.
 */
class Lattice {
public:
  typedef fst::StdArc::StateId StateId;

  /**
   * Constructor. Creates an empty array of size the size of the input. Creates
   * an initial state and adds it to the lattice.
   * @param words The input words to be reordered.
   * @param lmfile The language model file (in arpa or kenlm format).
   */
  Lattice(const std::vector<int>& words, const std::string& lmfile);

  /**
   * Destructor. Custom destructor because one field is a pointer.
   */
  ~Lattice();

  /**
   * Extends a column by extending all states in the column with the n-grams
   * provided by an n-gram loader.
   * @param ngramLoader The ngram loader that contains a list of n-grams with
   * coverage.
   * @param columnIndex The index of the column to be extended.
   * @param pruneNbest The nbest pruning threshold
   * @param pruneThreshold The float pruning threshold
   * @param maxOverlap The maximum overlap between state coverage and n-gram
   * coverage
   */
  void extend(const NgramLoader& ngramLoader, const int columnIndex,
              const int pruneNbest, const float pruneThreshold,
              const int maxOverlap);

  /**
   * Set final states for states that are in the column indexed by the length
   * of the input.
   * @param length The length of the input
   */
  void markFinalStates(const int length);

  /**
   * Add input to the output fst to ensure that we are able to regenerate the
   * input.
   */
  void addInput();

  /**
   * Applies fst operations to get a compact fst, including pruning.
   * @param pruneWeight The pruning threshold.
   */
  void compactFst(const int pruneWeight);

  /**
   * Write the output fst to a file.
   * @param filename The file name where to write the output fst.
   */
  void write(const string& filename) const;

private:
  /**
   * In case of overlap, checks if the history of a state is compatible with an
   * n-gram.
   * @param state The state to be potentially extended with n-gram.
   * @param ngram The n-gram used to potentially extend the state.
   * @param overlap The overlap between the n-gram coverage and the
   * state coverage
   * @param overlapCount The number of bits set in overlap.
   * @return True if the history of the state is compatible with the n-gram.
   */
  bool compatibleHistory(const State& state, const Ngram& ngram,
                         const Coverage& overlap, const int overlapCount) const;

  /**
   * Checks if an n-gram with a certain coverage can extend a state.
   * Conditions are coverage compatibility and start/end-of-sentence markers.
   * @param state The state to be extended.
   * @param ngram The n-gram extending the current state.
   * @param coverage The coverage of the n-gram.
   * @param maxOverlap The maximum overlap between state coverage and n-gram
   * coverage.
   * @param ngramToApply The n-gram to be applied to the state. This n-gram is
   * truncated if there is a non trivial overlap.
   * @return True if the state can be extended with the n-gram and coverage.
   */
  bool canApply(const State& state, const Ngram& ngram,
                const Coverage& coverage, const int maxOverlap,
                Ngram* ngramToApply) const;

  /**
   * Extends a state with an n-gram.
   * @param state The state to be extended.
   * @param ngram The n-gram used to extend the state.
   * @param coverage The coverage of the n-gram.
   * @param pruneNbest The maximum number of states in a column.
   * @param pruneThreshold The beam threshold pruning parameter.
   */
  void extend(const State& state, const std::vector<int>& ngram,
              const Coverage& coverage, const int pruneNbest,
              const float pruneThreshold);

  /**
   * Computes the cost of a state extended with an ngram.
   * @param state The state to be extended with an ngram.
   * @param ngram The ngram used to extend the state.
   * @param nextKenlmState The kenlm state we end up in.
   * @return The cost which the original cost + the cost of the n-gram.
   */
  Cost cost(const State& state, const std::vector<int>& ngram,
            lm::ngram::State* nextKenlmState) const;

  /**
   * Adds states and arcs to the fst based on the start state, the end state,
   * the n-gram begin applied and the cost of the n-gram. If the n-gram is
   * greater than a unigram, then intermediate are added to the fst.
   * @param state The start state.
   * @param ngram The n-gram being applied.
   * @param newState The end state.
   * @param applyNgramCost The lm cost of applying the n-gram.
   */
  void addFstStatesAndArcs(
      const State& state, const Ngram& ngram, const State* newState,
      const float applyNgramCost);

  /**
   * Adds states and arcs to the fst based on the start state,
   * the n-gram begin applied and the cost of the n-gram. If the n-gram is
   * greater than a unigram, then intermediate are added to the fst. The state
   * id for the last state added is returned in order to create a new Lattice
   * state with that id.
   * @param state The start state.
   * @param ngram The n-gram being applied.
   * @param applyNgramCost The lm cost of applying the n-gram.
   * @return The state id of the last state created.
   */
  StateId addFstStatesAndArcsNewState(
      const State& state, const Ngram& ngram, const float applyNgramCost);

  /**
   * Utility to compute the index in kenlm vocab from an integer id. This is a
   * bit inefficient. It should be possible to reuse the id directly.
   * @param id
   * @return The index in kenlm vocab.
   */
  const lm::WordIndex index(int id) const;

  /** The fst encoding the hypotheses. */
  boost::scoped_ptr<fst::StdVectorFst> fst_;

  /** Lattice as a vector of columns. Indices indicate how many words have been
   * covered. */
  std::vector<Column> columns_;

  /** Language model in KenLM format. */
  lm::ngram::Model* languageModel_;

  /** Input words to be reordered. */
  std::vector<int> inputWords_;

  friend class LatticeTest;
};

} // namespace gen
} // namespace eng
} // namespace cam

#endif /* LATTICE_H_ */
