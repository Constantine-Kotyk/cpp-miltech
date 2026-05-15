#include <gtest/gtest.h>
#include <cstring>
#include "ballistics.hpp"

TEST(BallisticsTest, ReadDataSuccess)
{
  DroneParams dParams;
  char path[] = TEST_DATA_DIR "/good.txt";
  EXPECT_EQ(read_data(path, dParams), kSuccess);
}

TEST(BallisticsTest, ReadEmptyFile)
{
  DroneParams dParams;
  char path[] = TEST_DATA_DIR "/empty.txt";
  EXPECT_EQ(read_data(path, dParams), kFileFieldCountMismatch);
}

TEST(BallisticsTest, ReadDataParseFailure)
{
  DroneParams dParams;
  char path[] = TEST_DATA_DIR "/bad.txt";
  EXPECT_EQ(read_data(path, dParams), kFileFieldParseFailed);
}

TEST(BallisticsTest, UnknownAmmo)
{
  DroneParams dParams = {0, 0, 0, 10, 10, 100, 0, ""};
  Solution solution{};

  float m, d, l;
  const bool result = get_ammo_params("unknown", m, d, l);

  ASSERT_FALSE(result);

  EXPECT_EQ(calc_ballistics(dParams, solution), kUnknownAmmo);
}

TEST(BallisticsTest, CalcKnownDropPoint)
{
  DroneParams dParams = {
    .x_ = 100.0,
    .y_ = 100.0,
    .z_ = 100.0,
    .target_x_ = 200.0,
    .target_y_ = 200.0,
    .attack_speed_ = 10.0,
    .acceleration_path_ = 10.0,
    .ammo_name_ = "VOG-17",
  };
  Solution solution;
  calc_ballistics(dParams, solution);
  EXPECT_NEAR(solution.fire_x_, 173.759, 0.01);
  EXPECT_NEAR(solution.fire_y_, 173.759, 0.01);
}