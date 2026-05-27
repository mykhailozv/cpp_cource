#include "gtest/gtest.h"

#include "ballistics.hpp"

const double EPS = 1e-3;

TEST(Ballistics, СomputeHorizontalRange)
{
  double height = 100.0;
  double attack_speed = 10.0;
  double drag = 0.07;
  double lift = 0.0;
  double mass = 0.35;
  double horizontal_ammo_range;
  const double expected_horizontal_ammo_range = 37.11022211502511;
  const bool expected_is_computed = true;
  bool is_computed = compute_horizontal_range(attack_speed, height, drag, lift, mass, horizontal_ammo_range);

  EXPECT_EQ(is_computed, expected_is_computed);

  EXPECT_NEAR(horizontal_ammo_range, expected_horizontal_ammo_range, EPS);
}

TEST(Ballistics, ComputeAll)
{
  std::string path = std::string(TEST_SOURCE_DIR) + "/data/input.txt";

  double fire_x, fire_y, maneuver_x, maneuver_y;
  bool need_maneuver;
  bool is_computed = calculate_fire_point(path.c_str(), fire_x, fire_y, need_maneuver, maneuver_x, maneuver_y);

  const double expected_fire_x = 195.088;
  const double expected_fire_y = 151.572;
  const double expected_maneuver_x = 193.567;
  const double expected_maneuver_y = 141.689;

  EXPECT_TRUE(is_computed);
  EXPECT_NEAR(fire_x, expected_fire_x, EPS);
  EXPECT_NEAR(fire_y, expected_fire_y, EPS);
  EXPECT_NEAR(maneuver_x, expected_maneuver_x, EPS);
  EXPECT_NEAR(maneuver_y, expected_maneuver_y, EPS);
  EXPECT_TRUE(need_maneuver);
}

TEST(Ballistics, UnknownAmmo)
{
  double drag, lift, mass;

  EXPECT_FALSE(get_ammo_parameters("INVALID", drag, lift, mass));
}

TEST(Ballistics, InvalidInputFileReturnsFalse)
{
  double xd, yd, zd, target_x, target_y, attack_speed, acceleration_path;
  std::string ammo_name;
  std::string path = std::string(TEST_SOURCE_DIR) + "/data/broken_input.txt";

  EXPECT_FALSE(read_input_file(path.c_str(), xd, yd, zd, target_x, target_y, attack_speed, acceleration_path, ammo_name));
}

TEST(Ballistics, MissingInputFileReturnsFalse)
{
  double xd, yd, zd, target_x, target_y, attack_speed, acceleration_path;
  std::string ammo_name;

  EXPECT_FALSE(read_input_file("some_thing", xd, yd, zd, target_x, target_y, attack_speed, acceleration_path, ammo_name));
}

TEST(Ballistics, ValidAmmoReturnsTrue)
{
  double drag, lift, mass;

  EXPECT_TRUE(get_ammo_parameters("VOG-17", drag, lift, mass));
}

TEST(Ballistics, ValidInputFileReturnsTrue)
{
  double xd, yd, zd, target_x, target_y, attack_speed, acceleration_path;
  std::string ammo_name;
  std::string path = std::string(TEST_SOURCE_DIR) + "/data/input.txt";

  EXPECT_TRUE(read_input_file(path.c_str(), xd, yd, zd, target_x, target_y, attack_speed, acceleration_path, ammo_name));
}