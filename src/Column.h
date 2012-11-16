/*
 * Column.h
 *
 *  Created on: 19 Oct 2012
 *      Author: jmp84
 */

#ifndef COLUMN_H_
#define COLUMN_H_

#include <set>
#include <boost/unordered_map.hpp>

namespace cam {
namespace eng {
namespace gen {

class State;
class StateKey;

/**
 * State pointer comparator used to be able to use State* as a key in a set.
 */
struct StatePointerComparator {
  bool operator()(const State* s1, const State* s2) const;
};

/**
 * A column in the lattice. A column has multiple states. We want to be able to
 * access these states quickly by a state key (history + coverage), so states
 * are indexed by a StateKey in a map. We also need to sort these states by cost
 * for pruning so states are also sorted by their cost in a set.
 */
class Column {
public:
  /**
   * Destructor. Custom destructor to delete states.
   */
  ~Column();

  /**
   * Checks if the column is empty, i.e. if it has no state.
   * @return
   */
  bool empty() const;

private:
  /** States indexed by StateKey (coverage + history). */
  boost::unordered_map<StateKey, State*> statesIndexByStateKey_;
  /** States sorted by their cost.*/
  std::set<State*, StatePointerComparator> statesSortedByCost_;

  friend class Lattice;
};

} // namespace gen
} // namespace eng
} // namespace cam

#endif /* COLUMN_H_ */
