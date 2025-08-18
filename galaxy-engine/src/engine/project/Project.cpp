#include "Project.hpp"

#include "engine/nodes/visitors/Deserializer.hpp"
#include "engine/nodes/visitors/Serializer.hpp"
#include "pch.hpp"

#include <filesystem>
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
    namespace fs        = std::filesystem;
    std::string folder  = fs::path(path).parent_path().string();
    m_projectPath       = path;
    m_projectFolderPath = folder == "" ? "" : folder + "/";

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
        uuid sceneId          = uuidFromString(scene["Id"].as<std::string>());
        std::string scenePath = scene["Path"].as<std::string>();

        m_paths[sceneId] = scenePath;
    }

    return true;
}

Scene& Project::_loadScene(std::string path, uuid id)
{
    if (m_scenes.find(id) == m_scenes.end()) {
        SceneDeSerializer deserializer;
        m_scenes[id]         = Scene();
        std::string fullPath = getSceneFullPath(path);
        deserializer.deserialize(m_scenes[id], fullPath.c_str());
    }
    return m_scenes[id];
}

Scene& Project::_createScene(std::string path)
{
    SceneSerializer serializer;
    std::string fullPath = getSceneFullPath(path);

    std::string scenePath = path;
    uuid sceneId          = _registerNewPath(path);

    m_scenes[sceneId] = Scene();
    m_scenes[sceneId].setUuid(sceneId);
    // TODO: offer a node selection for the first node
    m_scenes[sceneId].setNodePtr(std::make_shared<Node>());

    m_paths[sceneId] = scenePath;

    serializer.serialize(m_scenes[sceneId], fullPath.c_str());
    return m_scenes[sceneId];
}

void Project::_saveScene(uuid id)
{
    SceneSerializer serializer;
    serializer.serialize(m_scenes[id], getSceneFullPath(m_paths[id]).c_str());
}

bool Project::_save()
{
    YAML::Emitter yaml;
    yaml << YAML::BeginMap;
    yaml << YAML::Key << "Name" << YAML::Value << "UI";

    yaml << YAML::Key << "Scenes" << YAML::Value << YAML::BeginSeq;
    for (auto& value : m_paths) {
        yaml << YAML::BeginMap;
        yaml << YAML::Key << "Id" << YAML::Value << uuidToString(value.first);
        yaml << YAML::Key << "Path" << YAML::Value << value.second;
        yaml << YAML::EndMap;
    }
    yaml << YAML::EndSeq;
    yaml << YAML::EndMap;

    for (auto& scene : m_scenes) {
        if (scene.second.getNodePtr()) {
            _saveScene(scene.first);
        }
    }

    std::ofstream fout(m_projectPath);
    fout << yaml.c_str();
    return true;
}

void Project::create(const std::string& path)
{
    getInstance().m_projectPath = path;
    save();
}
} // namespace Galaxy
