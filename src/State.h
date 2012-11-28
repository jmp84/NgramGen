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

class StateKey;

class State {
public:
  typedef fst::StdArc::StateId StateId;
  /**
   * Constructor.
   * @param stateId The state id in openfst.
   * @param key The key that uniquely defines the state.
   * @param cost The cost.
   */
  State(StateId stateId, StateKey* key, const Cost cost);

  /**
   * Destructor. Custom destructor to delete the state key pointer.
   */
  ~State();

  /**
   * Checks if a state is initial, that is if the coverage has no bit set.
   * @return True if the coverage has no bit set, false otherwise.
   */
  bool isInitial() const;

  /**
   * Getter.
   * @return The openfst state id.
   */
  const StateId stateId() const;

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
  /** The state id in openfst. */
  StateId stateId_;
  /** A state key (pair coverage/history) that uniquely defines this state. */
  StateKey* stateKey_;
  /** The best cost so far, or shortest distance from the initial state. */
  Cost cost_;
};

} // namespace gen
} // namespace eng
} // namespace cam

#endif /* STATE_H_ */
