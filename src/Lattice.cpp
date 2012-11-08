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

Lattice::~Lattice() {
  delete languageModel_;
}

void Lattice::init(const std::vector<int>& words, const std::string& lmfile) {
  lattice_.clear();
  lattice_.resize(words.size() + 1);
  languageModel_ = new lm::ngram::Model(lmfile.c_str());
  Coverage emptyCoverage(words.size());
  // We initialize with a null context rather than a sentence begin context
  // because if we chop an input sentence, then the first word might not be a
  // sentence begin marker.
  lm::ngram::State initKenlmState(languageModel_->NullContextState());
  StateKey* initStateKey = new StateKey(emptyCoverage, initKenlmState);
  State* initState = new State(initStateKey, 0, std::vector<Arc>());
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
          // spurious ambiguity
          break;
        }
      }
    }
  }
}

void Lattice::pruneNbest(const int columnIndex, const int nbest) {
  int count = 0;
  for (std::set<State*, StatePointerComparator>::iterator stateIterator =
      lattice_[columnIndex].statesSortedByCost_.begin();
      stateIterator != lattice_[columnIndex].statesSortedByCost_.end();) {
    count++;
    if (count > nbest) {
      lattice_[columnIndex].statesIndexByStateKey_.erase(*((*stateIterator)->stateKey()));
      lattice_[columnIndex].statesSortedByCost_.erase(stateIterator++);
    } else {
      ++stateIterator;
    }
  }
}

void Lattice::pruneThreshold(const int columnIndex, const Cost threshold) {
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
      lattice_[columnIndex].statesSortedByCost_.erase(stateIterator++);
    } else {
      ++stateIterator;
    }
  }
}

void Lattice::convert2openfst(const int length, fst::StdVectorFst* res) const {
  // clean up first
  res->DeleteStates();
  // if there is no state in the last column, then failure, return an empty fst.
  if (lattice_[length].empty()) {
    return;
  }
  fst::StdVectorFst tempres1;
  std::stack<State*> statesToBeProcessed;
  std::stack<StateId> openfstStatesToBeProcessed;
  // Add initial states and epsilon transitions to the lattice final states
  convert2openfstInit(length, &statesToBeProcessed,
                      &openfstStatesToBeProcessed, &tempres1);
  while (!statesToBeProcessed.empty()) {
    // Follows incoming arcs and processes the stacks.
    convert2openfstProcess(&statesToBeProcessed, &openfstStatesToBeProcessed,
                           &tempres1);
  }
  // some final fst operations
  convert2openfstFinal(tempres1, res);
}

const string Lattice::print() const {
  std::ostringstream res;
  for (int i = 0; i < lattice_.size(); i++) {
    res << "COLUMN " << i << std::endl;
    res << printColumn(i);
  }
  return res.str();
}

void Lattice::extend(const State& state, const vector<int>& ngram,
                     const Coverage& coverage) {
  Coverage newCoverage = state.coverage() | coverage;
  int columnIndex = newCoverage.count();
  lm::ngram::State nextKenlmState;
  Cost applyNgramCost = cost(state, ngram, &nextKenlmState) * (-log(10));
  Cost newCost = state.cost() + applyNgramCost;
  Arc newArc(const_cast<State*>(&state), const_cast<std::vector<int>*>(&ngram),
             applyNgramCost);
  StateKey* newStateKey = new StateKey(newCoverage, nextKenlmState);
  State* newState;
  std::map<StateKey, State*>::const_iterator findNewStateKey =
      lattice_[columnIndex].statesIndexByStateKey_.find(*newStateKey);
  if (findNewStateKey != lattice_[columnIndex].statesIndexByStateKey_.end()) {
    Cost existingCost = findNewStateKey->second->cost();
    // if the new cost is smaller, then we need to remove the state from the
    // set containing states sorted by cost then reinsert a new state so the
    // ordering is still correct.
    if (newCost < existingCost) {
      newState = new State(newStateKey, newCost,
                           findNewStateKey->second->incomingArcs());
      // TODO check that the number of states is equal
      lattice_[columnIndex].statesIndexByStateKey_[*newStateKey] = newState;
      lattice_[columnIndex].statesSortedByCost_.erase(findNewStateKey->second);
      lattice_[columnIndex].statesSortedByCost_.insert(newState);
    } else {
      newState = findNewStateKey->second;
    }
    newState->addArc(newArc);
  } else {
    std::vector<Arc> incomingArcs(1, newArc);
    newState = new State(newStateKey, newCost, incomingArcs);
    lattice_[columnIndex].statesSortedByCost_.insert(newState);
    lattice_[columnIndex].statesIndexByStateKey_[*newStateKey] = newState;
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

void Lattice::convert2openfstInit(const int length,
                                  std::stack<State*>* statesToBeProcessed,
                                  std::stack<StateId>* openfstStatesToBeProcessed,
                                  fst::StdVectorFst* res) const {
  res->AddState();
  res->SetStart(0);
  for (std::multiset<State*, StatePointerComparator>::const_iterator it =
      lattice_[length].statesSortedByCost_.begin();
      it != lattice_[length].statesSortedByCost_.end(); ++it) {
    statesToBeProcessed->push(*it);
    StateId stateId = res->AddState();
    openfstStatesToBeProcessed->push(stateId);
    // Add an epsilon transition from the initial state to the lattice final
    // states
    res->AddArc(0, fst::StdArc(0, 0, fst::StdArc::Weight::One(), stateId));
  }
}

void Lattice::convert2openfstProcess(std::stack<State*>* statesToBeProcessed,
                                     std::stack<StateId>* openfstStatesToBeProcessed,
                                     fst::StdVectorFst* res) const {
  State* stateToBeProcessed = statesToBeProcessed->top();
  statesToBeProcessed->pop();
  StateId openfstStateToBeProcessed = openfstStatesToBeProcessed->top();
  openfstStatesToBeProcessed->pop();
  const vector<Arc>& incomingArcs = stateToBeProcessed->incomingArcs();
  if (incomingArcs.empty()) {
    if (stateToBeProcessed->isInitial()) {
      res->SetFinal(openfstStateToBeProcessed, fst::StdArc::Weight::One());
    }
    return;
  }
  for (int i = 0; i < incomingArcs.size(); ++i) {
    statesToBeProcessed->push(const_cast<State*>(incomingArcs[i].state()));
    const vector<int>* words = incomingArcs[i].ngram();
    StateId previousStateId = openfstStateToBeProcessed;
    StateId nextStateId;
    int ngramLastIndex = words->size() - 1;
    for (int j = ngramLastIndex; j >= 0; j--) {
      // put the cost of the arc on the first openfst arc, then for the
      // remaining arcs, put a cost of one.
      const fst::StdArc::Weight arcCost =
          (j == ngramLastIndex) ? incomingArcs[i].cost() : fst::StdArc::Weight::One();
      nextStateId = res->AddState();
      res->AddArc(previousStateId,
                  fst::StdArc((*words)[j], (*words)[j], arcCost, nextStateId));
      previousStateId = nextStateId;
    }
    openfstStatesToBeProcessed->push(nextStateId);
  }
}

void Lattice::convert2openfstFinal(const fst::StdVectorFst& tempres1,
                                   fst::StdVectorFst* res) const {
  fst::StdVectorFst tempres2;
  fst::Reverse(tempres1, &tempres2);
  fst::RmEpsilon(&tempres2);
  fst::StdVectorFst tempres3;
  fst::Determinize(tempres2, res);
  fst::Minimize(res);
}

const string Lattice::printColumn(const int index) const {
  std::ostringstream res;
  res << "states in the map" << std::endl;
  for (std::map<StateKey, State*>::const_iterator it =
      lattice_[index].statesIndexByStateKey_.begin();
      it != lattice_[index].statesIndexByStateKey_.end(); ++it) {
    res << it->first.coverage() << std::endl;
    res << it->second->cost() << std::endl;
    res << "Arcs: " << std::endl;
    for (int i = 0; i < it->second->incomingArcs().size(); i++) {
      const vector<int>* words = it->second->incomingArcs()[i].ngram();
      for (int j = 0; j < words->size(); j++) {
        res << (*words)[j] << " " << std::endl;
      }
      res << std::endl;
    }
  }
  res << "states in the set" << std::endl;
  for (std::multiset<State*, StatePointerComparator>::const_iterator it =
      lattice_[index].statesSortedByCost_.begin();
      it != lattice_[index].statesSortedByCost_.end(); ++it) {
    res << (*it)->stateKey()->coverage() << std::endl;
    res << (*it)->cost() << std::endl;
    res << "Arcs: " << std::endl;
    for (int i = 0; i < (*it)->incomingArcs().size(); i++) {
      const vector<int>* words = (*it)->incomingArcs()[i].ngram();
      for (int j = 0; j < words->size(); j++) {
        res << (*words)[j] << " " << std::endl;
      }
      res << std::endl;
    }
  }
  return res.str();
}

} // namespace gen
} // namespace eng
} // namespace cam
