#include "simulation/Target.h"

void Target::init(int targetId, int count){
    this->targetId = targetId;
    timeSteps = count;

    targets.resize(count);
}

Target::~Target(){}
