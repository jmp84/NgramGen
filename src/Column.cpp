/*
 * Column.cpp
 *
 *  Created on: 25 Oct 2012
 *      Author: jmp84
 */

#include "Column.h"

#include <glog/logging.h>

#include "State.h"
#include "StateKey.h"

namespace cam {
namespace eng {
namespace gen {

bool StatePointerComparator::operator()(const State* s1,
                                        const State* s2) const {
  // first compare by cost, then compare by state key (coverage + history)
  if (s1->cost() < s2->cost()) {
    return true;
  }
  if (s1->cost() > s2->cost()) {
    return false;
  }
  return (*(s1->stateKey()) < *(s2->stateKey()));
}

Column::~Column() {
  for (boost::unordered_map<StateKey, State*>::const_iterator it =
      statesIndexByStateKey_.begin(); it != statesIndexByStateKey_.end();
      ++it) {
    delete it->second;
  }
}

bool Column::empty() const {
  CHECK((statesIndexByStateKey_.empty() && statesSortedByCost_.empty()) ||
        (!statesIndexByStateKey_.empty() && !statesSortedByCost_.empty())) <<
            "The column state map and set should either be both empty or"
            " both not empty.";
  return (statesIndexByStateKey_.empty() && statesSortedByCost_.empty());
}

} // namespace gen
} // namespace eng
} // namespace cam
