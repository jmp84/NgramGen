/*
 * Lattice.cpp
 *
 *  Created on: 24 Oct 2012
 *      Author: jmp84
 */

#include "Lattice.h"

#include <map>
#include <boost/lexical_cast.hpp>
#include <boost/smart_ptr.hpp>
#include <lm/state.hh>

#include "Arc.h"
#include "Column.h"
#include "NgramLoader.h"
#include "State.h"
#include "StateKey.h"
#include "Types.h"

namespace cam {
namespace eng {
namespace gen {

Lattice::Lattice() : fst_(new fst::StdVectorFst()) {}

void Lattice::init(const std::vector<int>& words, const std::string& lmfile) {
  fst_->DeleteStates();
  lattice_.clear();
  lattice_.resize(words.size() + 1);
  languageModel_.reset(new lm::ngram::Model(lmfile.c_str()));
  Coverage emptyCoverage(words.size());
  // We initialize with a null context rather than a sentence begin context
  // because if we chop an input sentence, then the first word might not be a
  // sentence begin marker.
  lm::ngram::State initKenlmState(languageModel_->NullContextState());
  StateKey* initStateKey = new StateKey(emptyCoverage, initKenlmState);
  fst::StdArc::StateId startId = fst_->AddState();
  State* initState = new State(startId, initStateKey, 0);
  lattice_[0].statesIndexByStateKey_[*initStateKey] = initState;
  lattice_[0].statesSortedByCost_.insert(initState);
}

void Lattice::extend(const NgramLoader& ngramLoader, const int columnIndex) {
  const std::map<std::vector<int>, std::vector<Coverage> >& ngrams =
      ngramLoader.ngrams();
  for (std::multiset<State*, StatePointerComparator>::const_iterator stateIt =
      lattice_[columnIndex].statesSortedByCost_.begin();
      stateIt != lattice_[columnIndex].statesSortedByCost_.end(); ++stateIt) {
    for (std::map<std::vector<int>, std::vector<Coverage> >::const_iterator ngramIt =
        ngrams.begin(); ngramIt != ngrams.end(); ++ngramIt) {
      for (int i = 0; i < ngramIt->second.size(); ++i) {
        if ((*stateIt)->canApply(ngramIt->first, ngramIt->second[i])) {
          extend(**stateIt, ngramIt->first, ngramIt->second[i]);
          // we break to use only the first coverage of the ngram to avoid
          // spurious ambiguity (and therefore to have better pruning: e.g. if
          // we keep 2 states in a column and the first two correspond to the
          // same hypothesis)
          break;
        }
      }
    }
  }
}

void Lattice::pruneNbest(const int columnIndex, const int nbest) {
  CHECK_EQ(lattice_[columnIndex].statesIndexByStateKey_.size(),
           lattice_[columnIndex].statesSortedByCost_.size()) <<
               "Inconsistent number of states in column " << columnIndex;
  int count = 0;
  for (std::set<State*, StatePointerComparator>::iterator stateIterator =
      lattice_[columnIndex].statesSortedByCost_.begin();
      stateIterator != lattice_[columnIndex].statesSortedByCost_.end();) {
    count++;
    if (count > nbest) {
      lattice_[columnIndex].statesIndexByStateKey_.erase(*((*stateIterator)->stateKey()));
      fst_->DeleteStates(std::vector<fst::StdArc::StateId>(1, (*stateIterator)->stateId()));
      lattice_[columnIndex].statesSortedByCost_.erase(stateIterator++);
    } else {
      ++stateIterator;
    }
  }
}

void Lattice::pruneThreshold(const int columnIndex, const Cost threshold) {
  CHECK_EQ(lattice_[columnIndex].statesIndexByStateKey_.size(),
           lattice_[columnIndex].statesSortedByCost_.size()) <<
               "Inconsistent number of states in column " << columnIndex;
  std::multiset<State*, StatePointerComparator>::iterator stateIterator =
      lattice_[columnIndex].statesSortedByCost_.begin();
  if (stateIterator == lattice_[columnIndex].statesSortedByCost_.end()) {
    // the column is empty, nothing to be done
    return;
  }
  Cost minCost = (*stateIterator)->cost();
  while (stateIterator != lattice_[columnIndex].statesSortedByCost_.end()) {
    Cost cost = (*stateIterator)->cost();
    if (cost > minCost + threshold) {
      lattice_[columnIndex].statesIndexByStateKey_.erase(
          *((*stateIterator)->stateKey()));
      fst_->DeleteStates(std::vector<fst::StdArc::StateId>(1, (*stateIterator)->stateId()));
      lattice_[columnIndex].statesSortedByCost_.erase(stateIterator++);
    } else {
      ++stateIterator;
    }
  }
}

void Lattice::compactFst() {
  fst::StdVectorFst tempfst;
  fst::Determinize(*fst_, &tempfst);
  fst_.reset(&tempfst);
  fst::Minimize(fst_.get());
}

void Lattice::write(const string& filename) const {
  fst_->Write(filename);
}

void Lattice::extend(const State& state, const vector<int>& ngram,
                     const Coverage& coverage) {
  Coverage newCoverage = state.coverage() | coverage;
  int columnIndex = newCoverage.count();
  lm::ngram::State nextKenlmState;
  Cost applyNgramCost = cost(state, ngram, &nextKenlmState) * (-log(10));
  Cost newCost = state.cost() + applyNgramCost;
  StateKey* newStateKey = new StateKey(newCoverage, nextKenlmState);
  State* newState;
  std::map<StateKey, State*>::const_iterator findNewStateKey =
      lattice_[columnIndex].statesIndexByStateKey_.find(*newStateKey);
  if (findNewStateKey != lattice_[columnIndex].statesIndexByStateKey_.end()) {
    Cost existingCost = findNewStateKey->second->cost();
    State* oldState = findNewStateKey->second;
    // if the new cost is smaller, then we need to remove the state from the
    // set containing states sorted by cost then reinsert a new state so the
    // ordering is still correct.
    if (newCost < existingCost) {
      newState = new State(oldState->stateId(), newStateKey, newCost);
      lattice_[columnIndex].statesIndexByStateKey_[*newStateKey] = newState;
      lattice_[columnIndex].statesSortedByCost_.erase(oldState);
      lattice_[columnIndex].statesSortedByCost_.insert(newState);
      delete oldState;
    } else {
      newState = oldState;
    }
    fst::StdArc::StateId previousStateId = state.stateId();
    fst::StdArc::StateId nextStateId;
    for (int i = 0; i < ngram.size(); ++i) {
      if (i == ngram.size() - 1) {
        nextStateId = newState->stateId();
      } else {
        nextStateId = fst_->AddState();
      }
      fst_->AddArc(previousStateId,
                  fst::StdArc(ngram[i], ngram[i],
                              i == 0 ? applyNgramCost : fst::StdArc::Weight::One(),
                              nextStateId));
      previousStateId = nextStateId;
    }
  } else {
    fst::StdArc::StateId previousStateId = state.stateId();
    fst::StdArc::StateId nextStateId;
    for (int i = 0; i < ngram.size(); ++i) {
      fst::StdArc::StateId nextStateId = fst_->AddState();
      fst_->AddArc(previousStateId,
                  fst::StdArc(ngram[i], ngram[i],
                              i == 0 ? applyNgramCost : fst::StdArc::Weight::One(),
                              nextStateId));
      previousStateId = nextStateId;
    }
    newState = new State(nextStateId, newStateKey, newCost);
    lattice_[columnIndex].statesIndexByStateKey_[*newStateKey] = newState;
    lattice_[columnIndex].statesSortedByCost_.insert(newState);
  }
}

Cost Lattice::cost(const State& state, const vector<int>& ngram,
                   lm::ngram::State* nextKenlmState) const {
  Cost res = 0;
  lm::ngram::State startKenlmStateTemp;
  const lm::ngram::Vocabulary& vocab = languageModel_->GetVocabulary();
  for (int i = 0; i < ngram.size(); i++) {
    std::string word;
    if (ngram[i] == 2) {
      word = "</s>";
    } else if (ngram[i] == 1) {
      CHECK_EQ(0, i) << "Ngram with a start-of-sentence marker in the middle.";
      startKenlmStateTemp = languageModel_->BeginSentenceState();
      continue;
    } else {
      word = boost::lexical_cast<std::string>(ngram[i]);
    }
    if (i == 0) {
      startKenlmStateTemp = state.getKenlmState();
    } else if (i > 1 || ngram[0] != 1) {
      startKenlmStateTemp = *nextKenlmState;
    }
    // else startKenlmStateTemp has been set to
    // languageModel_->BeginSentenceState()
    res += languageModel_->Score(startKenlmStateTemp, vocab.Index(word),
                                 *nextKenlmState);
  }
  return res;
}

} // namespace gen
} // namespace eng
} // namespace cam
