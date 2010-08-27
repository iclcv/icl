#include <gtest/gtest.h>

#include <ICLQuick/Common.h>
#include <RegionDetector2.h>

static const Img8u &create_image() {
  static const char *img[41] = {
    ".........................................",
    ".xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx.",
    ".xx................................xxxxx.",
    ".x.....xxxxxxxxxxxxxxxxxxxxxxxxxx....xx..",
    ".xx....x........................x....xx..",
    ".xxxx..xx...xxxxxxxx..xxxxxx....x....xx..",
    ".xx....x.....xxxxxx...xxxxxx....x....xx..",
    ".xxx...x.....xx...x...x....x....x....xx..",
    ".xxxx..x.....xxxxxx...xxxxxx....x....xx..",
    ".xx....x...............xxxxxx...x...xxx..",
    ".xx....xxxxxxxxxxxx............xx.....x..",
    ".xx........xxxxxxxxxxxxxxxxxxxxx.....xx..",
    "..xxxx..........xxxxxxxxxxxx.......xxx...",
    ".....xxxxx.......................xxxx....",
    ".........xxxxxxxxxxxxxxxxxxxxxxxxxx......",
    ".................xx.......xx.............",
    "..................xx.....xx..............",
    ".................xx......xx..............",
    ".................xxx....xxx..............",
    ".............xxxxxxxxxxxxxxxx............",
    "...........xxxx......xxxxx..xxxx.........",
    ".........xxx...xxxxxx.....xxxxxxxx.......",
    "........xx.....xxxxxx.....xxxxx.xxx......",
    ".......xxxxxxxx......xxxxx.....xxxxx.....",
    "......xxxxxxxxx......xxxxx.....xxxxxx....",
    "......xx.......xxxxxx.....xxxxx....xxxx..",
    ".....xxx.......xxxxxx.....xxxxx.....xxx..",
    ".....xxxxxxxxxx......xxxxx.....xxxxxxxx..",
    ".....xxxxxxxxxx......xxxxx.....xxxxxxxx..",
    "....xxxxx......xxxxxx.....xxxxx.....xxxx.",
    "....xxxxx......xxxxxx.....xxxxx.....xxxx.",
    "....xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx.",
    "....xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx.",
    ".....xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx..",
    ".....xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx..",
    "......xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx...",
    ".......xxxxxxxxxxxxxxxxxxxxxxxxxxxxx.....",
    "........xxxxxxxxxxxxxxxxxxxxxxxxxxxx.....",
    "..........xxxxxxxxxxxxxxxxxxxxxxxx.......",
    ".............xxxxxxxxxxxxxxxxxxxx........",
    "................xxxxxxxxxxxxx............",
  };

  static Img8u im(Size(41,41),1);
  bool first = true;
  if(first){
    for(int x=0;x<41;++x){
      for(int y=0;y<41;++y){
        im(x,y,0) = (img[y][x] == 'x')*255;
      }
    }
    first = false;
  }
  return im;
}


TEST(RegionDetector, Global_Region_Count ) {
  RegionDetector2 rd(true);
  const std::vector<ImageRegion> &rs = rd.detect(&create_image());

  ASSERT_EQ(rs.size(),30) << "number of detected regions is wrong";
  
}

TEST(RegionDetector, Is_Border_Flag) {
  RegionDetector2 rd(true);
  const std::vector<ImageRegion> &rs = rd.detect(&create_image());

  static const bool isBorder[30] = {1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
  for(int i=0;i<30;++i){
    if(isBorder[i]) {
      ASSERT_TRUE(rs[i].isBorderRegion()) << "is border flag of region " << i << " is wrong";
    }else{
      ASSERT_FALSE(rs[i].isBorderRegion()) << "is border flag of region " << i << " is wrong";
    }
  }
}

std::vector<int> ids(const std::vector<ImageRegion> &rs){
  std::vector<int> is(rs.size());
  std::transform(rs.begin(),rs.end(),is.begin(),std::mem_fun_ref(&ImageRegion::getID));
  return is;
}

TEST(RegionDetector, Region_Neighbours) {
  RegionDetector2 rd(true);
  const std::vector<ImageRegion> &rs = rd.detect(&create_image());
  
  
  std::map<int,std::vector<int> > desiredNeighbours;
  
#define SET_DESIRED_NEIGHBOURS(ID,LIST) desiredNeighbours[ID] = parseVecStr<int>(LIST);
  
  SET_DESIRED_NEIGHBOURS(0,"1");
  SET_DESIRED_NEIGHBOURS(1,"2,0,9,10,11,12,14,15,16,18,19,23,24,26,27,28,29");
  SET_DESIRED_NEIGHBOURS(2,"1,3");
  SET_DESIRED_NEIGHBOURS(3,"2,4");
  SET_DESIRED_NEIGHBOURS(4,"3,5,6");
  SET_DESIRED_NEIGHBOURS(5,"4,7");
  SET_DESIRED_NEIGHBOURS(6,"4,8");
  SET_DESIRED_NEIGHBOURS(7,"5");
  SET_DESIRED_NEIGHBOURS(8,"6");
  SET_DESIRED_NEIGHBOURS(9,"1");

  SET_DESIRED_NEIGHBOURS(10,"1,13");
  SET_DESIRED_NEIGHBOURS(11,"1");
  SET_DESIRED_NEIGHBOURS(12,"1,13");
  SET_DESIRED_NEIGHBOURS(13,"10,12,14,16");
  SET_DESIRED_NEIGHBOURS(14,"1,13,17 ");
  SET_DESIRED_NEIGHBOURS(15,"1");
  SET_DESIRED_NEIGHBOURS(16,"1,13,17,20");
  SET_DESIRED_NEIGHBOURS(17,"14,16,18,21");
  SET_DESIRED_NEIGHBOURS(18,"1,17,22");
  SET_DESIRED_NEIGHBOURS(19,"1,20");
  
  SET_DESIRED_NEIGHBOURS(20,"16,19,21,24");
  SET_DESIRED_NEIGHBOURS(21,"17,20,22,25");
  SET_DESIRED_NEIGHBOURS(22,"18,21,23,26");
  SET_DESIRED_NEIGHBOURS(23,"1,22");
  SET_DESIRED_NEIGHBOURS(24,"1,20,25");
  SET_DESIRED_NEIGHBOURS(25,"21,24,26,28");
  SET_DESIRED_NEIGHBOURS(26,"1,22,25");
  SET_DESIRED_NEIGHBOURS(27,"1");
  SET_DESIRED_NEIGHBOURS(28,"1,25");
  SET_DESIRED_NEIGHBOURS(29,"1");

  for(unsigned int i=0;i<rs.size();++i){
    std::vector<int> desired = desiredNeighbours[rs[i].getID()];
    std::vector<int> detected = ids(rs[i].getNeighbours());

    ASSERT_EQ(desired.size(),detected.size()) << " neighbour count of "
                                              << " region " << i 
                                              << " is wrong";

    std::sort(desired.begin(),desired.end());
    std::sort(detected.begin(),detected.end());
    
    std::vector<int> tmp;
    std::set_union(desired.begin(),desired.end(),detected.begin(),detected.end(),
                   std::back_inserter(tmp));
    ASSERT_EQ(tmp.size(),desired.size()) << " at least one neighbour for "
                                         << " region " << i << " is wrong";
    
    tmp.clear();
    std::set_difference(desired.begin(),desired.end(),detected.begin(),detected.end(),
                        std::back_inserter(tmp));
    
    ASSERT_EQ(tmp.size(),0) << " at least one neighbour for "
                            << " region " << i << " is wrong";
  }
}


TEST(RegionDetector, Region_Sizes_And_Boundaries) {
  RegionDetector2 rd(true);
  const std::vector<ImageRegion> &rs = rd.detect(&create_image());

  int sizes[] = {485,598,145,86,99,23,26,3,4,22,6,2,8,12,10,1,12,10,10,14,12,10,10,9,12,10,10,12,10,10};
  int boundaryPixelCountsThinned[] = {294,159,83,61,53,16,16,3,5,14,9,2,7,10,8,1,10,8,8,12,10,8,8,7,10,8,8,10,8,8};
  int boundaryPixelCountsNormal[] = {346,208,102,68,60,20,20,4,6,20,10,2,10,12,10,1,12,10,10,14,12,10,10,10,12,10,10,12,10,10};
 
  for(unsigned int i=0;i<rs.size();++i){
    ASSERT_EQ(rs[i].getSize(),sizes[rs[i].getID()]) << "region size of region " << i << " is wrong";
    ASSERT_EQ(rs[i].getBoundaryPointCount(false), boundaryPixelCountsNormal[rs[i].getID()]) << "unthinned boundary point cout "
                                                                                            << "of region " << i << " is wrong";
    ASSERT_EQ(rs[i].getBoundaryPointCount(true), boundaryPixelCountsThinned[rs[i].getID()]) << "unthinned boundary point cout "
                                                                                            << "of region " << i << " is wrong";
  }
}

TEST(RegionDetector, Sub_Regions) {
  RegionDetector2 rd(true);
  const std::vector<ImageRegion> &rs = rd.detect(&create_image());

  std::map<int,std::vector<int> > desiredSubRegions;
  
#define SET_DESIRED_SUB_REGIONS(ID,LIST) desiredSubRegions[ID] = parseVecStr<int>(LIST)
  SET_DESIRED_SUB_REGIONS(1,"2,9,10,11,12,14,15,16,18,19,23,24,26,27,28,29");
  SET_DESIRED_SUB_REGIONS(2,"3");
  SET_DESIRED_SUB_REGIONS(3,"4");
  SET_DESIRED_SUB_REGIONS(4,"5,6");
  SET_DESIRED_SUB_REGIONS(5,"7");
  SET_DESIRED_SUB_REGIONS(6,"8");

  for(unsigned int i=0;i<rs.size();++i){
    std::vector<int> desired = desiredSubRegions[rs[i].getID()];
    std::vector<int> detected = ids(rs[i].getSubRegions());

    ASSERT_EQ(desired.size(),detected.size()) << " sub-region count of "
                                              << " region " << i 
                                              << " is wrong";

    std::sort(desired.begin(),desired.end());
    std::sort(detected.begin(),detected.end());
    
    std::vector<int> tmp;
    std::set_union(desired.begin(),desired.end(),detected.begin(),detected.end(),
                   std::back_inserter(tmp));
    ASSERT_EQ(tmp.size(),desired.size()) << " at least one sub-region for "
                                         << " region " << i << " is wrong";
    
    tmp.clear();
    std::set_difference(desired.begin(),desired.end(),detected.begin(),detected.end(),
                        std::back_inserter(tmp));
    
    ASSERT_EQ(tmp.size(),0) << " at least one sub-region for "
                            << " region " << i << " is wrong";
  }

}


TEST(RegionDetector, Recursive_Sub_Regions) {
  RegionDetector2 rd(true);
  const std::vector<ImageRegion> &rs = rd.detect(&create_image());

  std::map<int,std::vector<int> > desiredRecSubRegions;
  
#define SET_DESIRED_REC_SUB_REGIONS(ID,LIST) desiredRecSubRegions[ID] = parseVecStr<int>(LIST)
  SET_DESIRED_REC_SUB_REGIONS(1,"2,9,10,11,12,14,15,16,18,19,23,24,26,27,28,29,3,4,5,6,7,8");
  SET_DESIRED_REC_SUB_REGIONS(2,"3,4,5,6,7,8");
  SET_DESIRED_REC_SUB_REGIONS(3,"4,5,6,7,8");
  SET_DESIRED_REC_SUB_REGIONS(4,"5,6,7,8");
  SET_DESIRED_REC_SUB_REGIONS(5,"7");
  SET_DESIRED_REC_SUB_REGIONS(6,"8");

  for(unsigned int i=0;i<rs.size();++i){
    std::vector<int> desired = desiredRecSubRegions[rs[i].getID()];
    std::vector<int> detected = ids(rs[i].getSubRegions(false));
    
    ASSERT_EQ(desired.size(),detected.size()) << " sub-region count of "
                                              << " region " << i 
                                              << " is wrong";

    std::sort(desired.begin(),desired.end());
    std::sort(detected.begin(),detected.end());
    
    std::vector<int> tmp;
    std::set_union(desired.begin(),desired.end(),detected.begin(),detected.end(),
                   std::back_inserter(tmp));
    ASSERT_EQ(tmp.size(),desired.size()) << " at least one recursive sub-region for "
                                         << " region " << i << " is wrong";
    
    tmp.clear();
    std::set_difference(desired.begin(),desired.end(),detected.begin(),detected.end(),
                        std::back_inserter(tmp));
    
    ASSERT_EQ(tmp.size(),0) << " at least one recursiv sub-region for "
                            << " region " << i << " is wrong";
  }

}


TEST(RegionDetector, Region_Centers_Of_Gravity) {
  RegionDetector2 rd(true);
  const std::vector<ImageRegion> &rs = rd.detect(&create_image());
 
  Point32f cogs[] = {Point32f(18.2577,21.2845),Point32f(21.1421,25.597),Point32f(19.6759,7.56552),
                     Point32f(19.314,7.86047),Point32f(19.697,6.81818),Point32f(15.4348,6.30435),
                     Point32f(24.7308,7),Point32f(16,7),Point32f(24.5,7),Point32f(21.7727,16.3182),
                     Point32f(17.5,20),Point32f(26.5,20),Point32f(12.375,21.625),Point32f(17.5,21.5),
                     Point32f(23,21.5),Point32f(31,22),Point32f(17.5,23.5),Point32f(23,23.5),
                     Point32f(28,23.5),Point32f(11,25.5),Point32f(17.5,25.5),Point32f(23,25.5),
                     Point32f(28,25.5),Point32f(32.7778,25.5556),Point32f(17.5,27.5),Point32f(23,27.5),
                     Point32f(28,27.5),Point32f(11.5,29.5),Point32f(23,29.5),Point32f(33,29.5)};
  
  for(unsigned int i=0;i<rs.size();++i){
    const Point32f c = rs[i].getCOG(); 
    ASSERT_NEAR(c.x,cogs[rs[i].getID()].x,0.01) << " x-coordinate of region " << i << " is wrong";
    ASSERT_NEAR(c.y,cogs[rs[i].getID()].y,0.01) << " y-coordinate of region " << i << " is wrong";
  }
}

TEST(RegionDetector, Value_Constraints) {
  RegionDetector2 rd(0,2<<28,0,0);
  const std::vector<ImageRegion> *rs = &rd.detect(&create_image());
  
  ASSERT_EQ(rs->size(),21) << "value contraint for filtering regions does not work (used value range [0,0])";

  rd.setConstraints(0,2<<28,128,255);
  rs = &rd.detect(&create_image());
  
  ASSERT_EQ(rs->size(),9) << "value contraint for filtering regions does not work (used value range [128,255])";
}

TEST(RegionDetector, Size_Constraints) {
  RegionDetector2 rd(0,98,0,255);
  const std::vector<ImageRegion> *rs = &rd.detect(&create_image());
  
  ASSERT_EQ(rs->size(),26) << "size contraint for filtering regions does not work (used size range [0,98])";

  rd.setConstraints(0,99,0,255);
  rs = &rd.detect(&create_image());
  
  ASSERT_EQ(rs->size(),27) << "value contraint for filtering regions does not work (used value range [0,99])";

  
  rd.setConstraints(1,10,0,255);
  rs = &rd.detect(&create_image());
  
  ASSERT_EQ(rs->size(),16) << "value contraint for filtering regions does not work (used value range [1,16])";
}

TEST(RegionDetector, Combined_Constraints) {
  RegionDetector2 rd(1,10,128,255);
  const std::vector<ImageRegion> *rs = &rd.detect(&create_image());
  
  ASSERT_EQ(rs->size(),3) << "size contraint for filtering regions"
                          << " does not work (used size range [1,10] "
                          << "and value range [128,255])";

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
