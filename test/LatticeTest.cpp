/*
 * ColumnTest.cpp
 *
 *  Created on: 21 Nov 2012
 *      Author: jmp84
 */

#include <gtest/gtest.h>

#include <boost/lexical_cast.hpp>

#include "Lattice.h"
#include "StateKey.h"
#include "State.h"

namespace cam {
namespace eng {
namespace gen {

class LatticeTest : public ::testing::Test {
protected:
  virtual void SetUp() {
    vector<int> input;
    input.push_back(7);
    for (int i = 5; i <= 10; ++i) {
      input.push_back(i);
    }
    lattice_.reset(new Lattice(input, "test/lm.4.gz"));
    Coverage coverage(std::string("1111000"));
    lm::ngram::Model lm("test/lm.4.gz");
    lm::ngram::State lmState(lm.BeginSentenceState());
    const lm::ngram::Vocabulary& vocab = lm.GetVocabulary();
    lm::ngram::State nextLmState;
    for (int i = 5; i <= 7; ++i) {
      lm.Score(lmState, vocab.Index(boost::lexical_cast<std::string>(i)),
               nextLmState);
      lmState = nextLmState;
    }
    StateKey* stateKey = new StateKey(coverage, nextLmState);
    fst::StdArc::StateId stateId = 0;
    state_.reset(new State(stateId, stateKey, 0));
  }

  bool compatibleHistory(const State& state, const Ngram& ngram,
                         const Coverage& overlap,
                         const int overlapCount) const {
    return lattice_->compatibleHistory(state, ngram, overlap, overlapCount);
  }

  bool canApply(const State& state, const Ngram& ngram,
                const Coverage& coverage, Ngram* ngramToApply,
                const int maxOverlap) const {
    return lattice_->canApply(state, ngram, coverage, ngramToApply, maxOverlap);
  }

  boost::scoped_ptr<Lattice> lattice_;
  boost::scoped_ptr<State> state_;

};

TEST_F(LatticeTest, compatibleHistoryCompatible) {
  Ngram ngram;
  ngram.push_back(7);
  ngram.push_back(8);
  Coverage ngramCoverage(std::string("0001100"));
  Coverage overlap = state_->coverage() & ngramCoverage;
  EXPECT_EQ(1, overlap.count());
  EXPECT_TRUE(compatibleHistory(*state_, ngram, overlap, 1));
}

TEST_F(LatticeTest, compatibleHistoryIncompatible) {
  Ngram ngram;
  ngram.push_back(8);
  ngram.push_back(7);
  Coverage ngramCoverage(std::string("0001100"));
  Coverage overlap = state_->coverage() & ngramCoverage;
  EXPECT_FALSE(compatibleHistory(*state_, ngram, overlap, 1));
}

TEST_F(LatticeTest, compatibleHistoryCompatible2) {
  Ngram ngram;
  ngram.push_back(7);
  ngram.push_back(8);
  Coverage ngramCoverage(std::string("1000100"));
  Coverage overlap = state_->coverage() & ngramCoverage;
  EXPECT_TRUE(compatibleHistory(*state_, ngram, overlap, 1));
}

TEST_F(LatticeTest, canApplyOverlapGreaterThanMaxOverlap) {
  Ngram ngram;
  ngram.push_back(7);
  ngram.push_back(8);
  Coverage ngramCoverage(std::string("0001100"));
  Ngram ngramToApply;
  EXPECT_FALSE(canApply(*state_, ngram, ngramCoverage, &ngramToApply, 0));
}

TEST_F(LatticeTest, canApplyOverlapGreaterThanHistory) {
  Ngram ngram;
  ngram.push_back(7);
  for (int i = 5; i <= 7; ++i) {
    ngram.push_back(i);
  }
  Coverage ngramCoverage(std::string("1111000"));
  Ngram ngramToApply;
  EXPECT_FALSE(canApply(*state_, ngram, ngramCoverage, &ngramToApply, 0));
}

TEST_F(LatticeTest, canApplyOverlapIncludedInStateCoverage) {
  Ngram ngram;
  ngram.push_back(7);
  Coverage ngramCoverage(std::string("0001000"));
  Ngram ngramToApply;
  EXPECT_FALSE(canApply(*state_, ngram, ngramCoverage, &ngramToApply, 0));
}

TEST_F(LatticeTest, canApplyStartOfSentence) {
  Ngram ngram;
  ngram.push_back(1);
  ngram.push_back(8);
  Coverage ngramCoverage(std::string("0000110"));
  Ngram ngramToApply;
  EXPECT_FALSE(canApply(*state_, ngram, ngramCoverage, &ngramToApply, 0));
}

TEST_F(LatticeTest, canApplyEndOfSentence) {
  Ngram ngram;
  ngram.push_back(8);
  ngram.push_back(2);
  Coverage ngramCoverage(std::string("0000110"));
  Ngram ngramToApply;
  EXPECT_FALSE(canApply(*state_, ngram, ngramCoverage, &ngramToApply, 0));
}

TEST_F(LatticeTest, canApplyNgramToApply) {
  Ngram ngram;
  ngram.push_back(7);
  ngram.push_back(8);
  Coverage ngramCoverage(std::string("0001100"));
  Ngram ngramToApply;
  EXPECT_TRUE(canApply(*state_, ngram, ngramCoverage, &ngramToApply, 1));
  Ngram expectedNgram(1,8);
  EXPECT_EQ(expectedNgram, ngramToApply);
}

} // namespace gen
} // namespace eng
} // namespace cam
