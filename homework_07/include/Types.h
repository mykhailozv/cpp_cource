#pragma once

#include <string>


enum DronePhase {
    STOPPED,
    ACCELERATING,
    DECELERATING,
    TURNING,
    MOVING
};

struct Coord {
	double x;
	double y;

	Coord operator+(const Coord& other) const;
	Coord operator-(const Coord& other) const;
	Coord operator*(double s) const;
	Coord operator/(double s) const;
	bool operator==(const Coord& other) const;
	double length() const;
	Coord getNorm() const;
	double distanceTo(const Coord& other) const;
	double direction() const;
};

struct AmmoParams {
	std::string name;
	float mass; 	// маса (кг)
	float drag; 	// коефіцієнт опору
	float lift; 	// коефіцієнт підйому
};

struct DroneConfig {
	Coord startPos;     	// початкова позиція (x, y)
	double altitude;     	// висота
	double initialDir;   	// початковий напрямок (рад)
	double attackSpeed;  	// швидкість атаки (м/с)
	double accelPath;    	// шлях розгону (м)
    std::string ammoName;
	double arrayTimeStep;	// крок часу масиву цілей
	double simTimeStep;  	// крок симуляції
	double hitRadius;    	// радіус влучення
	double angularSpeed; 	// кутова швидкість (рад/с)
	double turnThreshold;	// поріг повороту (рад)
    double acceleration;    // прискорення дрона за модулем
};

struct Target{
	int targetId;
	int timeSteps;
	Coord* targets = nullptr;

	//Target(const Target&) = delete;
    Target& operator=(const Target&) = delete;

	void removeData();
	void init(int targetId, int count);
	~Target();
};


struct StepTimer{
    int stepIndex{0}; // steps - загальний крок симуляції
    int timeIndex{0}; // timeStep - індекс у масиві переміщення target
    int nexTimeIndex{1};
    int timeCount{1}; // STEPS - кількість time steps у масиві

    int tickStep{0};     // крок всередині одного timeIndex
    int stepsPerTime{0}; // arrayUnitSteps скільки sim-кроків у одному array кроці

    double simTimeStep{0.0};
    double tickTime{0.0};
	bool isStepReady{false};
	int stepSize{0};

    void init(const DroneConfig& droneConfig, int timeSteps);

    void update(int newStep);
    void step();
	bool reset();

    bool operator<(int value) const;

    StepTimer& operator+=(int delta);
};

struct SimStep {
	Coord pos;          	// позиція дрона
	double direction;    	// напрямок (рад)
    Coord directionVector;  // dx, dy
	int   state;        	// стан автомата (0-4)
	int   targetIdx;    	// індекс поточної цілі
    double velocity;
	Coord dropPoint;    	// точка скиду (куди летить дрон)
	Coord aimPoint;     	// куди впаде бомба (якщо скинути зараз)
	Coord predictedTarget;  // прогнозована позиція цілі

    void upDirection(double newDirection);
};
