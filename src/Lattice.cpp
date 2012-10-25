/*
 * Lattice.cpp
 *
 *  Created on: 24 Oct 2012
 *      Author: jmp84
 */

#include "Lattice.h"

#include <map>
#include <boost/lexical_cast.hpp>
#include <fst/fstlib.h>

#include "Column.h"
#include "Ngram.h"
#include "State.h"
#include "Types.h"

namespace cam {
namespace eng {
namespace gen {

void Lattice::init(const std::vector<int>& words) {
  lattice_.resize(words.size());
  Coverage emptyCoverage(words.size());
  lm::ngram::State beginSentenceState(languageModel_->BeginSentenceState());
  StateKey initStateKey(emptyCoverage, beginSentenceState);
  State initState(&initStateKey, 0);
  lattice_[0].statesIndexByStateKey_[initStateKey] = &State;
  lattice_[0].statesSortedByCost_.insert(initState);
}

Cost Lattice::cost(const State& state, const Ngram& ngram,
          lm::ngram::State* nextKenlmState) const {
  Cost res = 0;
  lm::ngram::State startKenlmState = state.getKenlmState();
  const lm::ngram::Vocabulary& vocab = languageModel_->GetVocabulary();
  const std::vector<int> words = ngram.ngram();
  for (int i = 0; i < static_cast<int>(words.size()); i++) {
    res = languageModel_->Score(startKenlmState,
                                vocab.Index(
                                    boost::lexical_cast<std::string>(words[i])),
                                *nextKenlmState);
    startKenlmState = *nextKenlmState;
  }
  return res;
}

void Lattice::extend(const State& state, const Ngram& ngram) {
  Coverage newCoverage = state.coverage() | ngram.coverage();
  int columnIndex = newCoverage.count();
  lm::ngram::State nextKenlmState;
  Cost newCost = cost(state, ngram, &nextKenlmState);
  //const State* statePointer = &state;
  //const Ngram* ngramPointer = &ngram;
  //Arc newArc(statePointer, ngramPointer);
  Arc newArc(&state, &ngram);
  StateKey newStateKey(newCoverage, nextKenlmState);
  State* newState;
  std::map<StateKey, State*>::const_iterator findNewStateKey =
      lattice_[columnIndex].statesIndexByStateKey_.find(newStateKey);
  if (findNewStateKey != lattice_[columnIndex].statesIndexByStateKey_.end()) {
    newState = findNewStateKey->second;
    Cost existingCost = newState->cost();
    if (newCost < existingCost) {
      newState->setCost(newCost);
      // TODO here we need the column to be resorted if we modify the cost.
    }
  } else {
    newState = new State(&newStateKey, newCost);
    lattice_[columnIndex].statesSortedByCost_.insert(*newState);
    lattice_[columnIndex].statesIndexByStateKey_[newStateKey] = newState;
  }
  newState->addArc(newArc);
}

void Lattice::prune(int columnIndex, int nbest) {
  int count = 0;
  for (std::set<State>::iterator stateIterator =
      lattice_[columnIndex].statesSortedByCost_.begin();
      stateIterator != lattice_[columnIndex].statesSortedByCost_.end();) {
    count++;
    if (count > nbest) {
      lattice_[columnIndex].statesIndexByStateKey_.erase(
          *stateIterator->stateKey());
      lattice_[columnIndex].statesSortedByCost_.erase(stateIterator++);
    } else {
      ++stateIterator;
    }
  }
}

void Lattice::prune(int columnIndex, Cost threshold) {
  std::multiset<State>::iterator stateIterator =
      lattice_[columnIndex].statesSortedByCost_.begin();
  if (stateIterator == lattice_[columnIndex].statesSortedByCost_.end()) {
    // the column is empty, nothing to be done
    return;
  }
  Cost minCost = stateIterator->cost();
  while (stateIterator != lattice_[columnIndex].statesSortedByCost_.end()) {
    Cost cost = stateIterator->cost();
    if (cost > minCost + threshold) {
      lattice_[columnIndex].statesIndexByStateKey_.erase(
          *stateIterator->stateKey());
      lattice_[columnIndex].statesSortedByCost_.erase(stateIterator++);
    } else {
      ++stateIterator;
    }
  }
}

void Lattice::convert2openfst(int length, fst::StdVectorFst* res) {
  typedef fst::StdArc::StateId StateId;
  // if there is no state in the last column, then failure, return a null fst.
  if (lattice_[length].empty()) {
    delete res;
    res = NULL;
    return;
  }
  // TODO clear the fst first
  res->AddState();
  res->SetStart(0);
  std::stack<State*> statesToBeProcessed;
  std::stack<StateId> openfstStatesToBeProcessed;
  for (std::multiset<State>::iterator it =
      lattice_[length].statesSortedByCost_.begin();
      it != lattice_[length].statesSortedByCost_.end(); it++) {
    statesToBeProcessed.push(&(*it));
    StateId stateId = res->AddState();
    openfstStatesToBeProcessed.push(stateId);
    // Add an epsilon transition from the initial state to the lattice final
    // states
    res->AddArc(0, fst::StdArc(0, 0, fst::StdArc::Weight::Zero(), stateId));
  }
  while (!statesToBeProcessed.empty()) {
    State* stateToBeProcessed = statesToBeProcessed.top();
    statesToBeProcessed.pop();
    StateId openfstStateToBeProcessed = openfstStatesToBeProcessed.top();
    openfstStatesToBeProcessed.pop();
    const vector<Arc> incomingArcs = stateToBeProcessed->incomingArcs();
    for (int i = 0; i < incomingArcs.size(); i++) {
      StateId stateId = res->AddState();
      openfstStatesToBeProcessed.push(stateId);
      res->AddArc(openfstStateToBeProcessed,
                  fst::StdArc(incomingArcs[i].ngram()->ngram()[0],
                              incomingArcs[i].ngram()->ngram()[0],
                              fst::StdArc::Weight::Zero(), stateId));
    }
  }
  // invert
  // determinize
  // minimize
  // write result maybe
}

} // namespace gen
} // namespace eng
} // namespace cam
