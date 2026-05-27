#include <cmath>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <fstream>

#include "ballistics.hpp"

const double EPS = 1e-5;
const double G = 9.81;

bool compute_flight_time(double attackSpeed, double zd, double d, double l, double m, double& t)
{
  double V0 = attackSpeed;
  double Z0 = zd;
  double a = d * G * m - 2 * d * d * l * V0;

  if (std::fabs(a) < EPS) {
    std::cout << "a is too close to zero\n";
    return 1;
  }

  double b = -3 * G * m * m + 3 * d * l * m * V0;
  double c = 6 * m * m * Z0;

  // Solve cubic equation using Cardano's method (trigonometric form, p < 0)
  double p = -b * b / (3 * a * a);

  if (p >= -EPS) {
    std::cout << "p must be negative for Cardano trig solution\n";
    return false;
  }

  double q = 2 * b * b * b / (27 * a * a * a) + c / a;

  double acCosArg = 1.5 * q / p * std::sqrt(-3 / p);

  if (acCosArg > 1.0) {
    std::cout << "Invalid acos argument > 1.0 (" << acCosArg << "). Task cannot be solved" << std::endl;
    return false;
  }
  else if (acCosArg < -1.0) {
    std::cout << "Invalid acos argument < -1.0 (" << acCosArg << "). Task cannot be solved" << std::endl;
    return false;
  }

  double phi = std::acos(acCosArg);
  t = 2 * std::sqrt(-p / 3) * std::cos((phi + 4 * M_PI) / 3) - b / (3 * a);

  if (t <= EPS) {
    std::cout << "Invalid time t: " << t << std::endl;
    return false;
  }
  return true;
}

double calculate_horizontal_range_by_time(double V0, double Z0, double d, double m, double l, double t)
{
  double t_2 = t * t;
  double t_3 = t_2 * t;
  double t_4 = t_3 * t;
  double t_5 = t_4 * t;
  double d_2 = d * d;
  double d_3 = d_2 * d;
  double d_4 = d_3 * d;
  double l_2 = l * l;
  double l_3 = l_2 * l;
  double l_4 = l_3 * l;
  double m_2 = m * m;
  double m_3 = m_2 * m;
  double m_4 = m_3 * m;

  double h = V0 * t - t_2 * d * V0 / (2 * m) + t_3 * (6 * d * G * l * m - 6 * d_2 * (l_2 - 1) * V0) / (36 * m_2) +
             t_4 * (-6 * d_2 * G * l * (1 + l_2 + l_4) * m + 3 * d_3 * l_2 * (1 + l_2) * V0 + 6 * d_3 * l_4 * (1 + l_2) * V0) /
               (36 * (1 + l_2) * (1 + l_2) * m_3) +
             t_5 * (3 * d_3 * G * l_3 * m - 3 * d_4 * l_2 * (1 + l_2) * V0) / (36 * (1 + l_2) * m_4);

  return h;
}

bool compute_horizontal_range(double attackSpeed, double zd, double d, double l, double m, double& h)
{
  if (m < EPS) {
    std::cerr << "Invalid m(mass): " << m << std::endl;
    return false;
  }

  if (l < -EPS) {
    std::cerr << "Invalid l(lift): " << l << std::endl;
    return false;
  }

  double t;
  if (compute_flight_time(attackSpeed, zd, d, l, m, t)) {
    h = calculate_horizontal_range_by_time(attackSpeed, zd, d, m, l, t);
    if (h <= EPS) {
      std::cerr << "Invalid h: " << h << std::endl;
      return false;
    }
    return true;
  }
  std::cerr << "Failed to compute flight time\n";
  return false;
}

bool get_ammo_parameters(const char* ammo_name, double& d, double& l, double& m)
{
  if (std::strlen(ammo_name) > 15) {
    std::cerr << "Ammo name name too long: " << ammo_name << std::endl;
    return false;
  }

  if (std::strcmp(ammo_name, "VOG-17") == 0) {
    m = 0.35;
    d = 0.07;
    l = 0.0;
  }
  else if (std::strcmp(ammo_name, "M67") == 0) {
    m = 0.6;
    d = 0.10;
    l = 0.0;
  }
  else if (std::strcmp(ammo_name, "RKG-3") == 0) {
    m = 1.2;
    d = 0.10;
    l = 0.0;
  }
  else if (std::strcmp(ammo_name, "GLIDING-VOG") == 0) {
    m = 0.45;
    d = 0.10;
    l = 1.0;
  }
  else if (std::strcmp(ammo_name, "GLIDING-RKG") == 0) {
    m = 1.4;
    d = 0.10;
    l = 1.0;
  }
  else {
    std::cout << "Unknown ammo_name\n";
    return false;
  }

  return true;
}

bool read_input_file(const char* path,
                     double& xd,
                     double& yd,
                     double& zd,
                     double& target_x,
                     double& target_y,
                     double& attack_speed,
                     double& acceleration_path,
                     std::string& ammo_name)
{
  std::ifstream input_stream(path);

  // std::cout << std::filesystem::current_path() << '\n';

  if (!input_stream.is_open()) {
    std::cout << "Failed to open " << path << " file\n";
    return false;
  }

  input_stream >> xd >> yd >> zd >> target_x >> target_y >> attack_speed >> acceleration_path >> ammo_name;

  if (input_stream.fail()) {
    std::cerr << "Invalid input file format\n";
    return false;
  }

  return true;
}

double calculate_distance(double x1, double y1, double x2, double y2)
{
  return std::sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
}

bool calculate_fire_point(const char* path, double& fire_x, double& fire_y, bool& need_maneur, double& maneuver_x, double& maneuver_y)
{
  double xd, yd, zd, target_x, target_y, attack_speed, acceleration_path;
  std::string ammo_name;

  if (!read_input_file(path, xd, yd, zd, target_x, target_y, attack_speed, acceleration_path, ammo_name)) {
    return false;
  }

  if (zd < EPS) {
    std::cout << "Invalid zd " << zd << std ::endl;
    return false;
  }

  // Zero attackSpeed may not be appropriate here; consider using < -EPS if needed
  if (attack_speed < EPS) {
    std::cout << "Invalid attackSpeed " << attack_speed << std ::endl;
    return false;
  }

  // Positive acceleration path is required to reach attack speed
  if (acceleration_path < EPS) {
    std::cout << "Invalid accelerationPath " << acceleration_path << std ::endl;
    return false;
  }

  double mass, drag, lift;

  if (!get_ammo_parameters(ammo_name.c_str(), drag, lift, mass)) {
    return false;
  }

  double horizontal_range;

  if (!compute_horizontal_range(attack_speed, zd, drag, lift, mass, horizontal_range)) {
    return false;
  }

  double distance = calculate_distance(xd, yd, target_x, target_y);

  if (distance < EPS) {
    // Zero distance case: move drone in fixed direction (-1, 0)
    std::cout << "Zero distance case: calculating maneuver point with (-1, 0) direction" << std::endl;
    maneuver_x = target_x - (horizontal_range + acceleration_path);
    maneuver_y = target_y;
    xd = maneuver_x;
    yd = maneuver_y;
    distance = horizontal_range + acceleration_path;
    need_maneur = true;
  }
  else if (horizontal_range + acceleration_path > distance) {
    // Not enough distance to accelerate and reach target - move drone back
    std::cout << "Distance is insufficient - calculating maneuver point" << std::endl;
    double ux = (target_x - xd) / distance;
    double uy = (target_y - yd) / distance;
    maneuver_x = target_x - (horizontal_range + acceleration_path) * ux;
    maneuver_y = target_y - (horizontal_range + acceleration_path) * uy;
    need_maneur = true;
  }
  else {
    maneuver_x = target_x;
    maneuver_y = target_y;
    need_maneur = false;
  }

  double ratio = (distance - horizontal_range) / distance;
  fire_x = xd + (target_x - xd) * ratio;
  fire_y = yd + (target_y - yd) * ratio;

  return true;
}

bool write_result_file(const char* path, double& fire_x, double& fire_y, bool& need_maneur, double& maneuver_x, double& maneuver_y)
{
  std::ofstream output_stream(path);

  if (!output_stream.is_open()) {
    std::cout << "Failed to open output.txt file\n";
    return false;
  }

  if (need_maneur) {
    output_stream << maneuver_x << " " << maneuver_y << std::endl;
  }

  output_stream << fire_x << " " << fire_y << std::endl;

  return true;
}