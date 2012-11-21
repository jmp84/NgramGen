/*
 * ColumnTest.cpp
 *
 *  Created on: 21 Nov 2012
 *      Author: jmp84
 */

#include <gtest/gtest.h>
#include "Column.h"
#include "StateKey.h"

TEST(emptyTest, Vanilla) {
  cam::eng::gen::Column c;
  EXPECT_TRUE(c.empty());
}
