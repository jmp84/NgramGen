/*
 * RuleCostComputer.h
 *
 *  Created on: 10 Jan 2013
 *      Author: jmp84
 */

#ifndef RULECOSTCOMPUTER_H_
#define RULECOSTCOMPUTER_H_

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

#include <lm/model.hh>

#include "features/Feature.h"
#include "features/FeatureFactory.h"
#include "features/Weights.h"

#include "include/params.h"
#include "include/tropical-sparse-tuple-weight-incls.h"
#include "include/tropical-sparse-tuple-weight.h"
#include "include/tropical-sparse-tuple-weight-decls.h"
#include "include/tropical-sparse-tuple-weight.makeweight.h"

#include "Types.h"

namespace cam {
namespace eng {
namespace gen {

class State;

/**
 * Computes the language model cost of a rule starting in a certain state
 * (with a certain history).
 * @param state The start we start from.
 * @param rule The rule we apply.
 * @param languageModel The language model.
 * @param nextKenlmState The kenlm state we end up in.
 * @return
 */
Cost lmCost(const State& state, const Ngram& rule,
            lm::ngram::Model* languageModel,
            lm::ngram::State* nextKenlmState);

/**
 * Functor to compute rule cost and the rule weight. The cost is always the dot
 * product feature values/feature weights. The weight depends on the template.
 * In the standard semiring, the weight is equal to the cost. In the sparse
 * tuple weight semiring, the cost is the vector of feature values.
 */
template <class Arc = fst::StdArc>
struct RuleCostAndWeightComputer {
  const Cost operator() (
      const State& state, const Ngram& rule, const Weights& weights,
      const std::vector<std::string>& featureNames,
      lm::ngram::Model* languageModel,
      lm::ngram::State* nextKenlmState, typename Arc::Weight* weight) {
    Cost res = lmCost(state, rule, languageModel, nextKenlmState);
    for (int i = 0; i < featureNames.size(); ++i) {
      boost::shared_ptr<Feature> feature =
          FeatureFactory::createFeature(featureNames[i]);
      res += feature->getValue(rule) * weights.getWeight(featureNames[i]);
    }
    *weight = res;
    return res;
  }
};

/**
 * Template specialization in the case of a sparse tuple weight semiring.
 */
template <>
struct RuleCostAndWeightComputer<TupleArc32> {
  const Cost operator() (
      const State& state, const Ngram& rule, const Weights& weights,
      const std::vector<std::string>& featureNames,
      lm::ngram::Model* languageModel,
      lm::ngram::State* nextKenlmState, TupleW32* weight) {
    Cost res = lmCost(state, rule, languageModel, nextKenlmState);
    // Warning: here the first index is 1 because 0 is reserved by
    // TropicalSparseTupleWeight
    weight->Push(1, res);
    for (int i = 0; i < featureNames.size(); ++i) {
      boost::shared_ptr<Feature> feature =
          FeatureFactory::createFeature(featureNames[i]);
      Cost featureValue = feature->getValue(rule);
      res += featureValue * weights.getWeight(featureNames[i]);
      weight->Push(i+2, featureValue);
    }
    return res;
  }
};

} // namespace gen
} // namespace eng
} // namespace cam

#endif /* RULECOSTCOMPUTER_H_ */
