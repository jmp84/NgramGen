/*
 * Lattice.cpp
 *
 *  Created on: 24 Oct 2012
 *      Author: jmp84
 */

#include "Lattice.h"

#include <map>
#include <boost/lexical_cast.hpp>
#include <lm/state.hh>

#include "Column.h"
#include "NgramLoader.h"
#include "State.h"
#include "StateKey.h"
#include "Types.h"

DEFINE_int32(prune_nbest, 0, "N-best pruning: number of states kept in a"
             " column");
DEFINE_double(prune_threshold, 0, "Threshold pruning: add this threshold to "
              " the lowest cost in a column to define what states are kept.");

namespace cam {
namespace eng {
namespace gen {

Lattice::Lattice() : fst_(new fst::StdVectorFst()) {
  languageModel_ = NULL;
}

Lattice::~Lattice() {
  delete languageModel_;
}

void Lattice::init(const std::vector<int>& words, const std::string& lmfile) {
  fst_->DeleteStates();
  lattice_.clear();
  lattice_.resize(words.size() + 1);
  languageModel_ = new lm::ngram::Model(lmfile.c_str());
  Coverage emptyCoverage(words.size());
  // We initialize with a null context rather than a sentence begin context
  // because if we chop an input sentence, then the first word might not be a
  // sentence begin marker.
  lm::ngram::State initKenlmState(languageModel_->NullContextState());
  StateKey* initStateKey = new StateKey(emptyCoverage, initKenlmState);
  StateId startId = fst_->AddState();
  fst_->SetStart(startId);
  State* initState = new State(startId, initStateKey, 0);
  lattice_[0].statesIndexByStateKey_[*initStateKey] = initState;
  lattice_[0].statesSortedByCost_.insert(initState);
}

void Lattice::extend(const NgramLoader& ngramLoader, const int columnIndex) {
  CHECK_EQ(lattice_[columnIndex].statesIndexByStateKey_.size(),
           lattice_[columnIndex].statesSortedByCost_.size()) <<
               "Inconsistent number of states in column " << columnIndex;
  const std::map<Ngram, std::vector<Coverage> >& ngrams =
      ngramLoader.ngrams();
  std::set<State*, StatePointerComparator>::const_iterator stateIt =
        lattice_[columnIndex].statesSortedByCost_.begin();
  if (stateIt == lattice_[columnIndex].statesSortedByCost_.end()) {
    return;
  }
  int numStates = 0;
  Cost minCost = (*stateIt)->cost();
  Cost beam = minCost + FLAGS_prune_threshold;
  while (stateIt != lattice_[columnIndex].statesSortedByCost_.end()) {
    numStates++;
    if (FLAGS_prune_nbest > 0 && numStates > FLAGS_prune_nbest) {
      break;
    }
    if (FLAGS_prune_threshold > 0 && (*stateIt)->cost() > beam) {
      break;
    }
    for (std::map<Ngram, std::vector<Coverage> >::const_iterator ngramIt =
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
    ++stateIt;
  }
}

void Lattice::markFinalStates(const int length) {
  if (lattice_[length].empty()) {
    // Failure, there are no final states
    return;
  }
  for (std::set<State*, StatePointerComparator>::const_iterator it =
      lattice_[length].statesSortedByCost_.begin();
      it != lattice_[length].statesSortedByCost_.end(); ++it) {
    fst_->SetFinal((*it)->stateId(), fst::StdArc::Weight::One());
  }
}

void Lattice::compactFst() {
  fst::StdVectorFst* tempfst = new fst::StdVectorFst();
  fst::Determinize(*fst_, tempfst);
  fst_.reset(tempfst);
  fst::Minimize(fst_.get());
}

void Lattice::write(const string& filename) const {
  fst_->Write(filename);
}

void Lattice::extend(const State& state, const Ngram& ngram,
                     const Coverage& coverage) {
  Coverage newCoverage = state.coverage() | coverage;
  int columnIndex = newCoverage.count();
  lm::ngram::State* nextKenlmState = new lm::ngram::State();
  Cost applyNgramCost = cost(state, ngram, nextKenlmState) * (-log(10));
  Cost newCost = state.cost() + applyNgramCost;
  StateKey* newStateKey = new StateKey(newCoverage, *nextKenlmState);
  delete nextKenlmState;
  State* newState;
  boost::unordered_map<StateKey, State*>::const_iterator findNewStateKey =
      lattice_[columnIndex].statesIndexByStateKey_.find(*newStateKey);
  if (findNewStateKey != lattice_[columnIndex].statesIndexByStateKey_.end()) {
    // first case: the new coverage and history already exist
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
      delete newStateKey;
      newState = oldState;
    }
    // Now add states and arc for the n-gram
    StateId previousStateId = state.stateId();
    StateId nextStateId;
    fst::StdArc::Weight arcCost;
    for (int i = 0; i < ngram.size(); ++i) {
      if (i == ngram.size() - 1) {
        nextStateId = newState->stateId();
        arcCost = applyNgramCost;
      } else {
        nextStateId = fst_->AddState();
        arcCost = fst::StdArc::Weight::One();
      }
      fst_->AddArc(previousStateId,
                  fst::StdArc(ngram[i], ngram[i], arcCost, nextStateId));
      previousStateId = nextStateId;
    }
  } else {
    // second case: the new coverage and history don't already exist
    StateId previousStateId = state.stateId();
    StateId nextStateId;
    fst::StdArc::Weight arcCost;
    for (int i = 0; i < ngram.size(); ++i) {
      arcCost = (i == ngram.size() - 1) ? applyNgramCost : fst::StdArc::Weight::One();
      nextStateId = fst_->AddState();
      fst_->AddArc(previousStateId,
                  fst::StdArc(ngram[i], ngram[i], arcCost, nextStateId));
      previousStateId = nextStateId;
    }
    newState = new State(nextStateId, newStateKey, newCost);
    lattice_[columnIndex].statesIndexByStateKey_[*newStateKey] = newState;
    lattice_[columnIndex].statesSortedByCost_.insert(newState);
  }
}

Cost Lattice::cost(const State& state, const Ngram& ngram,
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
