/*
 * Weights.h
 *
 *  Created on: 10 Jan 2013
 *      Author: jmp84
 */

#ifndef WEIGHTS_H_
#define WEIGHTS_H_

#include <map>
#include <string>

namespace cam {
namespace eng {
namespace gen {

/**
 * Map between feature names and feature weights.
 */
class Weights {
public:
  /**
   * Gets the weight corresponding to a feature name.
   * @param name The feature name.
   * @return The weight for that name.
   */
  const float getWeight(const std::string& name) const;

  /**
   * Adds a featureName/featureWeight pair.
   * @param name The feature name.
   * @param weight The feature weight.
   */
  void addWeight(const std::string& name, const float weight);

  /**
   * Clears the map holding the featureNAme/featureWeight pairs.
   */
  void clear();

private:
  /** Mapping feature name -> weight. */
  std::map<std::string, float> weights_;
};

} // namespace gen
} // namespace eng
} // namespace cam

#endif /* WEIGHTS_H_ */
