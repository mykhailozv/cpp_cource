#pragma once

#include <cstdint>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace mission_controller {

// Дискретні напрямки. Значення навмисно збігаються із константами
// повідомлення MoveCommand (UP=0, DOWN=1, LEFT=2, RIGHT=3).
// ROS-обгортка переносить їх на MoveCommand::UP / DOWN / LEFT / RIGHT.
constexpr std::uint8_t kUp = 0;
constexpr std::uint8_t kDown = 1;
constexpr std::uint8_t kLeft = 2;
constexpr std::uint8_t kRight = 3;

// Значення поля StudentStatus.state (мають збігатися з константами msg).
constexpr std::uint8_t kExploring = 0;
constexpr std::uint8_t kEngaging = 1;
constexpr std::uint8_t kReturning = 2;
constexpr std::uint8_t kDone = 3;
constexpr std::uint8_t kFailed = 4;

enum class ActionKind {
  Idle,    // чекаємо, поки світ повторно опублікує стан (напр. поки триває дія)
  Move,    // опублікувати /robot/cmd_move у напрямку `direction`
  Trigger  // викликати /payload/trigger для контакту contact_id / trigger_x / trigger_y
};

struct BrainCell {
  int x = 0;
  int y = 0;
  std::string cell_type;  // "#" / "." / "S" / "C" / "x"
  int contact_id = 0;
};

// Агностичний до ROS погляд на /robot/local_scan.
struct BrainScan {
  std::string scenario_name;
  int robot_x = 0;
  int robot_y = 0;
  std::vector<BrainCell> cells;
};

struct BrainAction {
  ActionKind kind = ActionKind::Idle;
  std::uint8_t direction = 0;
  int contact_id = 0;
  int trigger_x = 0;
  int trigger_y = 0;
};

// Чиста логіка дослідження без залежностей від ROS.
//
// Тримає в пам'яті вже досліджену карту та виконує пошук вглиб (DFS)
// прохідною частиною. Щоразу, коли в локальному огляді видно контакт (C),
// просить обгортку викликати /payload/trigger, і лише тоді виконує наступний
// рух. Так дія та рух природно серіалізуються, і вдається уникнути гонки
// cmd_move / enemy_down, до якої чутлива world model.
class ExplorerBrain {
public:
  ExplorerBrain();

  // Останній стан світу, отриманий із /robot/result.
  void set_world_status(const std::string& mission_result,
                        std::uint32_t max_steps,
                        std::uint32_t steps_taken);

  // Подати свіжий локальний огляд (оновлює внутрішню пам'ять карти).
  void process_scan(const BrainScan& scan);

  // Питає, що обгортці робити далі, з огляду на все побачене досі.
  BrainAction next_action();

  std::uint8_t state() const;

  // --- допоміжні методи для тестів ---
  std::set<std::pair<int, int>> visited_cells() const;
  std::set<int> triggered_contacts() const;

private:
  std::optional<std::pair<int, int>> decide_explore_move();
  bool is_passable(const std::pair<int, int>& pos) const;
  std::uint8_t direction_toward(const std::pair<int, int>& target) const;

  std::string mission_result_{"RUNNING"};
  std::uint32_t max_steps_ = 0;
  std::uint32_t steps_taken_ = 0;
  std::uint8_t state_ = kExploring;

  std::pair<int, int> robot_{0, 0};
  std::pair<int, int> start_{0, 0};
  bool has_start_ = false;

  struct ActiveContact {
    int id;
    std::pair<int, int> position;
  };

  // Знання про карту: позиція -> спостережений символ клітинки.
  std::map<std::pair<int, int>, char> known_;
  // Клітинки, на яких робот уже реально побував.
  std::set<std::pair<int, int>> visited_;
  // Трейл DFS (стек клітинок від старту до поточної).
  std::vector<std::pair<int, int>> path_;
  // Ідентифікатори контактів, для яких обгортку вже попросили викликати тригер.
  std::set<int> triggered_;
  // Контакти, видимі в останньому огляді.
  std::vector<ActiveContact> active_contacts_;
  // Останній напрямок руху (використовується для пріоритету прямого руху).
  std::uint8_t last_dir_ = kDown;
};

}  // namespace mission_controller