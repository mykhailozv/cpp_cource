#include <fstream>
#include <iostream>

#include "third_party/nlohmann/json.hpp"

#include "config/FileConfigLoader.h"
#include "Types.h"
#include "utils/Logging.h"
#include "utils/MathUtils.h"

using json = nlohmann::json;

constexpr double EPS = 0.001;

FileConfigLoader::FileConfigLoader(const std::string& ammoPath, const std::string& configPath)
    :ammoPath(ammoPath), configPath(configPath)
{
    // TODO: implement
}

bool FileConfigLoader::load()
{
    if (readAmmoInfo() && readConfig()) {
        if (setAmmoName(droneConfig->ammoName)) {
            return true;
        }
    }
    
    return false;
}

const DroneConfig* FileConfigLoader::getConfig() const
{
    return droneConfig;
}

const AmmoParams* FileConfigLoader::getAmmoParams() const
{
    if (size > 0 && index >= 0 && index < size) {
        return &ammoParamsList[index];
    }

    std::cerr << "ammoParamsList is not initialized\n";
    static AmmoParams dummy{};
    return &dummy;
}

FileConfigLoader::~FileConfigLoader()
{
    delete[] ammoParamsList;
    delete droneConfig;
}


NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Coord, x, y)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(AmmoParams, name, mass, drag, lift)

bool FileConfigLoader::readAmmoInfo(){
    std::ifstream readData(ammoPath);

    if (!readData.is_open())
    {
        std::cerr << "Failed to open " << ammoPath << " file\n";
        return false;
    }
    
    json data = json::parse(readData);
    int dataSize = data.size();

    if (dataSize <= 0)
    {
        return false;
    }

    delete[] ammoParamsList;    
    
    ammoParamsList = new AmmoParams[dataSize];

    for (int i = 0; i < dataSize; i++)
    {
        ammoParamsList[i] = data[i];
    }

    size = dataSize;

    return true;
    
}

bool FileConfigLoader::readConfig(){
    std::ifstream readData(configPath);

    if (!readData.is_open())
    {
        std::cerr << "Failed to open config.json file\n";
        return false;
    }
    
    json data = json::parse(readData);

    delete droneConfig;
    droneConfig = new DroneConfig();
    droneConfig->accelPath = data["drone"]["accelerationPath"];
    droneConfig->altitude = data["drone"]["altitude"];
    droneConfig->ammoName = data["ammo"];
    droneConfig->angularSpeed = data["drone"]["angularSpeed"];
    droneConfig->arrayTimeStep = data["targetArrayTimeStep"];
    droneConfig->attackSpeed = data["drone"]["attackSpeed"];
    droneConfig->hitRadius = data["simulation"]["hitRadius"];
    droneConfig->initialDir = data["drone"]["initialDirection"];
    droneConfig->simTimeStep = data["simulation"]["timeStep"];
    droneConfig->startPos.x = data["drone"]["position"]["x"];
    droneConfig->startPos.y = data["drone"]["position"]["y"];
    droneConfig->turnThreshold = data["drone"]["turnThreshold"];

    droneConfig->acceleration = MathUtils::calculateAcceleration(droneConfig->accelPath, droneConfig->attackSpeed);
    
    return true;
}

bool FileConfigLoader::setAmmoName(const std::string& ammoName){
    
    if (ammoParamsList == nullptr) {
        std::cerr << "ammoParamsList is not initialized\n";
        return false;
    }

    for (int i = 0; i < size; i++)
    {
        if (ammoName == ammoParamsList[i].name)
        {
            index = i;
            return true;
        }
        
    }

    std::cerr << "Unknown ammoName: " << ammoName;
    return false;
}