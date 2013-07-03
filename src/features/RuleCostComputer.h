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
 * @param state The state we start from.
 * @param rule The rule we apply.
 * @param languageModel The language model.
 * @param nextKenlmState The kenlm state we end up in.
 * @return The language model cost for the rule.
 */
Cost lmCost(const State& state, const Ngram& rule,
            boost::shared_ptr<lm::ngram::Model> languageModel,
            lm::ngram::State* nextKenlmState);

/**
 * Computes the language model cost for a deletion rule.
 * @param state The state we start from.
 * @param rule The rule we apply.
 * @param nextKenlmState The kenlm state we end up in.
 * @return The language model cost for the deletion rule (zero).
 */
Cost lmCostDeletion(const State& state, const Ngram& rule,
                    lm::ngram::State* nextKenlmState);

/**
 * Struct to compute rule cost and the rule weight. The cost is always the dot
 * product feature values/feature weights. The weight depends on the template.
 * In the standard semiring, the weight is equal to the cost. In the sparse
 * tuple weight semiring, the cost is the vector of feature values.
 */
template <class Arc = fst::StdArc>
struct RuleCostAndWeightComputer {
  /**
   * Computes the cost for a normal rule.
   * @param state The start state.
   * @param rule The rule.
   * @param weights The feature weights.
   * @param featureNames The list of feature names.
   * @param languageModel The language model.
   * @param nextKenlmState The kenlm state we end up in.
   * @param weight The weight (cost for std semiring)
   * @return The cost of the rule.
   */
  const Cost compute(
      const State& state, const Ngram& rule, const Weights& weights,
      const std::vector<std::string>& featureNames,
      boost::shared_ptr<lm::ngram::Model> languageModel,
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

  /**
   * Computes the cost for a deletion rule.
   * @param state The start state.
   * @param rule The rule.
   * @param weights The feature weights.
   * @param featureNames The list of feature names.
   * @param nextKenlmState The kenlm state we end up in.
   * @param weight The weight (cost for std semiring).
   * @return The cost of the deletion rule.
   */
  const Cost computeDeletion(
      const State& state, const Ngram& rule, const Weights& weights,
      const std::vector<std::string>& featureNames,
      lm::ngram::State* nextKenlmState, typename Arc::Weight* weight) {
    Cost res = lmCostDeletion(state, rule, nextKenlmState);
    for (int i = 0; i < featureNames.size(); ++i) {
      boost::shared_ptr<Feature> feature =
          FeatureFactory::createFeature(featureNames[i]);
      res +=
          feature->getValueDeletion(rule) * weights.getWeight(featureNames[i]);
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
  /**
   * Computes the cost for a normal rule.
   * @param state The start state.
   * @param rule The rule.
   * @param weights The feature weights.
   * @param featureNames The list of feature names.
   * @param languageModel The language model.
   * @param nextKenlmState The kenlm state we end up in.
   * @param weight The weight (vector of feature values in sparse tuple weight
   * semiring)
   * @return The cost of the rule.
   */
  const Cost compute(
      const State& state, const Ngram& rule, const Weights& weights,
      const std::vector<std::string>& featureNames,
      boost::shared_ptr<lm::ngram::Model> languageModel,
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

  /**
   * Computes the cost for a deletion rule.
   * @param state The start state.
   * @param rule The rule.
   * @param weights The feature weights.
   * @param featureNames The list of feature names.
   * @param nextKenlmState The kenlm state we end up in.
   * @param weight The weight (vector of feature values in sparse tuple weight
   * semiring)
   * @return The cost of the deletion rule.
   */
  const Cost computeDeletion(
      const State& state, const Ngram& rule, const Weights& weights,
      const std::vector<std::string>& featureNames,
      lm::ngram::State* nextKenlmState, TupleW32* weight) {
    Cost res = lmCostDeletion(state, rule, nextKenlmState);
    // Warning: here the first index is 1 because 0 is reserved by
    // TropicalSparseTupleWeight
    weight->Push(1, res);
    for (int i = 0; i < featureNames.size(); ++i) {
      boost::shared_ptr<Feature> feature =
          FeatureFactory::createFeature(featureNames[i]);
      Cost featureValue = feature->getValueDeletion(rule);
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
