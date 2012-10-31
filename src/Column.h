/*
 * Column.h
 *
 *  Created on: 19 Oct 2012
 *      Author: jmp84
 */

#ifndef COLUMN_H_
#define COLUMN_H_

#include <map>
#include <set>

namespace cam {
namespace eng {
namespace gen {

class State;
class StateKey;

struct StatePointerComparator {
  bool operator()(const State* s1, const State* s2) const;
};

/**
 * A column in the lattice. A column has multiple states. We want to be able to
 * access these states in O(1) by a state key (history + coverage), so states
 * are indexed by StateKey in a map. We also need to sort these states by cost
 * for pruning so states are also sorted by their cost. The states are stored
 * in a set. The map simply contains pointers to these states.
 */
class Column {
public:
  /**
   * Checks if the column is empty, i.e. if it has no state.
   * @return
   */
  bool empty();

private:
  /** States indexed by StateKey (coverage + history). */
  std::map<StateKey, State*> statesIndexByStateKey_;
  /** States sorted by their cost.*/
  std::multiset<State*, StatePointerComparator> statesSortedByCost_;

  friend class Lattice;
};

} // namespace gen
} // namespace eng
} // namespace cam

#endif /* COLUMN_H_ */
