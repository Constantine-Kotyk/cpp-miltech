#include <gtest/gtest.h>
#include <cstring>
#include "ballistics.hpp"

TEST(BallisticsTest, ReadDataSuccess) {
    DroneParams dP;
    char path[] = TEST_DATA_DIR "/good.txt";
    EXPECT_EQ(read_data(path, dP), SUCCESS);
}

TEST(BallisticsTest, ReadEmptyFile) {
    DroneParams dP;
    char path[] = TEST_DATA_DIR "/empty.txt";
    EXPECT_EQ(read_data(path, dP), FILE_FIELD_COUNT_MISMATCH);
}

TEST(BallisticsTest, ReadDataParseFailure) {
    DroneParams dP;
    char path[] = TEST_DATA_DIR "/bad.txt";
    EXPECT_EQ(read_data(path, dP), FILE_FIELD_PARSE_FAILED);
}

TEST(BallisticsTest, UnknownAmmo) {
    DroneParams dP = {0, 0, 0, 10, 10, 100, 0, ""};
    Solution solution{};

    float m, d, l;
    const bool result = getAmmoParams("unknown", m, d, l);

    ASSERT_FALSE(result);

    EXPECT_EQ(calcBallistics(dP, solution), UNKNOWN_AMMO);
}

TEST(BallisticsTest, CalcKnownDropPoint) {
    DroneParams dP = {
        .xd = 100.0,
        .yd = 100.0,
        .zd = 100.0,
        .targetX = 200.0,
        .targetY = 200.0,
        .attackSpeed = 10.0,
        .accelerationPath = 10.0,
        .ammo_name = "VOG-17",
    };
    Solution solution;
    calcBallistics(dP, solution);
    EXPECT_NEAR(solution.fireX, 173.759, 0.01);
    EXPECT_NEAR(solution.fireY, 173.759, 0.01);
}