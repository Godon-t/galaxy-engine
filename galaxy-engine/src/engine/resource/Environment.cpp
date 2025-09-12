#include "Environment.hpp"

#include "ResourceManager.hpp"

bool Galaxy::Environment::load(YAML::Node& data)
{
    if (!data["Type"] || data["Type"].as<std::string>() != std::string("Environment")) {
        GLX_CORE_ERROR("File unsupported");
        return false;
    }

    if (data["Skybox"]) {
        auto skyboxIDs = data["Skybox"];
        for (int i = 0; i < 6; i++) {
            uuid imageID = skyboxIDs[i].as<uint64_t>();
            m_skybox[i]  = ResourceManager::getInstance().load<Image>(Project::getPath(ProjectPathTypes::RESOURCE, imageID));
        }
    }

    return true;
}