/*
 * State.h
 *
 *  Created on: 19 Oct 2012
 *      Author: jmp84
 */

#ifndef STATE_H_
#define STATE_H_

#include <fst/fstlib.h>

#include "Types.h"

namespace lm {
namespace ngram {
class State;
}
}

namespace cam {
namespace eng {
namespace gen {

class Arc;
class StateKey;

class State {
public:
  /**
   * Constructor.
   * @param stateId The state id in openfst.
   * @param key The key that uniquely defines the state.
   * @param cost The cost.
   */
  State(fst::StdArc::StateId stateId, StateKey* key, const Cost cost);

  /**
   * Destructor.
   */
  ~State();

  /**
   * Checks if an n-gram with a certain coverage can extend the current state.
   * Conditions are coverage compatibility and start/end-of-sentence markers.
   * @param ngram
   * @param coverage
   * @return
   */
  bool canApply(const std::vector<int>& ngram, const Coverage& coverage) const;

  /**
   * Checks if a state is initial, that is if the coverage has no bit set.
   * @return True if the coverage has no bit set, false otherwise.
   */
  bool isInitial() const;

  /**
   * Getter.
   * @return The openfst state id.
   */
  const fst::StdArc::StateId stateId() const;

  /**
   * Getter.
   * @return The cost.
   */
  const Cost cost() const;

  /**
   * Getter.
   * @return The coverage.
   */
  const Coverage& coverage() const;

  /**
   * Getter.
   * @return The state key.
   */
  const StateKey* stateKey() const;

  /**
   * Getter.
   * @return The history (kenLM state) of the state key.
   */
  const lm::ngram::State& getKenlmState() const;

private:
  /**
   * Computes the overlap between the coverage vector of this state and another
   * coverage vector.
   * @param ngram
   * @return The overlap
   */
  int overlap(const Coverage& coverage) const;

  /** The state id in openfst. */
  fst::StdArc::StateId stateId_;
  /** A state key (pair coverage/history) that uniquely defines this state. */
  StateKey* stateKey_;
  /** The best cost so far, or shortest distance from the initial state. */
  Cost cost_;
};

} // namespace gen
} // namespace eng
} // namespace cam

#endif /* STATE_H_ */
