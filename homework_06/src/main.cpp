#include <iostream>

#include "ballistics.hpp"

int main(int argc, char** argv)
{
  std::cout << "Hello World!!!\n";

  std::string path;

  if (argc > 1) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    path = argv[1];
    std::cout << "input file: " << path << "\n\n";
  }
  else {
    std::cerr << "Error: no input file provided\n";
    return 1;
  }

  std::string output_path = "homework_06/data/output.txt";

  double fire_x = 0.0;
  double fire_y = 0.0;
  double maneuver_x = 0.0;
  double maneuver_y = 0.0;

  bool need_maneuver = false;

  bool is_computed = calculate_fire_point(path.c_str(), fire_x, fire_y, need_maneuver, maneuver_x, maneuver_y);

  if (!is_computed) {
    return 1;
  }

  if (!write_result_file(output_path.c_str(), fire_x, fire_y, need_maneuver, maneuver_x, maneuver_y)) {
    return 1;
  }

  if (need_maneuver) {
    std::cout << "maneuver point\n";
    std::cout << maneuver_x << " " << maneuver_y << "\n";
  }

  std::cout << "fire point\n";
  std::cout << fire_x << " " << fire_y << "\n";

  return 0;
}