/*
 * NgramLoader.cpp
 *
 *  Created on: 26 Oct 2012
 *      Author: jmp84
 */

#include <fstream>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
//#include <glog/logging.h>

#include "NgramLoader.h"
#include "Ngram.h"

namespace cam {
namespace eng {
namespace gen {

void NgramLoader::loadNgram(const std::string& fileName) {
  std::ifstream file(fileName.c_str());
  //CHECK(file.is_open()) << "Cannot open file " << fileName;
  std::string line;
  // skip the first two lines which are the ITG rules
  std::getline(file, line);
  std::getline(file, line);
  std::vector<std::string> parts;
  std::vector<std::string> ngramString;
  std::vector<int> ngramInt;
  while (std::getline(file, line)) {
    boost::split(parts, line, boost::is_any_of(" "));
    //CHECK_EQ(parts.size(), 3) << "Wrong format, should have 3 parts: " << line;
    Coverage coverage(parts[1]);
    boost::split(ngramString, parts[1], boost::is_any_of("_"));
    ngramInt.resize(ngramString.size());
    std::transform(ngramString.begin(), ngramString.end(), ngramInt.begin(),
                   boost::lexical_cast<int, std::string>);
    Ngram ngram(coverage, ngramInt);
    ngrams_.push_back(ngram);
  }
}

const std::vector<Ngram>& NgramLoader::ngrams() const {
  return ngrams_;
}

} // namespace gen
} // namespace eng
} // namespace cam
