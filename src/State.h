/*
 * State.h
 *
 *  Created on: 19 Oct 2012
 *      Author: jmp84
 */

#ifndef STATE_H_
#define STATE_H_

#include "Arc.h"
#include "StateKey.h"
#include "Types.h"

namespace cam {
namespace eng {
namespace gen {

class Ngram;

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
   * Redefine operator< to be able to have State as a key in a multiset. The
   * comparison is simply based on cost.
   * @param other The other state to compare to.
   * @return true if the cost of this state is less than the cost of the other
   * state.
   */
  bool operator<(const State& other) const;

  /**
   * Computes the overlap between the coverage vector of this state and another
   * coverage vector.
   * @param ngram
   * @return The overlap
   */
  int overlap(const Coverage& coverage) const;

  /**
   * Getter.
   * @return The cost.
   */
  Cost cost() const;

  /**
   * Setter.
   * @param cost The cost to be set.
   */
  void setCost(const Cost cost);

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
  lm::ngram::State getKenlmState() const;

  /**
   * Getter.
   * @return The list of incoming arcs.
   */
  const std::vector<Arc>& incomingArcs() const;

  /**
   * Add an arc to the list of incoming arcs.
   * @param arc The arc to be added to the list of incoming arcs.
   */
  void addArc(const Arc& arc);

private:
  StateKey* stateKey_;
  Cost cost_;
  std::vector<Arc> incomingArcs_;

  friend class Lattice;
};

} // namespace gen
} // namespace eng
} // namespace cam

#endif /* STATE_H_ */
