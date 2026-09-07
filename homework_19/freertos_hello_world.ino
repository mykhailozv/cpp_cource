
// FreeRTOS - Такий собі Hello World для Arduino ESP32
// Можливо неправильно зрозумів
// Робота з FreeRtos - За бажанням
// Набросав простеньких 3 таски

#include <Arduino.h>

/**
 * Головні поняття FreeRTOS:
 * Task (Задача) - незалежна функція яка виконується у фоновому режимі
 * Scheduler (Планувальник) - ядро ОС, яке розподіляє час процесора між задачами
 * Priority (Пріоритет) - чим вище число, тим "важливіша" задача, отримує більше часу
 * Stack (Стек) - пам'ять виділена окремо для кожної задачі
 * Tick - мінімальна одиниця часу у FreeRTOS, зазвичай 1ms
 */

// Задача 1: виводить повідомлення кожну секунду
void task_hello_one(void * task_parameters) {
    while (true) {
        Serial.println(" Привіт задача 1! Кожну секунду");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    // В задачах FreeRTOS ніколи не потрібно робити return
}

// Задача 2: виводить повідомлення кожні 2 секунди
void task_hello_two(void * task_parameters) {
    while (true) {
        Serial.println("Привіт задача 2! Кожні 2 секунди");
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

// Задача 3: показує статус пам'яті кожні 3 секунди
void task_memory_monitor(void * task_parameters) {
    while (true) {
        Serial.printf("\n Статус пам'яті:\n");
        Serial.printf("   Вільна пам'ять кучи: %d байт\n", xPortGetFreeHeapSize());
        Serial.printf("   Мінімальна вільна пам'ять (за час роботи): %d байт\n\n", xPortGetMinimumEverFreeHeapSize());
        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}


void setup() {
    Serial.begin(115200);
    while (!Serial) { delay(10); }

    Serial.println("\n=====================================");
    Serial.println(" FreeRTOS Hello World запущено!");
    Serial.println("=====================================\n");

    xTaskCreate(
        task_hello_one,
        "HelloTask1",
        2048,
        NULL,
        1,
        NULL
    );

    xTaskCreate(
        task_hello_two,
        "HelloTask2",
        2048,
        NULL,
        1,
        NULL
    );

    xTaskCreate(
        task_memory_monitor,
        "MemMonitor",
        2048,
        NULL,
        2, // Вищий пріоритет, тому отримає процесор першою
        NULL
    );


    Serial.println("Задачі успішно створено!");
}


void loop() {
    // Порожньо! Вся робота виконується в задачах
}
