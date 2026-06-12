#pragma once
#include <string>

bool compute_horizontal_range(double attack_speed,
                              double height_z,
                              double drag,
                              double lift,
                              double mass,
                              double& horizontal_range);                                    // NOLINT(bugprone-easily-swappable-parameters)
bool get_ammo_parameters(const char* ammo_name, double& drag, double& lift, double& mass);  // NOLINT(bugprone-easily-swappable-parameters)
bool read_input_file(const char* path,
                     double& drone_x,
                     double& drone_y,
                     double& height_z,
                     double& target_x,
                     double& target_y,
                     double& attack_speed,
                     double& acceleration_path,
                     std::string& ammo_name);  // NOLINT(bugprone-easily-swappable-parameters)
bool calculate_fire_point(const char* path,
                          double& fire_x,
                          double& fire_y,
                          bool& need_maneur,
                          double& maneuver_x,
                          double& maneuver_y);  // NOLINT(bugprone-easily-swappable-parameters)
bool write_result_file(const char* path,
                       double& fire_x,
                       double& fire_y,
                       bool& need_maneur,
                       double& maneuver_x,
                       double& maneuver_y);  // NOLINT(bugprone-easily-swappable-parameters)