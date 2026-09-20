#include "mission_controller/explorer_brain.hpp"

#include <algorithm>
#include <queue>

namespace mission_controller {

namespace {

constexpr int kDx[4] = {0, 0, -1, 1};  // UP, DOWN, LEFT, RIGHT
constexpr int kDy[4] = {-1, 1, 0, 0};

}  // namespace

ExplorerBrain::ExplorerBrain()
{
}

void ExplorerBrain::set_world_status(const std::string& mission_result,
                                     std::uint32_t max_steps,
                                     std::uint32_t steps_taken)
{
  mission_result_ = mission_result;
  max_steps_ = max_steps;
  steps_taken_ = steps_taken;
}

void ExplorerBrain::process_scan(const BrainScan& scan)
{
  robot_ = {scan.robot_x, scan.robot_y};

  if (!has_start_) {
    start_ = robot_;
    has_start_ = true;
  }

  visited_.insert(robot_);
  active_contacts_.clear();

  for (const auto& cell : scan.cells) {
    const std::pair<int, int> pos{cell.x, cell.y};
    // Запам'ятовуємо все, що не є стіною; стіни теж зберігаємо як є.
    if (!cell.cell_type.empty()) {
      known_[pos] = cell.cell_type.front();
    }
    if (cell.cell_type == "C") {
      ActiveContact contact;
      contact.id = cell.contact_id;
      contact.position = pos;
      active_contacts_.push_back(contact);
    }
  }

  // Детермінований порядок обробки контактів.
  std::sort(active_contacts_.begin(), active_contacts_.end(),
            [](const ActiveContact& a, const ActiveContact& b) { return a.id < b.id; });
}

BrainAction ExplorerBrain::next_action()
{
  BrainAction action;

  // 1) Дія має пріоритет: розбираємось з кожним видимим контактом перед рухом.
  if (!active_contacts_.empty()) {
    for (const auto& contact : active_contacts_) {
      if (triggered_.find(contact.id) == triggered_.end()) {
        triggered_.insert(contact.id);
        action.kind = ActionKind::Trigger;
        action.contact_id = contact.id;
        action.trigger_x = contact.position.first;
        action.trigger_y = contact.position.second;
        state_ = kEngaging;
        return action;
      }
    }
    // Усі видимі контакти вже активовано; чекаємо, поки світ опрацює
    // /payload/enemy_down і повторно опублікує огляд.
    state_ = kEngaging;
    action.kind = ActionKind::Idle;
    return action;
  }

  // 2) Термінальні результати.
  if (mission_result_ == "FAILED_MAX_STEPS") {
    state_ = kFailed;
    action.kind = ActionKind::Idle;
    return action;
  }

  // Щойно світ повідомив SUCCESS, він перестає приймати команди: зупиняємось.
  if (mission_result_ == "SUCCESS") {
    state_ = kDone;
    action.kind = ActionKind::Idle;
    return action;
  }

  // 3) Досі RUNNING: продовжуємо дослідження (пошук вглиб прохідною частиною).
  state_ = kExploring;
  const auto target = decide_explore_move();
  if (target.has_value()) {
    action.kind = ActionKind::Move;
    action.direction = direction_toward(*target);
    last_dir_ = action.direction;
    return action;
  }

  // Дослідження завершилось (відвідано всі досяжні прохідні клітинки, опрацьовано
  // всі контакти). Світ невдовзі має повідомити SUCCESS.
  state_ = kDone;
  action.kind = ActionKind::Idle;
  return action;
}

std::optional<std::pair<int, int>> ExplorerBrain::decide_explore_move()
{
  // Засіваємо стек DFS стартовою клітинкою на першому кроці дослідження.
  if (path_.empty()) {
    path_.push_back(robot_);
  }
  // Інваріант: path_.back() == robot_ після початку дослідження.
  if (path_.back() != robot_) {
    // Запобіжник: пере-коренюємось. Практично трапляється лише на першому виклику.
    path_.clear();
    path_.push_back(robot_);
  }

  // Намагаємось спершу продовжити рух у тому самому напрямку, що й попередній,
  // а далі решту напрямків, щоб коридори проходити прямо, а не розвертатись.
  // Це зменшує повторні відвідування й тримає шлях коротким.
  std::uint8_t order[4];
  std::size_t order_index = 0;
  for (int i = 0; i < 4; ++i) {
    const std::uint8_t dir = static_cast<std::uint8_t>((last_dir_ + i) % 4);
    order[order_index++] = dir;
  }

  for (const std::uint8_t dir : order) {
    const std::pair<int, int> neighbor{robot_.first + kDx[dir], robot_.second + kDy[dir]};
    if (visited_.find(neighbor) != visited_.end()) {
      continue;
    }
    if (!is_passable(neighbor)) {
      continue;
    }
    path_.push_back(neighbor);
    return neighbor;
  }

  // Немає неперевірених прохідних сусідів: відступаємо стеком DFS назад.
  if (path_.size() >= 2) {
    path_.pop_back();  // прибираємо поточну позицію
    const auto parent = path_.back();
    return parent;
  }
  // Ми на старті і вперед рухатись нікуди: дослідження завершено.
  return std::nullopt;
}

bool ExplorerBrain::is_passable(const std::pair<int, int>& pos) const
{
  const auto it = known_.find(pos);
  if (it == known_.end()) {
    return false;  // за межами карти / невідомо -> вважаємо непрохідним
  }
  return it->second != '#';
}

std::uint8_t ExplorerBrain::direction_toward(const std::pair<int, int>& target) const
{
  if (target.second < robot_.second) {
    return kUp;
  }
  if (target.second > robot_.second) {
    return kDown;
  }
  if (target.first < robot_.first) {
    return kLeft;
  }
  if (target.first > robot_.first) {
    return kRight;
  }
  return kDown;
}

std::uint8_t ExplorerBrain::state() const
{
  return state_;
}

std::set<std::pair<int, int>> ExplorerBrain::visited_cells() const
{
  return visited_;
}

std::set<int> ExplorerBrain::triggered_contacts() const
{
  return triggered_;
}

}  // namespace mission_controller