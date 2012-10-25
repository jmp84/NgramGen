/*
 * Arc.h
 *
 *  Created on: 19 Oct 2012
 *      Author: jmp84
 */

#ifndef ARC_H_
#define ARC_H_

//#include "Ngram.h"

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
  Arc(const State* origin, const Ngram* ngram);

  /**
   * Getter.
   * @return The n-gram.
   */
  const Ngram* ngram() const;

private:
  /** Pointer to the origin state of this arc.*/
  State* origin_;
  /** Label of the arc. Pointer for low memory consumption. */
  Ngram* ngram_;
};

} // namespace gen
} // namespace eng
} // namespace cam

#endif /* ARC_H_ */
