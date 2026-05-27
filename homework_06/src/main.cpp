#include <iostream>

#include "ballistics.hpp"

int main(int argc, char** argv)
{
  std::cout << "Hello World!!!" << std::endl;

  if (argc > 1) {
     std::cout << "input file: " << argv[1] << "\n\n";
  } else {
    std::cerr << "Error: no input file provided\n";
    return 1;
  }

  std::string output_path = "homework_06/data/output.txt";

  double fire_x, fire_y, maneuver_x, maneuver_y;
  bool need_maneuver;
  bool is_computed = calculate_fire_point(argv[1], fire_x, fire_y, need_maneuver, maneuver_x, maneuver_y);

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