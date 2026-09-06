#include <Arduino.h>
#include <Wire.h>
#include <esp_timer.h>

const int PIN_SDA = 8;
const int PIN_SCL = 9;

volatile bool     g_timer_flag = false;
volatile uint32_t g_actual_period_us = 0;
volatile uint32_t g_period_ms = 200;

#define BUF_SIZE 64
volatile char     g_rx_buf[BUF_SIZE];
volatile uint8_t  g_rx_head = 0;
volatile uint8_t  g_rx_tail = 0;

esp_timer_handle_t g_timer;


void IRAM_ATTR timer_callback(void* arg) {
  static uint32_t last = 0;
  uint32_t now = esp_timer_get_time();
  
  if (last) g_actual_period_us = now - last;
  last = now;

  g_timer_flag = true;
}


void serialEvent() {
  while (Serial.available()) {
    char c = Serial.read();
    uint8_t next = (g_rx_head + 1) % BUF_SIZE;
    if (next != g_rx_tail) {
      g_rx_buf[g_rx_head] = c;
      g_rx_head = next;
    }
  }
}


void setup() {
  Serial.begin(115200);
  while (!Serial);
  Wire.begin(PIN_SDA, PIN_SCL);

  esp_timer_create_args_t args = { .callback = timer_callback };
  esp_timer_create(&args, &g_timer);
  esp_timer_start_periodic(g_timer, g_period_ms * 1000);

  Serial.println("✅ ГОТОВО! ДЗ 36 cMiltech");
  Serial.println("Команди: p [мс]  - змінити період");
  Serial.println("");
}


void loop() {

  // Обробка команд UART
  static char line[32];
  static uint8_t line_pos = 0;

  while (g_rx_head != g_rx_tail) {
    char c = g_rx_buf[g_rx_tail];
    g_rx_tail = (g_rx_tail + 1) % BUF_SIZE;

    if (c == '\n' || c == '\r') {
      if (line_pos > 0) {
        line[line_pos] = 0;
        line_pos = 0;
        
        if (line[0] == 'p') {
          int new_p = atoi(line+2);
          if (new_p >= 50 && new_p <= 2000) {
            g_period_ms = new_p;

            // ✅ ФІКС: правильна зміна періоду таймера
            esp_timer_stop(g_timer);
            esp_timer_delete(g_timer);
            
            esp_timer_create_args_t args = { .callback = timer_callback };
            esp_timer_create(&args, &g_timer);
            esp_timer_start_periodic(g_timer, new_p * 1000ULL);

            Serial.printf("✅ Період змінено на %d мс\n", new_p);
          }
        }
      }
    } else {
      if (line_pos < 31) line[line_pos++] = c;
    }
  }


  // Чекаємо сигнал від таймера
  if (!g_timer_flag) return;
  g_timer_flag = false;


  // Читаємо MPU6050
  int16_t ax, ay, az;

  Wire.beginTransmission(0x68);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(0x68, 6);

  ax  = Wire.read() << 8 | Wire.read();
  ay  = Wire.read() << 8 | Wire.read();
  az  = Wire.read() << 8 | Wire.read();


  // Звіт по UART
  Serial.printf(
    "t=%7d ms  ax=%+5.2f ay=%+5.2f az=%+5.2f  period=%d us  mode=ok\n",
    millis(),
    ax / 16384.0f,
    ay / 16384.0f,
    az / 16384.0f,
    g_actual_period_us
  );

}
