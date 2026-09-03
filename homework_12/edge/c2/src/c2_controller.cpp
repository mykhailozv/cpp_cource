#include "c2_controller.hpp"
#include "fc_link.hpp"     // MAVSDK обгортка, API описано у fc_link.hpp
#include "udp_socket.hpp"  // UDP прийом, API описано у udp_socket.hpp

#include <nlohmann/json.hpp>  // Розбiр JSON з точками маршруту вiд auto_stub

#include <fstream>
#include <iostream>
#include <string>

static constexpr uint16_t STUB_PORT = 14560;

static const char* state_name(C2State s) {
    switch (s) {
        case C2State::DISARMED:      return "DISARMED";
        case C2State::ARMED_HOLD:    return "ARMED_HOLD";
        case C2State::ARMED_GUIDED:  return "ARMED_GUIDED";
        case C2State::ARMED_MANUAL:  return "ARMED_MANUAL";
    }
    return "UNKNOWN";
}

struct C2Controller::Impl {
    C2State state = C2State::DISARMED;
    FcLink   fc;
    UdpSocket udp{STUB_PORT};
    std::ofstream log;

    // Прапорці одноразових дій.
    bool healthy_created = false;   // /tmp/c2_healthy після першого HEARTBEAT
    bool hold_sent       = false;   // fc.hold() один раз на вхід у ARMED_HOLD

    explicit Impl(uint16_t fc_port)
        : fc(fc_port)
    {
        // У Docker stdout не є TTY, тому за замовчуванням повністю буферується.
        // Вимикаємо буферизацію, щоб логи були видні через docker compose logs.
        std::cout.setf(std::ios::unitbuf);
        log.open("/var/log/c2/c2.log", std::ios::out | std::ios::app);
    }

    void log_line(const std::string& msg) {
        std::cout << msg << "\n";
        if (log.is_open()) {
            log << msg << "\n";
            log.flush();
        }
    }

    void transition(C2State next) {
        if (next == state) return;
        std::string line = std::string("[C2] state: ") + state_name(state)
                         + " -> " + state_name(next);
        log_line(line);
        state = next;
    }
};

C2Controller::C2Controller(uint16_t fc_port)
    : impl_(std::make_unique<Impl>(fc_port))
{
}

C2Controller::~C2Controller() = default;

void C2Controller::tick() {
    // 1. Healthcheck: після першого HEARTBEAT створюємо /tmp/c2_healthy.
    if (!impl_->healthy_created && impl_->fc.is_connected()) {
        std::ofstream("/tmp/c2_healthy").close();
        impl_->healthy_created = true;
    }

    // 2. Оновлення стану C2 за телеметрією FC.
    if (!impl_->fc.is_armed()) {
        impl_->transition(C2State::DISARMED);
    } else {
        switch (impl_->fc.flight_mode()) {
            case FcLink::FlightMode::Guided:
                impl_->transition(C2State::ARMED_GUIDED);
                break;
            case FcLink::FlightMode::Hold:
                impl_->transition(C2State::ARMED_HOLD);
                break;
            case FcLink::FlightMode::Manual:
                impl_->transition(C2State::ARMED_MANUAL);
                break;
            default:
                // Невідомий режим — лишаємо поточний стан.
                break;
        }
    }

    // 3. Дія при вході у стан: один раз надіслати fc.hold() для ARMED_HOLD.
    if (impl_->state == C2State::ARMED_HOLD && !impl_->hold_sent) {
        impl_->fc.hold();
        impl_->hold_sent = true;
    }
    if (impl_->state != C2State::ARMED_HOLD) {
        impl_->hold_sent = false;
    }

    // 4. Читання точки маршруту від auto_stub (неблокуючий UDP).
    char buf[512];
    ssize_t n = impl_->udp.recv(buf, sizeof(buf) - 1);
    if (n <= 0) {
        return;  // немає даних (EAGAIN) або помилка — нічого робити
    }
    buf[n] = '\0';

    // 5. Арбітраж: передати або заблокувати точку залежно від стану.
    switch (impl_->state) {
        case C2State::ARMED_GUIDED: {
            try {
                auto j = nlohmann::json::parse(buf);
                float north = j.at("north_m").get<float>();
                float east  = j.at("east_m").get<float>();
                impl_->fc.go_to_ned(north, east);
                impl_->log_line("[C2] fwd: north=" + std::to_string(north)
                              + " east=" + std::to_string(east));
            } catch (const nlohmann::json::exception& e) {
                impl_->log_line(std::string("[C2] error: invalid waypoint JSON: ") + e.what());
            }
            break;
        }
        case C2State::DISARMED:
        case C2State::ARMED_HOLD:
        case C2State::ARMED_MANUAL:
            impl_->log_line(std::string("[C2] blocked: waypoint in ")
                          + state_name(impl_->state));
            break;
    }
}

C2State C2Controller::current_state() const {
    return impl_->state;
}