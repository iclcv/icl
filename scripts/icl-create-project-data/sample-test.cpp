#include <gtest/gtest.h>

TEST(TestCaseName, TestName) {
  FAIL();
}

/*******************************************************************************
Dear programmer,
thanks for taking a look at this file. Above you find the simplest test you can
write using the Google C++ Testing Framework (GTest). You may run it by invoking
'make test' on the shell in the main project directory. When doing so, you will
see the test fails -- which is calling for action of course. But first let's
have a look at...


== The Philosophy of Test Driven Development (TDD) ==

TDD is a software development technique with very short development cycles, that
build onto each other iteratively. In each cycle you do:
  1) write an automated test for a new feature
  2) run all tests and see if the new one fails
  3) write some code to make the tests pass
  4) refactor the code & rerun the tests

So why would you want to do this? Well, amoung the many advantages, these are
some of the most striking ones:
• you will detect more bugs earlier
• you will have more confidence in your code
• it gives you the courage to refactor code, because you can actually check
  the code still works after changing it
• it will help you write modular code and focus on the core requirements

So that is that. What you could do now is changing the above test to make it
test an actual feature of the software you have / want to write and then write
and modify your code until it succeeds. Any .cpp files in the test directory
will be automatically included when calling 'make test'. The remaining part of
the text deals with how to write tests in GTest. More information about TDD can
easily be found online.


== Google C++ Testing Framework ==

Terminology:
• assertion (has a result: success / nonfatal failure / fatal failure)
• test (has-many assertions, fails if crashes or one assertion fails)
• test case (has-many tests, for logical structuring)
• test fixture (class for sharing common objects and subroutines amoung several tests)
• test program (has-many tests and test cases)

Assertions:
• use ASSERT_* for fatal, EXPECT_* for nonfatal failures
• try using EXPECT_* at all places it makes sense
• simply '<<' custom messages in the assertion macros
• typical assertions:
  ||        Assertion             |||     Verifies              ||
  ----------------------------------------------------------------
  || SUCCEED();                    | always succeeds            ||
  || FAIL();                       | always fails               ||
  || ASSERT_TRUE(condition);       | condition is true          ||
  || ASSERT_FALSE(condition);      | condition is false         ||
  ----------------------------------------------------------------
  || ASSERT_EQ(expected, actual);  | expected == actual         ||
  || ASSERT_NE(val1, val2);        | val1 != val2               ||
  || ASSERT_LT(val1, val2);        | val1 < val2                ||
  || ASSERT_LE(val1, val2);        | val1 <= val2               ||
  || ASSERT_GT(val1, val2);        | val1 > val2                ||
  || ASSERT_GE(val1, val2);        | val1 >= val2               ||
  -----------------------------------------------------------------------------------
  || ASSERT_FLOAT_EQ(expected, actual);  | the two float values are almost equal   ||
  || ASSERT_DOUBLE_EQ(expected, actual); | the two double values are almost equal  ||
  || ASSERT_NEAR(val1, val2, abs_error); | the difference between val1 and val2    ||
  ||                                     | doesn't exceed the given absolute error ||
  ------------------------------------------------------------------------------------------------
  || ASSERT_THROW(statement, exception_type); | statement throws an exception of the given type ||
  || ASSERT_ANY_THROW(statement);             | statement throws an exception of any type       ||
  || ASSERT_NO_THROW(statement);              | statement doesn't throw any exception           ||
  ------------------------------------------------------------------------------------------------
  || ASSERT_PRED1(pred1, val1);       | pred1(val1) returns true       ||
  || ASSERT_PRED2(pred2, val1, val2); | pred2(val1, val2) returns true ||
  || ...                              | ...                            ||
  (These last ones can be used if a boolean function is available. GTest will
   print the function's arguments when the assertion fails.)
• all the assertion macros above also come in the EXPECT_* flavour

Tests:
• TEST(test_case_name, test_name) {
    # any commands, assertions
  } 

Fixtures:
• when several test operate on similar data, use fixtures
• these are child classes of ::testing::Test, with SetUp() and TearDown() methods
• they are instantiated freshly for each test
• TEST_F(fixture_class_name, test_name)
  {
    # any commands, assertions
  }
  
(see http://code.google.com/p/googletest/wiki/GoogleTestPrimer for more
information)

*******************************************************************************/
