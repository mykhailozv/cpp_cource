/**
 */

#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <cstdint>
#include <cstring>
#include <chrono>
#include <thread>
#include <stdexcept>

// ========== Константи MPU-6050 з даташиту ==========
constexpr uint8_t MPU6050_ADDR = 0x68;
constexpr uint8_t REG_WHO_AM_I    = 0x75;
constexpr uint8_t REG_PWR_MGMT_1  = 0x6B;
constexpr uint8_t REG_ACCEL_XOUT_H = 0x3B;
constexpr uint8_t WHO_AM_I_EXPECTED = 0x68;

constexpr float ACCEL_LSB = 16384.0f;
constexpr float GYRO_LSB  = 131.0f;
constexpr float TEMP_LSB  = 340.0f;
constexpr float TEMP_OFFSET = 36.53f;

class I2CBus {
private:
    int fd;
public:
    I2CBus(const char* bus_path) {
        fd = open(bus_path, O_RDWR);
        if (fd < 0) throw std::runtime_error(std::string("Не вдалося відкрити шину: ") + strerror(errno));
    }
    ~I2CBus() { if (fd >= 0) close(fd); }
    
    void select_device(uint8_t addr) {
        if (ioctl(fd, I2C_SLAVE, addr) < 0) throw std::runtime_error("Неможливо обрати пристрій");
    }
    
    void read_register(uint8_t reg, uint8_t* buf, size_t len) {
        if (write(fd, &reg, 1) != 1) throw std::runtime_error("Помилка запису адреси регістру");
        if (read(fd, buf, len) != (ssize_t)len) throw std::runtime_error("Помилка читання даних");
    }
    
    void write_register(uint8_t reg, uint8_t val) {
        uint8_t b[2] = {reg, val};
        if (write(fd, b, 2) != 2) throw std::runtime_error("Помилка запису регістру");
    }
};

struct SensorData {
    float ax, ay, az, gx, gy, gz, temp;
};

class MPU6050 {
    I2CBus& bus;
public:
    MPU6050(I2CBus& b) : bus(b) {
        uint8_t id;
        bus.read_register(REG_WHO_AM_I, &id, 1);
        if (id != WHO_AM_I_EXPECTED) throw std::runtime_error("Неправильний ID пристрою!");
        bus.write_register(REG_PWR_MGMT_1, 0);
    }
    
    SensorData read() {
        uint8_t buf[14];
        bus.read_register(REG_ACCEL_XOUT_H, buf, 14);
        
        SensorData d;
        d.ax = (int16_t)((buf[0]<<8)|buf[1]) / ACCEL_LSB;
        d.ay = (int16_t)((buf[2]<<8)|buf[3]) / ACCEL_LSB;
        d.az = (int16_t)((buf[4]<<8)|buf[5]) / ACCEL_LSB;
        d.temp = ((int16_t)((buf[6]<<8)|buf[7]) / TEMP_LSB) + TEMP_OFFSET;
        d.gx = (int16_t)((buf[8]<<8)|buf[9]) / GYRO_LSB;
        d.gy = (int16_t)((buf[10]<<8)|buf[11]) / GYRO_LSB;
        d.gz = (int16_t)((buf[12]<<8)|buf[13]) / GYRO_LSB;
        return d;
    }
};

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cerr << "Використання: " << argv[0] << " /dev/i2c-1 0x68\n";
        return 1;
    }
    try {
        I2CBus bus(argv[1]);
        bus.select_device((uint8_t)strtol(argv[2], 0, 0));
        MPU6050 dev(bus);
        
        std::cout << "MPU6050 підключено успішно!\n";
        while(true) {
            SensorData d = dev.read();
            std::cout << "\033[H\033[2J";
            std::cout << "═══════ MPU-6050 ═══════\n";
            std::cout << "Температура: " << d.temp << " °C\n";
            std::cout << "A: X=" << d.ax << " Y=" << d.ay << " Z=" << d.az << " g\n";
            std::cout << "G: X=" << d.gx << " Y=" << d.gy << " Z=" << d.gz << " °/s\n";
            std::cout << "════════════════════════\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
    } catch (std::exception& e) {
        std::cerr << "Помилка: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
