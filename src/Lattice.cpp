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
#include <lm/state.hh>

#include "Column.h"
#include "Ngram.h"
#include "NgramLoader.h"
#include "State.h"
#include "Types.h"

namespace cam {
namespace eng {
namespace gen {

void Lattice::print() {
  for (int i = 0; i < lattice_.size(); i++) {
    std::cerr << "COLUMN " << i << std::endl;
    printColumn(i);
  }
}

void Lattice::printColumn(int index) {
  std::cerr << "states in the map" << std::endl;
  for (std::map<StateKey, State*>::iterator it =
      lattice_[index].statesIndexByStateKey_.begin();
      it != lattice_[index].statesIndexByStateKey_.end(); ++it) {
    std::cerr << it->first.coverage_ << std::endl;
    //std::cerr << it->first.kenlmState_ << std::endl;
    std::cerr << it->second->cost_ << std::endl;
    std::cerr << "Arcs: " << std::endl;
    for (int i = 0; i < it->second->incomingArcs_.size(); i++) {
      const vector<int>* words = it->second->incomingArcs_[i].ngram();
      for (int j = 0; j < words->size(); j++) {
        std::cerr << (*words)[j] << " " << std::endl;
      }
      std::cerr << std::endl;
    }
  }
  std::cerr << "states in the set" << std::endl;
  for (std::multiset<State*, StatePointerComparator>::const_iterator it =
      lattice_[index].statesSortedByCost_.begin();
      it != lattice_[index].statesSortedByCost_.end(); ++it) {
    std::cerr << (*it)->stateKey_->coverage_ << std::endl;
    //std::cerr << it->first.kenlmState_ << std::endl;
    std::cerr << (*it)->cost_ << std::endl;
    std::cerr << "Arcs: " << std::endl;
    for (int i = 0; i < (*it)->incomingArcs_.size(); i++) {
      const vector<int>* words = (*it)->incomingArcs_[i].ngram();
      for (int j = 0; j < words->size(); j++) {
        std::cerr << (*words)[j] << " " << std::endl;
      }
      std::cerr << std::endl;
    }
  }
}

void Lattice::init(const std::vector<int>& words, const std::string& lmfile) {
  lattice_.resize(words.size() + 1);
  languageModel_ = new lm::ngram::Model(lmfile.c_str());
  Coverage emptyCoverage(words.size());
  lm::ngram::State beginSentenceState(languageModel_->BeginSentenceState());
  StateKey* initStateKey = new StateKey(emptyCoverage, beginSentenceState);
  State* initState = new State(initStateKey, 0, std::vector<Arc>());
  lattice_[0].statesIndexByStateKey_[*initStateKey] = initState;
  lattice_[0].statesSortedByCost_.insert(initState);
}

Cost Lattice::cost(const State& state, const vector<int>& ngram,
                   lm::ngram::State* nextKenlmState) const {
  Cost res = 0;
  lm::ngram::State startKenlmState = state.getKenlmState();
  const lm::ngram::Vocabulary& vocab = languageModel_->GetVocabulary();
  for (int i = 0; i < ngram.size(); i++) {
    std::string word;
    if (ngram[i] == 2) {
      word = "</s>";
    } else if (ngram[i] == 1) {
      word = "<s>";
    } else {
      word = boost::lexical_cast<std::string>(ngram[i]);
    }
    res = languageModel_->Score(startKenlmState, vocab.Index(word),
                                *nextKenlmState);
    startKenlmState = *nextKenlmState;
  }
  return res;
}

void Lattice::extend(const NgramLoader& ngramLoader, int columnIndex) {
  const std::map<std::vector<int>, std::vector<Coverage> >& ngrams =
      ngramLoader.ngrams();
  for (std::multiset<State*, StatePointerComparator>::const_iterator stateIt =
      lattice_[columnIndex].statesSortedByCost_.begin();
      stateIt != lattice_[columnIndex].statesSortedByCost_.end(); ++stateIt) {
    for (std::map<std::vector<int>, std::vector<Coverage> >::const_iterator ngramIt =
        ngrams.begin(); ngramIt != ngrams.end(); ++ngramIt) {
      for (int i = 0; i < ngramIt->second.size(); i++) {
        if ((*stateIt)->overlap(ngramIt->second[i]) == 0) {
          extend(**stateIt, ngramIt->first, ngramIt->second[i]);
          // we break to use only the first coverage of the ngram to avoid
          // spurious ambiguity
          break;
        }
      }
    }
  }
}

void Lattice::extend(const State& state, const vector<int>& ngram,
                     const Coverage& coverage) {
  Coverage newCoverage = state.coverage() | coverage;
  int columnIndex = newCoverage.count();
  lm::ngram::State nextKenlmState;
  Cost newCost = cost(state, ngram, &nextKenlmState);
  Cost diffCost = newCost - state.cost();
  Arc newArc(const_cast<State*>(&state), const_cast<std::vector<int>*>(&ngram),
             diffCost);
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

void Lattice::prune(int columnIndex, int nbest) {
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

void Lattice::prune(int columnIndex, Cost threshold) {
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

void Lattice::convert2openfst(int length, fst::StdVectorFst* res) {
  //this->print();
  typedef fst::StdArc::StateId StateId;
  // if there is no state in the last column, then failure, return a null fst.
  if (lattice_[length].empty()) {
    delete res;
    res = NULL;
    return;
  }
  res->DeleteStates();
  fst::StdVectorFst tempres1;
  tempres1.AddState();
  tempres1.SetStart(0);
  std::stack<State*> statesToBeProcessed;
  std::stack<StateId> openfstStatesToBeProcessed;
  for (std::multiset<State*, StatePointerComparator>::const_iterator it =
      lattice_[length].statesSortedByCost_.begin();
      it != lattice_[length].statesSortedByCost_.end(); ++it) {
    statesToBeProcessed.push(*it);
    StateId stateId = tempres1.AddState();
    openfstStatesToBeProcessed.push(stateId);
    // Add an epsilon transition from the initial state to the lattice final
    // states
    tempres1.AddArc(0, fst::StdArc(0, 0, fst::StdArc::Weight::One(), stateId));
  }
  while (!statesToBeProcessed.empty()) {
    State* stateToBeProcessed = statesToBeProcessed.top();
    statesToBeProcessed.pop();
    StateId openfstStateToBeProcessed = openfstStatesToBeProcessed.top();
    openfstStatesToBeProcessed.pop();
    const vector<Arc>& incomingArcs = stateToBeProcessed->incomingArcs();
    if (incomingArcs.empty()) {
      tempres1.SetFinal(openfstStateToBeProcessed, fst::StdArc::Weight::One());
    }
    for (int i = 0; i < incomingArcs.size(); i++) {
      statesToBeProcessed.push(const_cast<State*>(incomingArcs[i].state()));
      const vector<int>* words = incomingArcs[i].ngram();
      StateId previousStateId = openfstStateToBeProcessed;
      StateId nextStateId;
      int ngramLastIndex = words->size() - 1;
      for (int j = ngramLastIndex; j >= 0; j--) {
        // put the cost of the arc on the first openfst arc, then for the
        // remaining arc, put a cost of one.
        const fst::StdArc::Weight arcCost =
            (j == ngramLastIndex) ? incomingArcs[i].cost() : fst::StdArc::Weight::One();
        nextStateId = tempres1.AddState();
        tempres1.AddArc(previousStateId,
                        fst::StdArc((*words)[j], (*words)[j], arcCost, nextStateId));
        previousStateId = nextStateId;
      }
      openfstStatesToBeProcessed.push(nextStateId);
    }
  }
  tempres1.Write("tempres1.fst");
  fst::StdVectorFst tempres2;
  fst::Reverse(tempres1, &tempres2);
  fst::RmEpsilon(&tempres2);
  fst::StdVectorFst tempres3;
  fst::Determinize(tempres2, res);
  fst::Minimize(res);
}

} // namespace gen
} // namespace eng
} // namespace cam
