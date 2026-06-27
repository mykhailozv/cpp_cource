#pragma once

#include <string>

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

    AmmoParams* ammoParamsList = nullptr;
    std::string ammoPath;
    std::string configPath;
    int size = 0;
    int index = -1;

};