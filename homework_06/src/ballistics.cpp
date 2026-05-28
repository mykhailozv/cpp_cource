#include <cmath>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <fstream>

#include "ballistics.hpp"

const double EPS = 1e-5;
const double GRAVITY = 9.81;

bool compute_flight_time(double attack_speed,     // NOLINT(bugprone-easily-swappable-parameters)
                         double height_z,         // NOLINT(bugprone-easily-swappable-parameters)
                         double drag,             // NOLINT(bugprone-easily-swappable-parameters)
                         double lift,             // NOLINT(bugprone-easily-swappable-parameters)
                         double mass,             // NOLINT(bugprone-easily-swappable-parameters)
                         double& time_of_flight)  // NOLINT(bugprone-easily-swappable-parameters)
{
  double vel_0 = attack_speed;
  double height_0 = height_z;

  // NOLINTBEGIN(readability-identifier-length)
  double a = drag * GRAVITY * mass - 2 * drag * drag * lift * vel_0;

  if (std::fabs(a) < EPS) {
    std::cout << "a is too close to zero\n";
    return false;
  }

  double b = -3 * GRAVITY * mass * mass + 3 * drag * lift * mass * vel_0;
  double c = 6 * mass * mass * height_0;

  // Solve cubic equation using Cardano's method (trigonometric form, p < 0)
  double p = -b * b / (3 * a * a);

  if (p >= -EPS) {
    std::cout << "p must be negative for Cardano trig solution\n";
    return false;
  }

  double q = 2 * b * b * b / (27 * a * a * a) + c / a;
  // NOLINTEND(readability-identifier-length)

  double acCosArg = 1.5 * q / p * std::sqrt(-3 / p);

  if (acCosArg > 1.0) {
    std::cout << "Invalid acos argument > 1.0 (" << acCosArg << "). Task cannot be solved" << "\n";
    return false;
  }

  if (acCosArg < -1.0) {
    std::cout << "Invalid acos argument < -1.0 (" << acCosArg << "). Task cannot be solved" << "\n";
    return false;
  }

  double phi = std::acos(acCosArg);
  time_of_flight = 2 * std::sqrt(-p / 3) * std::cos((phi + 4 * M_PI) / 3) - b / (3 * a);

  if (time_of_flight <= EPS) {
    std::cout << "Invalid time t: " << time_of_flight << "\n";
    return false;
  }

  return true;
}

double calculate_horizontal_range_by_time(double vel_0,           // NOLINT(bugprone-easily-swappable-parameters)
                                          double height_0,        // NOLINT(bugprone-easily-swappable-parameters)
                                          double drag,            // NOLINT(bugprone-easily-swappable-parameters)
                                          double mass,            // NOLINT(bugprone-easily-swappable-parameters)
                                          double lift,            // NOLINT(bugprone-easily-swappable-parameters)
                                          double time_of_flight)  // NOLINT(bugprone-easily-swappable-parameters)
{
  double t_2 = time_of_flight * time_of_flight;
  double t_3 = t_2 * time_of_flight;
  double t_4 = t_3 * time_of_flight;
  double t_5 = t_4 * time_of_flight;
  double d_2 = drag * drag;
  double d_3 = d_2 * drag;
  double d_4 = d_3 * drag;
  double l_2 = lift * lift;
  double l_3 = l_2 * lift;
  double l_4 = l_3 * lift;
  double m_2 = mass * mass;
  double m_3 = m_2 * mass;
  double m_4 = m_3 * mass;

  double horizontal_range =
    vel_0 * time_of_flight - t_2 * drag * vel_0 / (2 * mass) +
    t_3 * (6 * drag * GRAVITY * lift * mass - 6 * d_2 * (l_2 - 1) * vel_0) / (36 * m_2) +
    t_4 * (-6 * d_2 * GRAVITY * lift * (1 + l_2 + l_4) * mass + 3 * d_3 * l_2 * (1 + l_2) * vel_0 + 6 * d_3 * l_4 * (1 + l_2) * vel_0) /
      (36 * (1 + l_2) * (1 + l_2) * m_3) +
    t_5 * (3 * d_3 * GRAVITY * l_3 * mass - 3 * d_4 * l_2 * (1 + l_2) * vel_0) / (36 * (1 + l_2) * m_4);

  return horizontal_range;
}

bool compute_horizontal_range(double attack_speed, double height_z, double drag, double lift, double mass, double& horizontal_range)
{
  if (mass < EPS) {
    std::cerr << "Invalid m(mass): " << mass << "\n";
    return false;
  }

  if (lift < -EPS) {
    std::cerr << "Invalid l(lift): " << lift << "\n";
    return false;
  }

  double time_of_flight = 0.0;

  if (compute_flight_time(attack_speed, height_z, drag, lift, mass, time_of_flight)) {
    horizontal_range = calculate_horizontal_range_by_time(attack_speed, height_z, drag, mass, lift, time_of_flight);
    if (horizontal_range <= EPS) {
      std::cerr << "Invalid h: " << horizontal_range << "\n";
      return false;
    }
    return true;
  }

  std::cerr << "Failed to compute flight time\n";
  return false;
}

bool get_ammo_parameters(const char* ammo_name, double& drag, double& lift, double& mass)  // NOLINT(bugprone-easily-swappable-parameters)
{
  if (std::strlen(ammo_name) > 15) {
    std::cerr << "Ammo name name too long: " << ammo_name << "\n";
    return false;
  }

  if (std::strcmp(ammo_name, "VOG-17") == 0) {
    mass = 0.35;
    drag = 0.07;
    lift = 0.0;
  }
  else if (std::strcmp(ammo_name, "M67") == 0) {
    mass = 0.6;
    drag = 0.10;
    lift = 0.0;
  }
  else if (std::strcmp(ammo_name, "RKG-3") == 0) {
    mass = 1.2;
    drag = 0.10;
    lift = 0.0;
  }
  else if (std::strcmp(ammo_name, "GLIDING-VOG") == 0) {
    mass = 0.45;
    drag = 0.10;
    lift = 1.0;
  }
  else if (std::strcmp(ammo_name, "GLIDING-RKG") == 0) {
    mass = 1.4;
    drag = 0.10;
    lift = 1.0;
  }
  else {
    std::cout << "Unknown ammo_name\n";
    return false;
  }

  return true;
}

bool read_input_file(const char* path,
                     double& drone_x,
                     double& drone_y,
                     double& height_z,
                     double& target_x,
                     double& target_y,
                     double& attack_speed,
                     double& acceleration_path,
                     std::string& ammo_name)
{
  std::ifstream input_stream(path);

  if (!input_stream.is_open()) {
    std::cout << "Failed to open " << path << " file\n";
    return false;
  }

  input_stream >> drone_x >> drone_y >> height_z >> target_x >> target_y >> attack_speed >> acceleration_path >> ammo_name;

  if (input_stream.fail()) {
    std::cerr << "Invalid input file format\n";
    return false;
  }

  return true;
}

// NOLINTBEGIN(readability-identifier-length)
double calculate_distance(double x1, double y1, double x2, double y2)  // NOLINT(bugprone-easily-swappable-parameters)
{
  return std::sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
}
// NOLINTEND(readability-identifier-length)

bool calculate_fire_point(const char* path,    // NOLINT(bugprone-easily-swappable-parameters)
                          double& fire_x,      // NOLINT(bugprone-easily-swappable-parameters)
                          double& fire_y,      // NOLINT(bugprone-easily-swappable-parameters)
                          bool& need_maneur,   // NOLINT(bugprone-easily-swappable-parameters)
                          double& maneuver_x,  // NOLINT(bugprone-easily-swappable-parameters)
                          double& maneuver_y)  // NOLINT(bugprone-easily-swappable-parameters)
{
  double drone_x = 0.0;
  double drone_y = 0.0;
  double height_z = 0.0;
  double target_x = 0.0;
  double target_y = 0.0;
  double attack_speed = 0.0;
  double acceleration_path = 0.0;
  std::string ammo_name;

  if (!read_input_file(path, drone_x, drone_y, height_z, target_x, target_y, attack_speed, acceleration_path, ammo_name)) {
    return false;
  }

  if (height_z < EPS) {
    std::cout << "Invalid height_z " << height_z << "\n";
    return false;
  }

  // Zero attackSpeed may not be appropriate here; consider using < -EPS if needed
  if (attack_speed < EPS) {
    std::cout << "Invalid attackSpeed " << attack_speed << "\n";
    return false;
  }

  // Positive acceleration path is required to reach attack speed
  if (acceleration_path < EPS) {
    std::cout << "Invalid accelerationPath " << acceleration_path << "\n";
    return false;
  }

  double mass = 0.0;
  double drag = 0.0;
  double lift = 0.0;

  if (!get_ammo_parameters(ammo_name.c_str(), drag, lift, mass)) {
    return false;
  }

  double horizontal_range = 0.0;

  if (!compute_horizontal_range(attack_speed, height_z, drag, lift, mass, horizontal_range)) {
    return false;
  }

  double distance = calculate_distance(drone_x, drone_y, target_x, target_y);

  if (distance < EPS) {
    // Zero distance case: move drone in fixed direction (-1, 0)
    std::cout << "Zero distance case: calculating maneuver point with (-1, 0) direction" << "\n";
    maneuver_x = target_x - (horizontal_range + acceleration_path);
    maneuver_y = target_y;
    drone_x = maneuver_x;
    drone_y = maneuver_y;
    distance = horizontal_range + acceleration_path;
    need_maneur = true;
  }
  else if (horizontal_range + acceleration_path > distance) {
    // Not enough distance to accelerate and reach target - move drone back
    std::cout << "Distance is insufficient - calculating maneuver point" << "\n";
    double unit_x = (target_x - drone_x) / distance;
    double unit_y = (target_y - drone_y) / distance;
    maneuver_x = target_x - (horizontal_range + acceleration_path) * unit_x;
    maneuver_y = target_y - (horizontal_range + acceleration_path) * unit_y;
    need_maneur = true;
  }
  else {
    maneuver_x = target_x;
    maneuver_y = target_y;
    need_maneur = false;
  }

  double ratio = (distance - horizontal_range) / distance;
  fire_x = drone_x + (target_x - drone_x) * ratio;
  fire_y = drone_y + (target_y - drone_y) * ratio;

  return true;
}

bool write_result_file(const char* path,    // NOLINT(bugprone-easily-swappable-parameters)
                       double& fire_x,      // NOLINT(bugprone-easily-swappable-parameters)
                       double& fire_y,      // NOLINT(bugprone-easily-swappable-parameters)
                       bool& need_maneur,   // NOLINT(bugprone-easily-swappable-parameters)
                       double& maneuver_x,  // NOLINT(bugprone-easily-swappable-parameters)
                       double& maneuver_y)  // NOLINT(bugprone-easily-swappable-parameters)
{
  std::ofstream output_stream(path);

  if (!output_stream.is_open()) {
    std::cout << "Failed to open output.txt file\n";
    return false;
  }

  if (need_maneur) {
    output_stream << maneuver_x << " " << maneuver_y << "\n";
  }

  output_stream << fire_x << " " << fire_y << "\n";

  return true;
}