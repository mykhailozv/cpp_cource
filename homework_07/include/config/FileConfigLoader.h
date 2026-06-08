#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "interfaces/IConfigLoader.h"

class FileConfigLoader : public IConfigLoader {

public:
    FileConfigLoader(const std::string& ammoPath, const std::string& configPath);

    bool load() override;

    const DroneConfig* getConfig() const override;

    const AmmoParams* getAmmoParams() const override;

    virtual ~FileConfigLoader();

private:
    bool readAmmoInfo();
    bool readConfig();
    bool setAmmoName(const std::string& ammoName);

    DroneConfig* droneConfig;

    std::unordered_map<std::string, AmmoParams> ammoParamsMap;
    std::string ammoPath;
    std::string configPath;
    std::unordered_map<std::string, AmmoParams>::const_iterator cachedAmmo;
    bool cacheValid = false;
};