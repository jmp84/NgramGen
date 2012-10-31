/*
 * Column.cpp
 *
 *  Created on: 25 Oct 2012
 *      Author: jmp84
 */

#include "Column.h"
#include "State.h"

namespace cam {
namespace eng {
namespace gen {

bool StatePointerComparator::operator()(const State* s1,
                                        const State* s2) const {
  return s1->cost() < s2->cost();
}

bool Column::empty() {
  if (statesIndexByStateKey_.empty() && !statesSortedByCost_.empty()) {
    // TODO throw exception or do a google check
  }
  if (!statesIndexByStateKey_.empty() && statesSortedByCost_.empty()) {
    // TODO throw exception or do a google check
  }
  return (statesIndexByStateKey_.empty() && statesSortedByCost_.empty());
}

} // namespace gen
} // namespace eng
} // namespace cam
