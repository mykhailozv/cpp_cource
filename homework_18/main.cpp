
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_SSD1306.h>
#include <esp_timer.h>

#define I2C_SDA_PIN     8
#define I2C_SCL_PIN     9
#define OLED_WIDTH      128
#define OLED_HEIGHT     64
#define RING_BUF_SIZE   64

// Стан пристрою - змінні що змінюються в перериванні
volatile uint32_t g_measure_period_ms = 200;
volatile bool     g_timer_flag = false;
volatile uint32_t g_last_timer_us = 0;
volatile uint32_t g_actual_period_us = 0;

// Кільцевий буфер приймання UART
volatile char     g_uart_rx_buf[RING_BUF_SIZE];
volatile uint8_t g_uart_rx_head = 0;
volatile uint8_t g_uart_rx_tail = 0;

// Глобальні об'єкти периферії
Adafruit_MPU6050 mpu;
Adafruit_SSD1306 oled(OLED_WIDTH, OLED_HEIGHT, &Wire, -1);


void IRAM_ATTR timer_callback(void* arg) {
    (void)arg;
    
    // Тільки ОДНА дія: встановити прапорець
    uint32_t now = esp_timer_get_time();
    
    if (g_last_timer_us != 0) {
        g_actual_period_us = now - g_last_timer_us;
    }
    g_last_timer_us = now;
    
    g_timer_flag = true;
}


void uart_rx_event() {
    while (Serial.available() > 0) {
        char c = Serial.read();
        
        // Кільцевий буфер: якщо є місце - записуємо
        uint8_t next = (g_uart_rx_head + 1) % RING_BUF_SIZE;
        if (next != g_uart_rx_tail) {
            g_uart_rx_buf[g_uart_rx_head] = c;
            g_uart_rx_head = next;
        }
    }
}

void setup() {
    Serial.begin(115200);
    while (!Serial) { delay(10); }
    
    // Ініціалізація I²C шини
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN, 400000); // 400 кГц швидкий режим
    
    // Ініціалізація IMU MPU-6050
    if (!mpu.begin(0x68, &Wire)) {
        Serial.println("MPU6050 не знайдено!");
        while(1) delay(1000);
    }
    mpu.setAccelerometerRange(MPU6050_RANGE_2_G);
    Serial.println("MPU6050 ініціалізовано");
    
    // Ініціалізація OLED дисплея
    if (!oled.begin(SSD1306_SWITCHCAPVCC, 0x3C, &Wire)) {
        Serial.println("OLED не знайдено!");
        while(1) delay(1000);
    }
    oled.clearDisplay();
    oled.setTextSize(1);
    oled.setTextColor(SSD1306_WHITE);
    Serial.println("OLED ініціалізовано");
    
    // Налаштування апаратного таймера ESP32
    esp_timer_handle_t timer;
    esp_timer_create_args_t timer_args = {
        .callback = &timer_callback,
        .arg = nullptr,
        .name = "measure_timer"
    };
    esp_timer_create(&timer_args, &timer);
    esp_timer_start_periodic(timer, g_measure_period_ms * 1000);
    
    // Підписуємось на переривання UART
    Serial.onReceive(uart_rx_event);
    
    Serial.println("\n Система запущена!");
    Serial.println("Доступні команди:");
    Serial.println("  p <мс>  - змінити період виміру (наприклад: p 100)");
    Serial.println("  reset   - скинути лічильник часу\n");
}


// ==============================================
// ОБРОБКА КОМАНД UART
// ==============================================
void process_uart_commands() {
    // Читаємо рядок з кільцевого буфера
    static char line_buf[32];
    static uint8_t line_pos = 0;
    
    while (g_uart_rx_head != g_uart_rx_tail) {
        char c = g_uart_rx_buf[g_uart_rx_tail];
        g_uart_rx_tail = (g_uart_rx_tail + 1) % RING_BUF_SIZE;
        
        if (c == '\n' || c == '\r') {
            if (line_pos > 0) {
                line_buf[line_pos] = 0;
                line_pos = 0;
                
                // Розбираємо команду
                if (line_buf[0] == 'p') {
                    int new_period = atoi(line_buf + 2);
                    if (new_period >= 50 && new_period <= 5000) {
                        g_measure_period_ms = new_period;
                        Serial.printf(" Період змінено на %d мс\n", new_period);
                    } else {
                        Serial.println(" Період має бути 50..5000 мс");
                    }
                }
                else if (strcmp(line_buf, "reset") == 0) {
                    g_last_timer_us = 0;
                    Serial.println(" Лічильник скинуто");
                }
            }
        } else {
            if (line_pos < sizeof(line_buf)-1) {
                line_buf[line_pos++] = c;
            }
        }
    }
}


// ==============================================
// ГОЛОВНИЙ ЦИКЛ
// ==============================================
void loop() {

    // 1. Обробляємо команди (ніколи не блокуємо!)
    process_uart_commands();

    // 2. Чекаємо на сигнал від таймера
    if (!g_timer_flag) {
        return;
    }
    g_timer_flag = false; // Скидаємо прапорець ОДРАЗУ
    
    // 3. Отримуємо час, читаємо дані з датчика
    uint32_t uptime = millis();
    
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);
    
    // 4. ЗВІТ ПО UART - кожен період ОДИН РЯДОК
    Serial.printf(
        "t=%7d ms  ax=%+5.2f ay=%+5.2f az=%+5.2f  period=%4d us  mode=normal\n",
        uptime,
        a.acceleration.x / 9.81f,
        a.acceleration.y / 9.81f,
        a.acceleration.z / 9.81f,
        g_actual_period_us
    );

    // 5. Виводимо стан на OLED
    oled.clearDisplay();
    oled.setCursor(0, 0);
    oled.printf("Uptime: %d s\n", uptime / 1000);
    oled.printf("Ax: %.2f G\n", a.acceleration.x / 9.81f);
    oled.printf("Ay: %.2f G\n", a.acceleration.y / 9.81f);
    oled.printf("Az: %.2f G\n", a.acceleration.z / 9.81f);
    oled.printf("Period: %d us", g_actual_period_us);
    oled.display();
}
