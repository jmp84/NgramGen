/*
 * Constraints.h
 *
 *  Created on: 4 Mar 2013
 *      Author: jmp84
 */

#ifndef CONSTRAINTS_H_
#define CONSTRAINTS_H_

#include <string>
#include <vector>

/**
 * Base class for constraints. Doesn't impose constraints.
 */
class Constraints {
public:
  /**
   * Destructor.
   */
  virtual ~Constraints();

  /**
   * Imposes no constraints.
   * @param id The sentence id.
   * @return A vector containing one element set to true. This means that the
   * sentence is considered as only one chunk and that this chunk can be
   * reordered.
   */
  virtual std::vector<bool> constrain(const int id) const;
};

class ChunkConstraints : public Constraints {
public:
  /**
   * Destructor.
   */
  virtual ~ChunkConstraints();

  /**
   * Constructor. Reads a constraints file and loads the constraints.
   * @param constraintsFile The constraints file.
   */
  ChunkConstraints(const std::string& constraintsFile);

  /**
   * Generates decoding constraints for a particular input.
   * @param id The id of the input sentence.
   * @return A list of boolean corresponding to chunks of the input sentence.
   * If an element in the list is true, then the chunk can be reordered,
   * otherwise the chunk needs to stay as is.
   */
  virtual std::vector<bool> constrain(const int id) const;

private:
  /** Decoding constraints. */
  std::vector<std::vector<bool> > constraints_;
};

#endif /* CONSTRAINTS_H_ */
