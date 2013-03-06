/*
 * Constraints.cpp
 *
 *  Created on: 4 Mar 2013
 *      Author: jmp84
 */

#include "Constraints.h"
#include <fstream>
#include <glog/logging.h>
#include <boost/dynamic_bitset.hpp>

Constraints::~Constraints() {}

std::vector<bool> Constraints::constrain(const int id) const {
  return std::vector<bool>(1, true);
}

ChunkConstraints::~ChunkConstraints() {}

ChunkConstraints::ChunkConstraints(const std::string& constraintsFile) {
  std::ifstream file(constraintsFile.c_str());
  CHECK(file.good()) << "Cannot open file " << constraintsFile;
  std::string line;
  while (getline(file, line)) {
    std::vector<bool> constraints;
    boost::dynamic_bitset<> bits(line);
    for (int i = bits.size() - 1; i >= 0; --i) {
      constraints.push_back(bits.test(i));
    }
    constraints_.push_back(constraints);
  }
}

std::vector<bool> ChunkConstraints::constrain(const int id) const {
  CHECK_LT(id - 1, constraints_.size()) << "Sentence id " << id <<
      " out of range.";
  CHECK_GE(id - 1, 0) << "Sentence id " << id << " out of range.";
  return constraints_[id - 1];
}
