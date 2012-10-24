/*
 * StateTest.cpp
 *
 *  Created on: 22 Oct 2012
 *      Author: jmp84
 */

#include "Ngram.h"
#include "State.h"
#include "StateKey.h"
#include "gtest/gtest.h"

namespace {

TEST(overlapTest, simpleOverlapTest) {
  using namespace cam::eng::gen;
  Coverage coverage(5);
  coverage[0] = 1;
  coverage[2] = 1;
  lm::ngram::State kenlmState;
  StateKey stateKey(coverage, kenlmState);
  State state(&stateKey);
  Coverage coverage2(5);
  coverage2[1] = 0;
  Ngram ngram(coverage2);
  EXPECT_EQ(0, state.overlap(ngram));
}

} // namespace
