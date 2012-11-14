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

#include "Types.h"

// pruning parameters
DECLARE_int32(prune_nbest);
DECLARE_double(prune_threshold);

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
   * Constructor
   */
  Lattice();

  /**
   * Destructor. Custom destructor because one field is a pointer.
   */
  ~Lattice();

  /**
   * Initializes the lattice. Creates an empty array of size the size of the
   * input. Creates an initial state and adds it to the lattice.
   * @param words The input words to be reordered.
   * @param lmfile The language model file (in arpa or kenlm format).
   */
  void init(const std::vector<int>& words, const std::string& lmfile);

  /**
   * Extends a column by extending all states in the column with the n-grams
   * provided by an n-gram loader.
   * @param ngramLoader The ngram loader that contains a list of n-grams with
   * coverage.
   * @param columnIndex The index of the column to be extended.
   */
  void extend(const NgramLoader& ngramLoader, const int columnIndex);

  /**
   * Set final states for states that are in the column indexed by the length
   * of the input.
   * @param length The length of the input
   */
  void markFinalStates(const int length);

  /**
   * Applies fst operations to get a compact fst.
   */
  void compactFst();

  /**
   * Write the output fst to a file.
   * @param filename The file name where to write the output fst.
   */
  void write(const string& filename) const;

private:
  /**
   * Extends a state with an n-gram.
   * @param state The state to be extended.
   * @param ngram The n-gram used to extend the state.
   * @param coverage The coverage of the n-gram.
   */
  void extend(const State& state, const std::vector<int>& ngram,
              const Coverage& coverage);

  /**
   * Computes the cost of a state extended with an ngram.
   * @param state The state to be extended with an ngram.
   * @param ngram The ngram used to extend the state.
   * @param nextKenlmState The kenlm state we end up in.
   * @return The cost which the original cost + the cost of the n-gram.
   */
  Cost cost(const State& state, const std::vector<int>& ngram,
            lm::ngram::State* nextKenlmState) const;

  /** The fst encoding the hypotheses. */
  boost::scoped_ptr<fst::StdVectorFst> fst_;

  /** Lattice as a vector of columns. Indices indicate how many words have been
   * covered. */
  std::vector<Column> lattice_;

  /** Language model in KenLM format. */
  lm::ngram::Model* languageModel_;
};

} // namespace gen
} // namespace eng
} // namespace cam

#endif /* LATTICE_H_ */
