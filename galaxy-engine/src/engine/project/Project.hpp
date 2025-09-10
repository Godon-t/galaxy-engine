#pragma once

#include "Scene.hpp"
#include "pch.hpp"

#include "UUID.hpp"

namespace Galaxy {
class Scene;

enum class ProjectPathTypes {
    SCENE,
    RESOURCE
};

class Project {
public:
    inline static bool load(const std::string& path) { return getInstance()._load(path); }
    inline static bool save() { return getInstance()._save(); }
    inline static std::string getPath(ProjectPathTypes type, const uuid& id) { return getInstance()._getPath(type, id); }
    inline static std::unordered_map<uuid, std::string>& getPaths(ProjectPathTypes type) { return getInstance()._getPaths(type); }
    inline static bool updatePath(ProjectPathTypes type, const uuid& id, const std::string& newPath) { return getInstance()._updatePath(type, id, newPath); }
    inline static bool doesPathExist(const std::string& path) { return getInstance().m_reversePathSearch.find(path) != getInstance().m_reversePathSearch.end(); }
    inline static uuid getPathId(const std::string& path) { return getInstance().m_reversePathSearch.find(path)->second; }

    inline static uuid registerNewPath(ProjectPathTypes type, const std::string& path) { return getInstance()._registerNewPath(type, path); }
    inline static bool deletePath(ProjectPathTypes type, uuid pathId) { return getInstance()._deletePath(type, pathId); }
    inline static void savePaths() { getInstance()._savePaths(); }

    static void extractExtension(const std::string& input, std::string& filePath, std::string& fileExtension);

    inline static void setName(const std::string& name) { getInstance().m_name = name; }

    static void create(const std::string& path);

    inline static Scene& loadScene(uuid id) { return getInstance()._loadScene(getInstance().getPath(ProjectPathTypes::SCENE, id), id); }
    inline static Scene& createScene(std::string path) { return getInstance()._createScene(path); }
    inline static void saveScene(uuid id) { getInstance()._saveScene(id); }
    static bool isSceneValid(uuid id);

    inline static std::string getProjectRootPath() { return getInstance().m_projectFolderPath; }
    inline static std::string toRelativePath(const std::string& absoluteFile) { return getInstance()._toRelativePath(absoluteFile); }

private:
    static Project& getInstance();

    uuid _registerNewPath(ProjectPathTypes type, const std::string& path);
    bool _deletePath(ProjectPathTypes type, uuid pathId);
    void _savePaths();

    std::string _getPath(ProjectPathTypes type, const uuid&);
    bool _updatePath(ProjectPathTypes type, const uuid&, const std::string& newPath);
    bool _load(const std::string& path);

    Scene& _loadScene(std::string path, uuid id);
    Scene& _createScene(std::string path);
    void _saveScene(uuid id);

    std::unordered_map<uuid, std::string>& _getPaths(ProjectPathTypes type);

    inline std::string getSceneFullPath(std::string& scenePath) { return m_projectFolderPath + scenePath; }
    std::string _toRelativePath(const std::string& absoluteFile);

    bool _save();

    std::string m_name;
    std::string m_projectFolderPath;
    std::string m_projectPath;
    std::unordered_map<
        ProjectPathTypes,
        std::unordered_map<uuid, std::string>>
        m_paths;
    std::unordered_map<std::string, uuid> m_reversePathSearch;

    std::unordered_map<uuid, Scene> m_scenes;
};
}