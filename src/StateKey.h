/*
 * StateKey.h
 *
 *  Created on: 19 Oct 2012
 *      Author: jmp84
 */

#ifndef STATEKEY_H_
#define STATEKEY_H_

#include "lm/state.hh"

#include "Types.h"

namespace cam {
namespace eng {
namespace gen {

/**
 * Uniquely defines a State by a history and a coverage.
 */
class StateKey {
public:
  /**
   * Constructor.
   * @param coverage The coverage so far.
   * @param kenlmState The history.
   */
  StateKey(const Coverage& coverage, const lm::ngram::State& kenlmState);

  /**
   * Redefine operator< to be able to have StateKey as a key in a map.
   * @param other The other StateKey to be compared to.
   * @return true if this object is less than the other one.
   */
  bool operator<(const StateKey& other) const;

  /**
   * Getter.
   * @return The coverage.
   */
  const Coverage& coverage() const;

private:
  /** Coverage of the input so far. */
  Coverage coverage_;
  /** The KenLM state. Stores the history to apply the language model. */
  lm::ngram::State kenlmState_;

  friend class State;
};

} // namespace gen
} // namespace eng
} // namespace cam

#endif /* STATEKEY_H_ */
