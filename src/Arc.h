/*
 * Arc.h
 *
 *  Created on: 19 Oct 2012
 *      Author: jmp84
 */

#ifndef ARC_H_
#define ARC_H_

#include <vector>
#include "Types.h"

namespace cam {
namespace eng {
namespace gen {

class State;

/**
 * An arc between two states in a lattice. An arc is an incoming arc, there is
 * only an origin.
 */
class Arc {
public:
  /**
   * Constructor
   * @param origin The origin state.
   * @param ngram The n-gram label.
   * @param cost The cost.
   */
  Arc(State* origin, std::vector<int>* ngram, const Cost cost);

  /**
   * Getter.
   * @return The origin state.
   */
  const State* state() const;

  /**
   * Getter.
   * @return The n-gram label.
   */
  const std::vector<int>* ngram() const;

  /**
   * Getter.
   * @return The cost of the arc.
   */
  const Cost cost() const;

private:
  /** Pointer to the origin state of this arc.*/
  State* origin_;
  /** Label of the arc. Pointer for low memory consumption. */
  std::vector<int>* ngram_;
  /** Cost of the arc, e.g. the language model negative ln probability of the
   * n-gram given the history of the origin state.*/
  Cost cost_;
};

} // namespace gen
} // namespace eng
} // namespace cam

#endif /* ARC_H_ */
