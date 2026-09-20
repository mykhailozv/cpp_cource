// ExplorerBrain інтеграційний тест без ROS: проганяє всі обов'язкові
// сценарії крізь справжню WorldModel (underground_world) і мозок рішення,
// перевіряючи повний контракт зарахування.
//
// Запускається в складі `colcon test` пакета mission_controller.

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

#include "gtest/gtest.h"

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

void expect_scenario_success(const std::string& filename)
{
  const auto scenario = underground_world::load_scenario(
    std::filesystem::path(UNDERGROUND_WORLD_SOURCE_DIR) / "config" / filename);
  underground_world::WorldModel world(scenario);
  mission_controller::ExplorerBrain brain;

  constexpr std::size_t kMaxIterations = 1000000;
  std::size_t guard = 0;
  bool finished = false;

  while (!finished && guard < kMaxIterations) {
    ++guard;
    const auto snap = world.snapshot();
    brain.set_world_status(snap.result.mission_result,
                           snap.result.max_steps,
                           snap.metrics.steps_taken);
    brain.process_scan(to_brain_scan(snap.scan));

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

  EXPECT_EQ(result.mission_result, "SUCCESS");
  EXPECT_EQ(metrics.contacts_seen, metrics.contacts_down);
  EXPECT_EQ(metrics.contacts_down, scenario.contacts.size());
  EXPECT_GE(metrics.map_coverage_percent, 99.0F);
  EXPECT_EQ(metrics.invalid_moves, 0U);
  EXPECT_EQ(metrics.invalid_triggers, 0U);
  EXPECT_EQ(metrics.duplicate_triggers, 0U);
  EXPECT_LT(metrics.steps_taken, scenario.max_steps);
}

}  // namespace

TEST(ExplorerBrainOfflineTest, TrainingCorridor)
{
  expect_scenario_success("training_corridor.yaml");
}

TEST(ExplorerBrainOfflineTest, SmallRooms)
{
  expect_scenario_success("small_rooms.yaml");
}

TEST(ExplorerBrainOfflineTest, BranchingTrench)
{
  expect_scenario_success("branching_trench.yaml");
}

TEST(ExplorerBrainOfflineTest, DeadEndBunker)
{
  expect_scenario_success("dead_end_bunker.yaml");
}