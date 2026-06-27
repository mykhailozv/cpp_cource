#pragma once

class MathUtils{
public:
    static constexpr double EPS = 0.001;
    static constexpr double GRAVITY = 9.81;

    static bool needStopForRotation(double currentDir, double neededDir, double turnThreshold);
    static double calculateDirectionDelta(double dronDir, double targetDir);
    static double calculateRotationTime(double dronDir, double neededDir, double dronVelocity, double turnThreshold, double acceleration, double angularSpeed);
    static double calculateAccelerationPath(double startVelocity, double stopVelocity, double acceleration);
    static double calculateBrakingDistance(double velocity, double acceleration);
    static double calculateAcceleration(double accelerationPath, double attackSpeed);
    static void solveStopAtEnd(double v0, double maxV, double a, double d, double& time, double& vEnd);
};