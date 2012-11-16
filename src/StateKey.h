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
   * Redefine operator== to be able to use StateKey in an unordered_map
   * @param other The other StateKey to compare to.
   * @return true if this object is equal to the other one.
   */
  bool operator==(const StateKey& other) const;

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
  friend std::size_t hash_value(const StateKey& stateKey);
};

/**
 * Hash function to be used in an unordered_map.
 * @param stateKey The key to hash.
 * @return The hash value.
 */
std::size_t hash_value(const StateKey& stateKey);

} // namespace gen
} // namespace eng
} // namespace cam

#endif /* STATEKEY_H_ */
