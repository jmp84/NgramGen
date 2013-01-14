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

#include "features/RuleCostComputer.h"
#include "NgramLoader.h"
#include "State.h"
#include "StateKey.h"
#include "Types.h"

namespace cam {
namespace eng {
namespace gen {

// start and end-of-sentence symbols
enum {
  STARTSENTENCE = 1,
  ENDSENTENCE = 2
};

Lattice::Lattice(const std::vector<int>& words, const std::string& lmfile,
                 const std::vector<std::string>& featureNames,
                 const Weights& weights) :
    fst_(new fst::StdVectorFst()), columns_(words.size() + 1),
    languageModel_(new lm::ngram::Model(lmfile.c_str())), inputWords_(words),
    featureNames_(featureNames), weights_(weights) {
  Coverage emptyCoverage(words.size());
  // We initialize with a null context rather than a sentence begin context
  // because if we chop an input sentence, then the first word might not be a
  // sentence begin marker.
  lm::ngram::State initKenlmState(languageModel_->NullContextState());
  StateKey* initStateKey = new StateKey(emptyCoverage, initKenlmState);
  StateId startId = fst_->AddState();
  fst_->SetStart(startId);
  State* initState = new State(startId, initStateKey, 0, true);
  columns_[0].statesIndexByStateKey_[*initStateKey] = initState;
  columns_[0].statesSortedByCost_.insert(initState);
}

Lattice::~Lattice() {
  delete languageModel_;
}

void Lattice::extend(const NgramLoader& ngramLoader, const int columnIndex,
                     const int pruneNbest, const float pruneThreshold,
                     const int maxOverlap) {
  CHECK_EQ(columns_[columnIndex].statesIndexByStateKey_.size(),
           columns_[columnIndex].statesSortedByCost_.size()) <<
               "Inconsistent number of states in column " << columnIndex;
  VLOG(1) << "Number of states in column " << columnIndex << " " <<
      columns_[columnIndex].statesSortedByCost_.size();
  const std::map<Ngram, std::vector<Coverage> >& ngrams =
      ngramLoader.ngrams();
  std::set<State*, StatePointerComparator>::const_iterator stateIt =
        columns_[columnIndex].statesSortedByCost_.begin();
  if (stateIt == columns_[columnIndex].statesSortedByCost_.end()) {
    return;
  }
  int numStates = 0;
  Cost minCost = (*stateIt)->cost();
  Cost beam = minCost + pruneThreshold;
  while (stateIt != columns_[columnIndex].statesSortedByCost_.end()) {
    numStates++;
    if ((pruneNbest > 0 && numStates > pruneNbest) ||
        (pruneThreshold > 0 && (*stateIt)->cost() > beam)) {
      break;
    }
    for (std::map<Ngram, std::vector<Coverage> >::const_iterator ngramIt =
        ngrams.begin(); ngramIt != ngrams.end(); ++ngramIt) {
      for (int i = 0; i < ngramIt->second.size(); ++i) {
        Ngram ngramToApply;
        if (canApply(**stateIt, ngramIt->first, ngramIt->second[i],
                     maxOverlap, &ngramToApply)) {
          extend(**stateIt, ngramToApply, ngramIt->second[i], pruneNbest,
                 pruneThreshold);
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
  if (columns_[length].empty()) {
    // Failure, there are no final states
    return;
  }
  for (std::set<State*, StatePointerComparator>::const_iterator it =
      columns_[length].statesSortedByCost_.begin();
      it != columns_[length].statesSortedByCost_.end(); ++it) {
    fst_->SetFinal((*it)->stateId(), fst::StdArc::Weight::One());
  }
}

void Lattice::addInput() {
  State* startState = *(columns_[0].statesSortedByCost_.begin());
  lm::ngram::State endKenlmState;
  Cost costInput = cost(*startState, inputWords_, &endKenlmState) * (-log(10));
  StateId id = fst_->Start();
  StateId nextId;
  fst::StdArc::Weight arcCost;
  const lm::ngram::Vocabulary& vocab = languageModel_->GetVocabulary();
  for (int i = 0; i < inputWords_.size(); ++i) {
    arcCost = (i == inputWords_.size() - 1) ? costInput : fst::StdArc::Weight::One();
    nextId = fst_->AddState();
    fst_->AddArc(id, fst::StdArc(inputWords_[i], inputWords_[i], arcCost, nextId));
    id = nextId;
  }
  fst_->SetFinal(nextId, fst::StdArc::Weight::One());
}

void Lattice::compactFst(const int pruneWeight) {
  fst::Connect(&(*fst_));
  if (pruneWeight > 0) {
    fst::StdArc::Weight w = pruneWeight;
    fst::Prune(&*fst_, w);
  }
  fst::StdVectorFst* tempfst = new fst::StdVectorFst();
  fst::Determinize(*fst_, tempfst);
  fst_.reset(tempfst);
  fst::Minimize(fst_.get());
}

void Lattice::write(const string& filename) const {
  fst_->Write(filename);
}

void Lattice::whenLostInput() const {
  bool hasInput = false;
  for (int columnIndex = columns_.size() - 1; columnIndex >= 0; --columnIndex) {
    for (std::set<State*, StatePointerComparator>::const_iterator stateIt =
        columns_[columnIndex].statesSortedByCost_.begin();
        stateIt != columns_[columnIndex].statesSortedByCost_.end();
        ++stateIt) {
      if ((*stateIt)->hasInput()) {
        hasInput = true;
        break;
      }
    }
    if (hasInput && columnIndex == columns_.size() - 1) {
      return;
    }
    if (hasInput) {
      LOG(INFO) << "Partial input was last seen in column " << columnIndex;
      return;
    }
  }
}

bool Lattice::compatibleHistory(const State& state, const Ngram& ngram,
                                const Coverage& overlap,
                                const int overlapCount) const {
  if (overlapCount == 0) {
    return true;
  }
  // first check that the first words in the ngram correspond to the history
  std::set<int> overlappingWords;
  for (int i = 0; i < overlapCount; ++i) {
    lm::WordIndex windex = index(ngram[i]);
    if (windex != state.getKenlmState().words[overlapCount - 1 -i]) {
      return false;
    }
    overlappingWords.insert(ngram[i]);
  }
  // then check that the first words in the ngram correspond to the input words
  // corresponding to the overlap bits
  int overlapSize = overlap.size();
  for (int i = 0; i < overlapSize; ++i) {
    if (overlap.test(i)) {
      if (overlappingWords.find(inputWords_[overlapSize - i - 1]) ==
          overlappingWords.end()) {
        return false;
      }
    }
  }
  return true;
}

bool Lattice::canApply(const State& state, const Ngram& ngram,
                       const Coverage& coverage, const int maxOverlap,
                       Ngram* ngramToApply) const {
  Coverage ol = state.coverage() & coverage;
  int olcount = ol.count();
  // olcount cannot be greater than the maximum overlap allowed (FLAGS_overlap)
  // olcount cannot be greater than the size of the history
  if (olcount > maxOverlap || olcount >= languageModel_->Order()) {
    return false;
  }
  // ol cannot be included in the current coverage otherwise we don't extend
  // anything. In the special case were ol is the empty set (for example in the
  // initial state), we don't want coverage to be the empty set either.
  if (ol == coverage) {
    return false;
  }
  // Check that the history of this state is compatible with the ngram. True
  // if the overlap is empty.
  if (!compatibleHistory(state, ngram, ol, olcount)) {
    return false;
  }
  // check that if the ngram starts with start-of-sentence, then the current
  // state is initial (that is, has an empty coverage)
  if (ngram[0] == STARTSENTENCE && !state.isInitial()) {
    return false;
  }
  // checks that if the ngram ends with end-of-sentence, then the resulting
  // coverage will cover all words
  if (ngram[ngram.size() - 1] == ENDSENTENCE) {
    int sizeNextCoverage =
        state.coverage().count() + coverage.count() - olcount;
    if (sizeNextCoverage < coverage.size()) {
      return false;
    }
  }
  // at this point, we can apply the ngram. We need to truncate it in case there
  // is a non trivial overlap
  *ngramToApply = ngram;
  for (int i = 0; i < olcount; ++i) {
    ngramToApply->erase(ngramToApply->begin());
  }
  return true;
}

void Lattice::extend(const State& state, const Ngram& ngram,
                     const Coverage& coverage, const int pruneNbest,
                     const float pruneThreshold) {
  Coverage newCoverage = state.coverage() | coverage;
  int columnIndex = newCoverage.count();
  lm::ngram::State* nextKenlmState = new lm::ngram::State();
  Cost applyNgramCost = cost(state, ngram, nextKenlmState);
  Cost newCost = state.cost() + applyNgramCost;
  StateKey* newStateKey = new StateKey(newCoverage, *nextKenlmState);
  delete nextKenlmState;
  State* newState;
  boost::unordered_map<StateKey, State*>::const_iterator findNewStateKey =
      columns_[columnIndex].statesIndexByStateKey_.find(*newStateKey);
  // check if the new state will contain a hypothesis corresponding to the
  // partial input
  bool hasInput = checkNextStateHasInput(state, columnIndex, ngram);
  if (findNewStateKey != columns_[columnIndex].statesIndexByStateKey_.end()) {
    // first case: the new coverage and history already exist
    Cost existingCost = findNewStateKey->second->cost();
    State* oldState = findNewStateKey->second;
    // if the new cost is smaller, then we need to remove the state from the
    // set containing states sorted by cost then reinsert a new state so the
    // ordering is still correct.
    if (newCost < existingCost) {
      newState = new State(oldState->stateId(), newStateKey, newCost,
                           hasInput || oldState->hasInput());
      columns_[columnIndex].statesIndexByStateKey_[*newStateKey] = newState;
      columns_[columnIndex].statesSortedByCost_.erase(oldState);
      columns_[columnIndex].statesSortedByCost_.insert(newState);
      delete oldState;
    } else {
      delete newStateKey;
      newState = oldState;
      if (hasInput) {
        newState->setHasInput(hasInput);
      }
    }
    // Now add states and arc for the n-gram
    addFstStatesAndArcs(state, ngram, newState, applyNgramCost);
  } else {
    // second case: the new coverage and history don't already exist
    // TODO refactor this bit
    if (pruneNbest > 0 && columnIndex != inputWords_.size() &&
        columns_[columnIndex].statesSortedByCost_.size() >= pruneNbest) {
      std::set<State*, StatePointerComparator>::const_iterator stateIt =
          columns_[columnIndex].statesSortedByCost_.end();
      stateIt--;
      Cost maxCost = (*stateIt)->cost();
      if (newCost >= maxCost) {
        return;
      }
      StateId nextStateId =
          addFstStatesAndArcsNewState(state, ngram, applyNgramCost);
      newState = new State(nextStateId, newStateKey, newCost, hasInput);
      columns_[columnIndex].statesIndexByStateKey_[*newStateKey] = newState;
      columns_[columnIndex].statesSortedByCost_.insert(newState);
      stateIt = columns_[columnIndex].statesSortedByCost_.end();
      stateIt--;
      columns_[columnIndex].statesIndexByStateKey_.erase(*((*stateIt)->stateKey()));
      columns_[columnIndex].statesSortedByCost_.erase(stateIt);
    }
    else if (pruneThreshold > 0 && columnIndex != inputWords_.size() &&
        !columns_[columnIndex].empty()) {
      std::set<State*, StatePointerComparator>::const_iterator stateIt =
          columns_[columnIndex].statesSortedByCost_.begin();
      Cost minCost = (*stateIt)->cost();
      Cost beam = minCost + pruneThreshold;
      if (newCost > beam) {
        return;
      }
      // TODO if newCost is the best cost, the beam changes and we need to
      // remove states!!!
    } else {
      StateId nextStateId =
          addFstStatesAndArcsNewState(state, ngram, applyNgramCost);
      newState = new State(nextStateId, newStateKey, newCost, hasInput);
      columns_[columnIndex].statesIndexByStateKey_[*newStateKey] = newState;
      columns_[columnIndex].statesSortedByCost_.insert(newState);
    }
  }
}

Cost Lattice::cost(const State& state, const Ngram& rule,
                   lm::ngram::State* nextKenlmState) const {
  Cost res = 0;
  Cost lmc = lmCost(state, rule, nextKenlmState);
  Cost rc = ruleCost(rule, weights_, featureNames_);
  res += weights_.getWeight("lm") * lmc + rc;
  return res;
}

Cost Lattice::lmCost(const State& state, const Ngram& rule,
                     lm::ngram::State* nextKenlmState) const {
  Cost res = 0;
  lm::ngram::State startKenlmStateTemp;
  const lm::ngram::Vocabulary& vocab = languageModel_->GetVocabulary();
  for (int i = 0; i < rule.size(); i++) {
    if (rule[i] == STARTSENTENCE) {
      CHECK_EQ(0, i) << "Ngram with a start-of-sentence marker in the middle.";
      startKenlmStateTemp = languageModel_->BeginSentenceState();
      continue;
    }
    if (i == 0) {
      startKenlmStateTemp = state.getKenlmState();
    } else if (i > 1 || rule[0] != 1) {
      startKenlmStateTemp = *nextKenlmState;
    }
    // else startKenlmStateTemp has been set to
    // languageModel_->BeginSentenceState()
    res += languageModel_->Score(startKenlmStateTemp, index(rule[i]),
                                 *nextKenlmState);
  }
  return res * (-log(10));
}

void Lattice::addFstStatesAndArcs(const State& state, const Ngram& ngram,
                                  const State* newState,
                                  const float applyNgramCost) {
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
}

Lattice::StateId Lattice::addFstStatesAndArcsNewState(
    const State& state, const Ngram& ngram, const float applyNgramCost) {
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
  return nextStateId;
}

const lm::WordIndex Lattice::index(int id) const {
  const lm::ngram::Vocabulary& vocab = languageModel_->GetVocabulary();
  if (id == ENDSENTENCE) {
    return vocab.Index("</s>");
  }
  if (id == STARTSENTENCE) {
    return vocab.Index("<s>");
  }
  return vocab.Index(boost::lexical_cast<std::string>(id));
}

const bool Lattice::checkNextStateHasInput(
    const State& state, const int columnIndex, const Ngram& ngram) const {
  if (!state.hasInput()) {
    return false;
  }
  int previousColumnIndex = columnIndex - ngram.size();
  for (int i = 0; i < ngram.size(); ++i) {
    if (ngram[i] != inputWords_[previousColumnIndex + i]) {
      return false;
    }
  }
  return true;
}

} // namespace gen
} // namespace eng
} // namespace cam
