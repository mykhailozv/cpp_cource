#pragma once
#include <string>

bool compute_horizontal_range(double attackSpeed, double zd, double d, double l, double m, double& h);
bool get_ammo_parameters(const char* ammo_name, double& d, double& l, double& m);
bool read_input_file(const char* path,
                     double& xd,
                     double& yd,
                     double& zd,
                     double& target_x,
                     double& target_y,
                     double& attack_speed,
                     double& acceleration_path,
                     std::string& ammo_name);
bool calculate_fire_point(const char* path, double& fire_x, double& fire_y, bool& need_maneur, double& maneuver_x, double& maneuver_y);
bool write_result_file(const char* path, double& fire_x, double& fire_y, bool& need_maneur, double& maneuver_x, double& maneuver_y);