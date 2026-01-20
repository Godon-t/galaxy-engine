#include "Project.hpp"

#include "engine/nodes/visitors/Deserializer.hpp"
#include "engine/nodes/visitors/Serializer.hpp"
#include "pch.hpp"

#include <filesystem>
#include <fstream>
#include <yaml-cpp/yaml.h>

namespace Galaxy {
constexpr ProjectPathTypes ProjectPathTypesArray[] = {
    ProjectPathTypes::SCENE,
    ProjectPathTypes::RESOURCE
};
inline const char* toString(ProjectPathTypes type)
{
    switch (type) {
    case ProjectPathTypes::SCENE:
        return "Scenes";
    case ProjectPathTypes::RESOURCE:
        return "Resources";
    default:
        return "";
    }
}

Project& Project::getInstance()
{
    static Project instance;
    return instance;
}

uuid Project::_registerNewPath(ProjectPathTypes type, const std::string& path)
{
    uuid pathId;
    m_paths[type][pathId]     = path;
    m_reversePathSearch[path] = pathId;
    return pathId;
}

bool Project::_deletePath(ProjectPathTypes type, uuid pathId)
{
    auto pathStr = m_paths[type][pathId];
    m_reversePathSearch.erase(pathStr);
    m_paths[type].erase(pathId);
    return true;
}

void Project::_savePaths()
{
    YAML::Emitter yaml;
    yaml << YAML::BeginMap;
    yaml << YAML::Key << "Name" << YAML::Value << "UI";

    for (const auto& [type, assoc] : m_paths) {
        std::string typeStr = toString(type);

        yaml << YAML::Key << typeStr << YAML::Value << YAML::BeginSeq;
        for (auto& value : m_paths[type]) {
            yaml << YAML::BeginMap;
            yaml << YAML::Key << "Id" << YAML::Value << value.first;
            yaml << YAML::Key << "Path" << YAML::Value << value.second;
            yaml << YAML::EndMap;
        }
        yaml << YAML::EndSeq;
    }
    yaml << YAML::EndMap;
    std::ofstream fout(m_projectPath);
    fout << yaml.c_str();
}

std::string Project::_getPath(ProjectPathTypes type, const uuid& id)
{
    return m_paths[type][id];
}

bool Project::_updatePath(ProjectPathTypes type, const uuid& id, const std::string& newPath)
{
    if (m_paths[type].find(id) != m_paths[type].end()) {
        m_reversePathSearch.erase(m_paths[type][id]);
        m_paths[type][id]            = newPath;
        m_reversePathSearch[newPath] = id;
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

    for (auto& type : ProjectPathTypesArray) {
        std::string typeStr = toString(type);

        for (auto pathData : data[typeStr.c_str()]) {
            uuid pathId(pathData["Id"].as<uint64_t>());
            std::string pathStr = pathData["Path"].as<std::string>();

            m_paths[type][pathId]        = pathStr;
            m_reversePathSearch[pathStr] = pathId;
        }
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
    uuid sceneId          = _registerNewPath(ProjectPathTypes::SCENE, path);

    m_scenes[sceneId] = Scene();
    m_scenes[sceneId].setUuid(sceneId);
    // TODO: offer a node selection for the first node
    m_scenes[sceneId].setNodePtr(std::make_shared<Node>());

    m_paths[ProjectPathTypes::SCENE][sceneId] = scenePath;
    m_reversePathSearch[scenePath]            = sceneId;

    serializer.serialize(m_scenes[sceneId], fullPath.c_str());
    return m_scenes[sceneId];
}

void Project::_saveScene(uuid id)
{
    SceneSerializer serializer;
    serializer.serialize(m_scenes[id], getSceneFullPath(m_paths[ProjectPathTypes::SCENE][id]).c_str());
}

std::unordered_map<uuid, std::string>& Project::_getPaths(ProjectPathTypes type)
{
    return m_paths[type];
}

std::string Project::_toRelativePath(const std::string& absoluteFile)
{
    std::filesystem::path filePath(absoluteFile);
    std::filesystem::path rootPath(getProjectRootPath());

    return std::filesystem::relative(filePath, rootPath).string();
}

bool Project::_save()
{
    _savePaths();

    for (auto& scene : m_scenes) {
        if (scene.second.getNodePtr()) {
            _saveScene(scene.first);
        }
    }

    return true;
}

void Project::extractExtension(const std::string& input, std::string& filePath, std::string& fileExtension)
{
    std::filesystem::path path(input.c_str());

    fileExtension = path.extension().string();
    filePath      = path.stem().string();
}

std::string Project::getFolderPath(const std::string& filePath)
{
    std::filesystem::path path(filePath.c_str());
    return path.parent_path().string();
}

void Project::create(const std::string& path)
{
    getInstance().m_projectPath = path;
    save();
}
bool Project::isSceneValid(uuid id)
{
    auto& paths = getInstance().m_paths[ProjectPathTypes::SCENE];
    return paths.find(id) != paths.end();
}
} // namespace Galaxy
