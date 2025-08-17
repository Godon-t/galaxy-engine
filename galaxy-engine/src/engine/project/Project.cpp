#include "Project.hpp"

#include "engine/nodes/visitors/Deserializer.hpp"
#include "engine/nodes/visitors/Serializer.hpp"
#include "pch.hpp"

#include <fstream>
#include <yaml-cpp/yaml.h>

namespace Galaxy {
Project& Project::getInstance()
{
    static Project instance;
    return instance;
}

uuid Project::_registerNewPath(const std::string& path)
{
    uuid pathId     = m_uuidGenerator.getUUID();
    m_paths[pathId] = path;
    return pathId;
}

std::string Project::_getPath(const uuid& id)
{
    return m_paths[id];
}

bool Project::_updatePath(const uuid& id, const std::string& newPath)
{
    if (m_paths.find(id) != m_paths.end()) {
        m_paths[id] = newPath;
        return true;
    }

    return false;
}

bool Project::_load(const std::string& path)
{
    m_projectPath = path;

    std::ifstream stream(path.c_str());

    if (!stream)
        return false;

    std::stringstream strStream;
    strStream << stream.rdbuf();
    auto data = YAML::Load(strStream.str());

    if (!data["Name"])
        return false;

    m_name = data["Name"].as<std::string>();
    for (auto scene : data["Scenes"]) {
        uuid sceneId(scene["Id"].as<std::string>());
        std::string scenePath = scene["Path"].as<std::string>();

        m_paths[sceneId] = scenePath;
    }

    return true;
}

Scene& Project::_loadScene(std::string path, uuid id)
{
    if (m_scenes.find(id) == m_scenes.end()) {
        SceneDeSerializer deserializer;
        m_scenes[id] = Scene(path);
        deserializer.deserialize(m_scenes[id], path.c_str());
    }
    return m_scenes[id];
}

Scene& Project::_createScene(std::string path)
{
    SceneSerializer serializer;
    std::string fullPath = m_projectPath + path;

    std::string scenePath = path;
    uuid sceneId          = _registerNewPath(path);

    m_scenes[sceneId] = Scene(scenePath);
    m_scenes[sceneId].setUuid(sceneId);

    serializer.serialize(m_scenes[sceneId], fullPath.c_str());
    return m_scenes[sceneId];
}

bool Project::_save()
{
    std::ofstream fout(m_projectPath);
    YAML::Emitter yaml;
    yaml << YAML::BeginMap;
    yaml << YAML::Key << "Name" << YAML::Value << "UI";

    yaml << YAML::Key << "Scenes" << YAML::Value << YAML::BeginSeq;
    for (auto& value : m_paths) {
        yaml << YAML::BeginMap;
        yaml << YAML::Key << "Id" << YAML::Value << value.first.bytes();
        yaml << YAML::Key << "Path" << YAML::Value << value.second;
        yaml << YAML::EndMap;
    }
    yaml << YAML::EndSeq;
    yaml << YAML::EndMap;

    fout << yaml.c_str();
    return true;
}

void Project::create(const std::string& path)
{
    getInstance().m_projectPath = path;
    save();
    Scene& demo = createScene("demo.glx");
}
} // namespace Galaxy
