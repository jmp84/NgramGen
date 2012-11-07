/*
 * Column.cpp
 *
 *  Created on: 25 Oct 2012
 *      Author: jmp84
 */

#include "Column.h"

#include <glog/logging.h>
#include "State.h"

namespace cam {
namespace eng {
namespace gen {

bool StatePointerComparator::operator()(const State* s1,
                                        const State* s2) const {
  return (s1->cost() < s2->cost());
}

bool Column::empty() const {
  CHECK((statesIndexByStateKey_.empty() && statesSortedByCost_.empty()) ||
        (!statesIndexByStateKey_.empty() && !statesSortedByCost_.empty())) <<
            "The column state map and multiset should either be both empty or"
            " both not empty.";
  return (statesIndexByStateKey_.empty() && statesSortedByCost_.empty());
}

} // namespace gen
} // namespace eng
} // namespace cam
