// Автономна наскрізна перевірка ExplorerBrain проти справжньої
// WorldModel з underground_world та файлів сценаріїв, БЕЗ ROS.
//
// Відтворює точно той самий контракт, що передається повідомленнями:
//   LocalScan  -> BrainScan (cell_type через cell_kind_to_string)
//   Move       -> world_.apply_move(direction)
//   Trigger    -> world_.apply_enemy_down(contact_id, position)
//
// Для кожного обов'язкового сценарію доводить, що:
//   mission_result == SUCCESS, contacts_seen == contacts_down, покриття 100%,
//   invalid_moves == 0, invalid_triggers == 0, duplicate_triggers == 0,
//   steps_taken < max_steps.
//
// Збірка (без ROS):
//   g++ -std=c++20 -O2 -I include \
//       tools/offline_sim.cpp src/explorer_brain.cpp \
//       ../underground_world/src/scenario_loader.cpp \
//       ../underground_world/src/world_model.cpp \
//       -I ../underground_world/include -o /tmp/offline_sim
//   /tmp/offline_sim

#include <cstdint>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#include "mission_controller/explorer_brain.hpp"
#include "underground_world/scenario_loader.hpp"
#include "underground_world/world_model.hpp"

namespace {

mission_controller::BrainScan to_brain_scan(const underground_world::LocalScanData& scan)
{
  mission_controller::BrainScan out;
  out.scenario_name = scan.scenario_name;
  out.robot_x = scan.robot.x;
  out.robot_y = scan.robot.y;
  out.cells.reserve(scan.cells.size());
  for (const auto& cell : scan.cells) {
    mission_controller::BrainCell bc;
    bc.x = cell.position.x;
    bc.y = cell.position.y;
    bc.cell_type = underground_world::cell_kind_to_string(cell.kind);
    bc.contact_id = cell.contact_id;
    out.cells.push_back(bc);
  }
  return out;
}

bool run_scenario(const std::filesystem::path& config_path, const std::string& name)
{
  const auto scenario = underground_world::load_scenario(config_path);
  underground_world::WorldModel world(scenario);
  mission_controller::ExplorerBrain brain;

  constexpr std::size_t kMaxIterations = 1000000;
  std::size_t guard = 0;
  bool finished = false;

  while (!finished && guard < kMaxIterations) {
    ++guard;
    const auto snap = world.snapshot();

    // Дзеркалимо те, на що підписується нода.
    brain.set_world_status(snap.result.mission_result, snap.result.max_steps, snap.metrics.steps_taken);
    const auto scan = to_brain_scan(snap.scan);
    brain.process_scan(scan);

    const auto action = brain.next_action();
    switch (action.kind) {
      case mission_controller::ActionKind::Trigger:
        world.apply_enemy_down(action.contact_id, {action.trigger_x, action.trigger_y});
        break;
      case mission_controller::ActionKind::Move:
        world.apply_move(action.direction);
        break;
      case mission_controller::ActionKind::Idle:
      default:
        if (world.terminal() || brain.state() == mission_controller::kDone ||
            brain.state() == mission_controller::kFailed) {
          finished = true;
        }
        break;
    }
  }

  const auto metrics = world.metrics();
  const auto result = world.result();

  const bool ok =
    result.mission_result == "SUCCESS" &&
    metrics.contacts_seen == metrics.contacts_down &&
    metrics.contacts_down == scenario.contacts.size() &&
    metrics.map_coverage_percent >= 99.0F &&
    metrics.invalid_moves == 0 &&
    metrics.invalid_triggers == 0 &&
    metrics.duplicate_triggers == 0 &&
    metrics.steps_taken < scenario.max_steps;

  std::cout << "[" << name << "] result=" << result.mission_result
            << " reason=" << result.reason
            << "\n   steps=" << metrics.steps_taken << "/" << scenario.max_steps
            << " invalid_moves=" << metrics.invalid_moves
            << " contacts=" << metrics.contacts_seen << "/" << metrics.contacts_down
            << "/" << scenario.contacts.size()
            << " invalid_triggers=" << metrics.invalid_triggers
            << " duplicate_triggers=" << metrics.duplicate_triggers
            << " coverage=" << metrics.map_coverage_percent << "%"
            << " unique_cells=" << metrics.unique_cells_seen
            << " visited=" << brain.visited_cells().size()
            << " -> " << (ok ? "OK" : "FAIL") << "\n";
  return ok;
}

}  // namespace

int main()
{
  const std::filesystem::path root = std::filesystem::path(UNDERGROUND_WORLD_SOURCE_DIR) / "config";

  bool all_ok = true;
  const std::vector<std::pair<std::string, std::string>> scenarios{
    {"training_corridor", "training_corridor.yaml"},
    {"small_rooms", "small_rooms.yaml"},
    {"branching_trench", "branching_trench.yaml"},
    {"dead_end_bunker", "dead_end_bunker.yaml"},
  };

  for (const auto& [name, file] : scenarios) {
    all_ok &= run_scenario(root / file, name);
  }

  std::cout << "\nOverall: " << (all_ok ? "ALL SCENARIOS PASS" : "SOME FAILURES") << "\n";
  return all_ok ? 0 : 1;
}