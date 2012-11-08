/*
 * State.h
 *
 *  Created on: 19 Oct 2012
 *      Author: jmp84
 */

#ifndef STATE_H_
#define STATE_H_

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
   * @param key The key that uniquely defines the state.
   * @param cost The cost.
   * @param incomingArcs The incoming arcs.
   */
  State(StateKey* key, const Cost cost, const std::vector<Arc>& incomingArcs);

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
   * Add an arc to the list of incoming arcs.
   * @param arc The arc to be added to the list of incoming arcs.
   */
  void addArc(const Arc& arc);

  /**
   * Checks if a state is initial, that is if the coverage has no bit set.
   * @return True if the coverage has no bit set, false otherwise.
   */
  bool isInitial() const;

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

  /**
   * Getter.
   * @return The list of incoming arcs.
   */
  const std::vector<Arc>& incomingArcs() const;

private:
  /**
   * Computes the overlap between the coverage vector of this state and another
   * coverage vector.
   * @param ngram
   * @return The overlap
   */
  int overlap(const Coverage& coverage) const;

  /** A state key (pair coverage/history) that uniquely defines this state. */
  StateKey* stateKey_;
  /** The best cost so far, or shortest distance from the initial state. */
  Cost cost_;
  /** List of incoming arcs. */
  std::vector<Arc> incomingArcs_;
};

} // namespace gen
} // namespace eng
} // namespace cam

#endif /* STATE_H_ */
