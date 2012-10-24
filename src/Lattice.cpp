/*
 * Lattice.cpp
 *
 *  Created on: 24 Oct 2012
 *      Author: jmp84
 */

#include <map>

#include <boost/lexical_cast.hpp>

#include "lm/model.hh"

#include "Lattice.h"
#include "Column.h"
#include "Ngram.h"
#include "State.h"
#include "Types.h"

namespace cam {
namespace eng {
namespace gen {

Cost Lattice::cost(const State& state, const Ngram& ngram,
          lm::ngram::State* nextKenlmState) const {
  Cost res = 0;
  lm::ngram::State startKenlmState = state.getKenlmState();
  const lm::ngram::Vocabulary& vocab = languageModel_->GetVocabulary();
  const std::vector<int> words = ngram.ngram();
  for (int i = 0; i < boost::lexical_cast<int>(words.size()); i++) {
    res = languageModel_->Score(startKenlmState,
                                vocab.Index(
                                    boost::lexical_cast<std::string>(words)),
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

} // namespace gen
} // namespace eng
} // namespace cam
