#pragma once

#include <string>

#include "interfaces/IConfigLoader.h"

class FileConfigLoader : public IConfigLoader {

public:
    FileConfigLoader(const std::string& path);

    bool load() override;

    const DroneConfig& getConfig() const override;

    const AmmoParams& getAmmoParams() const override;

    virtual ~FileConfigLoader();

private:
    bool readAmmoInfo();
    bool readConfig();

    DroneConfig* droneConfig;

    AmmoParams* ammoParamsList = nullptr;
    std::string ammoPath;
    int size = 0;
    int index = -1;

};