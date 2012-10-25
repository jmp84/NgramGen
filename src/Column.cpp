/*
 * Column.cpp
 *
 *  Created on: 25 Oct 2012
 *      Author: jmp84
 */

namespace cam {
namespace eng {
namespace gen {

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
