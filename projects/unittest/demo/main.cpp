#include "gtest/gtest.h"

int main(int argc, char** argv)
{
//#error xx
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
