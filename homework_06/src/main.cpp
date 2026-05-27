#include <iostream>

#include "ballistics.hpp"

int main(int argc, char** argv)
{
  std::cout << "Hello World!!!" << std::endl;
  std::cout << "argc: " << argc << std::endl;
  std::cout << "argv[0]: " << argv[0] << "\n\n";

  std::string path = "homework_06/data/input.txt";
  std::string output_path = "homework_06/data/output.txt";

  double fire_x, fire_y, maneuver_x, maneuver_y;
  bool need_maneuver;
  bool is_computed = calculate_fire_point(path.c_str(), fire_x, fire_y, need_maneuver, maneuver_x, maneuver_y);

  if (!is_computed) {
    return 1;
  }

  if (!write_result_file(output_path.c_str(), fire_x, fire_y, need_maneuver, maneuver_x, maneuver_y)) {
    return 1;
  }

  if (need_maneuver) {
    std::cout << "maneuver point\n";
    std::cout << maneuver_x << " " << maneuver_y << std::endl;
  }

  std::cout << "fire point\n";
  std::cout << fire_x << " " << fire_y << std::endl;

  return 0;
}