/*
 * Types.hpp
 *
 *  Created on: 19 Oct 2012
 *      Author: jmp84
 */

#ifndef TYPES_H_
#define TYPES_H_

/**
 * Some typedefs.
 */

#include <set>
#include "boost/dynamic_bitset.hpp"

namespace cam {
namespace eng {
namespace gen {

/** Multiset input */
typedef std::multiset<int> Multiset;

/** Coverage vector of the input */
typedef boost::dynamic_bitset<> Coverage;

/** Cost. Could use a double if needed. */
typedef float Cost;

} // namespace gen
} // namespace eng
} // namespace cam

#endif /* TYPES_H_ */
