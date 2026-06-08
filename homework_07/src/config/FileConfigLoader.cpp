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
    :ammoPath(ammoPath), configPath(configPath), cacheValid(false)
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
    if (cacheValid) {
        return &cachedAmmo->second;
    }

    ERROR("Ammo cache is not initialized");
    static AmmoParams dummy{};
    return &dummy;
}

FileConfigLoader::~FileConfigLoader()
{
    delete droneConfig;
}


NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Coord, x, y)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(AmmoParams, name, mass, drag, lift)

bool FileConfigLoader::readAmmoInfo(){
    std::ifstream readData(ammoPath);

    if (!readData.is_open())
    {
        ERROR("Failed to open " << ammoPath << " file");
        return false;
    }

    json data;

    try {
        data = json::parse(readData);
    } catch (const json::parse_error& e) {
        ERROR("JSON parse error: " << e.what());
        return false;
    }

    int dataSize = data.size();

    if (dataSize <= 0)
    {
        return false;
    }

    try {
        std::vector<AmmoParams> ammoParamsList = data.get<std::vector<AmmoParams>>();
        ammoParamsMap.clear();
        for (const auto& ammo : ammoParamsList)
        {
            ammoParamsMap.emplace(ammo.name, ammo);
        }
    } catch (const json::exception& e) {
        ERROR("Ammo parsing error: " << e.what());
        return false;
    }

    return true;
    
}

bool FileConfigLoader::readConfig(){
    std::ifstream readData(configPath);

    if (!readData.is_open())
    {
        ERROR("Failed to open config.json file");
        return false;
    }
    
    delete droneConfig;
    droneConfig = new DroneConfig();

    json data;

    try {
        data = json::parse(readData);
    } catch (const json::parse_error& e) {
        ERROR("JSON parse error: " << e.what());
        return false;
    }

    if (!data.contains("drone")){
        ERROR("Missing key: drone");
        return false;
    }

    const auto& drone = data["drone"];

    if (!drone.contains("position")) {
        ERROR("Missing key: drone.position");
        return false;
    }

    const auto& pos = drone["position"];

    if (!pos.contains("x")) {
        ERROR("Missing key: drone.position.x");
        return false;
    }

    if (!pos.contains("y")) {
        ERROR("Missing key: drone.position.y");
        return false;
    }

    if (!drone.contains("accelerationPath")) {
        ERROR("Missing key: drone.accelerationPath");
        return false;
    }

    if (!drone.contains("altitude")) {
        ERROR("Missing key: drone.altitude");
        return false;
    }

    if (!drone.contains("angularSpeed")) {
        ERROR("Missing key: drone.angularSpeed");
        return false;
    }

    if (!drone.contains("attackSpeed")) {
        ERROR("Missing key: drone.attackSpeed");
        return false;
    }

    if (!drone.contains("initialDirection")) {
        ERROR("Missing key: drone.initialDirection");
        return false;
    }

    if (!drone.contains("turnThreshold")) {
        ERROR("Missing key: drone.turnThreshold");
        return false;
    }

    if (!data.contains("ammo")) {
        ERROR("Missing key: ammo");
        return false;
    }

    if (!data.contains("targetArrayTimeStep")) {
        ERROR("Missing key: targetArrayTimeStep");
        return false;
    }

    if (!data.contains("simulation")) {
        ERROR("Missing key: simulation");
        return false;
    }

    const auto& sim = data["simulation"];

    if (!sim.contains("hitRadius")) {
        ERROR("Missing key: simulation.hitRadius");
        return false;
    }

    if (!sim.contains("timeStep")) {
        ERROR("Missing key: simulation.timeStep");
        return false;
    }


    droneConfig->accelPath = drone["accelerationPath"];
    droneConfig->altitude = drone["altitude"];
    droneConfig->ammoName = data["ammo"];
    droneConfig->angularSpeed = drone["angularSpeed"];
    droneConfig->arrayTimeStep = data["targetArrayTimeStep"];
    droneConfig->attackSpeed = drone["attackSpeed"];
    droneConfig->hitRadius = sim["hitRadius"];
    droneConfig->initialDir = drone["initialDirection"];
    droneConfig->simTimeStep = sim["timeStep"];
    droneConfig->startPos.x = pos["x"];
    droneConfig->startPos.y = pos["y"];
    droneConfig->turnThreshold = drone["turnThreshold"];

    droneConfig->acceleration = MathUtils::calculateAcceleration(droneConfig->accelPath, droneConfig->attackSpeed);
    
    return true;
}

bool FileConfigLoader::setAmmoName(const std::string& ammoName){
    
    if (ammoParamsMap.empty()) {
        ERROR("ammoParamsMap is not initialized");
        return false;
    }

    auto it = ammoParamsMap.find(ammoName);
    if (it == ammoParamsMap.end()){
        ERROR("Unknown ammoName: " << ammoName);
        return false;
    }

    cachedAmmo = it;
    cacheValid = true;
    
    return true;
    
}