//
// Created by skyitachi on 2019-05-20.
//

#include <gtest/gtest.h>

int factorial(int a) {
  if (!a) return 1;
  return factorial(a - 1) * a;
}

TEST(FactorialTest, Case1) {
  EXPECT_EQ(factorial(0), 2);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
