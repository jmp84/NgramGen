/*
 * Arc.h
 *
 *  Created on: 19 Oct 2012
 *      Author: jmp84
 */

#ifndef ARC_H_
#define ARC_H_

#include <vector>

namespace cam {
namespace eng {
namespace gen {

class Ngram;
class State;

/**
 * An arc between two states in a lattice. An arc is an incoming arc, there is
 * therefore only an origin.
 */
class Arc {
public:
  /**
   * Constructor
   * @param origin The origin state
   * @param ngram The n-gram label
   */
  Arc(State* origin, std::vector<int>* ngram);

  /**
   * Getter
   * @return The state.
   */
  const State* state() const;

  /**
   * Getter.
   * @return The n-gram.
   */
  const std::vector<int>* ngram() const;

private:
  /** Pointer to the origin state of this arc.*/
  State* origin_;
  /** Label of the arc. Pointer for low memory consumption. */
  std::vector<int>* ngram_;
};

} // namespace gen
} // namespace eng
} // namespace cam

#endif /* ARC_H_ */
