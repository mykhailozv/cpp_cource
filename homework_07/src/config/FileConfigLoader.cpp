#include <fstream>
#include <iostream>

#include "third_party/nlohmann/json.hpp"

#include "config/FileConfigLoader.h"
#include "Types.h"

using json = nlohmann::json;

FileConfigLoader::FileConfigLoader(const std::string& path)
    :ammoPath(path)
{
    // TODO: implement
}

bool FileConfigLoader::load()
{
    // TODO: implement
    return false;
}

const DroneConfig& FileConfigLoader::getConfig() const
{
    // TODO: implement
    return *droneConfig;
}

const AmmoParams& FileConfigLoader::getAmmoParams() const
{
    if (size > 0 && index >= 0 && index < size) {
        return ammoParamsList[index];
    }
    static AmmoParams dummy{};
    return dummy;
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
    std::ifstream readData("config.json");

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
    
    return true;
}