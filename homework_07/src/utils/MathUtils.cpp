#include "utils/MathUtils.h"
#include <cmath>


double MathUtils::calculateDirectionDelta(double dronDir, double targetDir) {
    double delta = targetDir - dronDir;
    
    // Нормалізуємо кут до діапазону [-π, π]
    while (delta > M_PI) {
        delta -= 2 * M_PI;
    }
    while (delta < -M_PI) {
        delta += 2 * M_PI;
    }
    
    return delta;
}

bool MathUtils::needStopForRotation(double currentDir, double neededDir, double turnThreshold) {
    double delta = calculateDirectionDelta(currentDir, neededDir);
    double absDelta = std::fabs(delta);
    return absDelta > turnThreshold;
}

double MathUtils::calculateRotationTime(double dronDir, double neededDir, double dronVelocity, double turnThreshold, double acceleration, double angularSpeed) {
    double absDelta = std::fabs(calculateDirectionDelta(dronDir, neededDir));
 
    if (absDelta < turnThreshold) {
        return 0.0;
    }
    
    double resultTime{0};
    if (dronVelocity > EPS)
    {
        // add TimeToStop
        resultTime += dronVelocity / acceleration;
    }
    
    resultTime += absDelta / angularSpeed;
    return resultTime;
}

double MathUtils::calculateAccelerationPath(double startVelocity, double stopVelocity, double acceleration){
    if (std::abs(acceleration) < EPS) {
        return 0.0; // exit(1)?
    }

    return std::fabs((stopVelocity * stopVelocity - startVelocity * startVelocity) 
           / (2.0 * acceleration));
}

double MathUtils::calculateBrakingDistance(double velocity, double acceleration) {
    if (std::abs(acceleration) < EPS) {
        return 0.0;
    }
    
    return (velocity * velocity) / (2.0 * acceleration);
}

double MathUtils::calculateAcceleration(double accelerationPath, double attackSpeed){
    return attackSpeed * attackSpeed / (2 * accelerationPath);
}

// Solves the time-optimal motion profile problem
void MathUtils::solveStopAtEnd(double v0, double maxV, double a, double d, double& time, double& vEnd) {
    // acceleration from v0 to max
    double s_acc = (maxV * maxV - v0 * v0) / (2.0 * a);

    // decceleration from max to 0
    double s_brake = (maxV * maxV) / (2.0 * a);

    double s_min = (v0 * v0) / (2.0 * a);

    if (d <= s_min) {
        vEnd = std::sqrt(v0 * v0 - 2.0 * a * d);
        time = (v0 - vEnd) / a;

        return;
    }

    //acc, line, dec
    if (s_acc + s_brake <= d) {
        double t_acc = (maxV - v0) / a;
        double t_brake = maxV / a;

        double s_cruise = d - s_acc - s_brake;
        double t_cruise = s_cruise / maxV;

        time = t_acc + t_cruise + t_brake;
        vEnd = 0;
    } else {
        // acc, dec (without max)
        double v_peak_sq = (v0 * v0) / 2.0 + a * d;
        double v_peak = std::sqrt(v_peak_sq);

        double t_acc = (v_peak - v0) / a;
        double t_brake = v_peak / a;

        time = t_acc + t_brake;
        vEnd = 0;
    }
}
