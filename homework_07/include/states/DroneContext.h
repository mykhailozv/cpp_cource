#pragma once

#include "simulation/SimStep.h"
#include "config/DroneConfig.h"


struct DroneContext {
    const DroneConfig* cfg{nullptr};
    SimStep* current{nullptr};   // simStep (поточний крок)
    SimStep* next{nullptr};      // simStep + 1 (наступний крок, який заповнюємо)
    double distance{0.0};        // відстань до прогнозованої цілі
    double neededDir{0.0};       // потрібний напрямок
    double ammoRange{0.0};       // calculateHorizontalAmmoRange()
    bool* inProgress{nullptr};   // прапорець завершення

    // Допоміжні методи для зручності
    double attackSpeed()    const { return cfg->attackSpeed; }
    double acceleration()   const { return cfg->acceleration; }
    double angularSpeed()   const { return cfg->angularSpeed; }
    double simTimeStep()    const { return cfg->simTimeStep; }
    double turnThreshold()  const { return cfg->turnThreshold; }
    double hitRadius()      const { return cfg->hitRadius; }
    double accelPath()      const { return cfg->accelPath; }
};