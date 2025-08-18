#pragma once

#include "Scene.hpp"
#include "pch.hpp"

#include "uuid.hpp"

namespace Galaxy {
class Scene;

class Project {
public:
    inline static bool load(const std::string& path) { return getInstance()._load(path); }
    inline static bool save() { return getInstance()._save(); }
    inline static std::string getPath(const uuid& id) { return getInstance()._getPath(id); }
    inline static std::unordered_map<uuid, std::string>& getPaths() { return getInstance().m_paths; }
    inline static bool updatePath(const uuid& id, const std::string& newPath) { return getInstance()._updatePath(id, newPath); }

    inline static void setName(const std::string& name) { getInstance().m_name = name; }

    static void create(const std::string& path);

    inline static Scene& loadScene(uuid id) { return getInstance()._loadScene(getInstance().getPath(id), id); }
    inline static Scene& createScene(std::string path) { return getInstance()._createScene(path); }
    inline static void saveScene(uuid id) { getInstance()._saveScene(id); }
    inline static bool isSceneValid(uuid id) { return getInstance().m_paths.find(id) != getInstance().m_paths.end(); }

private:
    static Project& getInstance();

    uuid _registerNewPath(const std::string& path);
    std::string _getPath(const uuid&);
    bool _updatePath(const uuid&, const std::string& newPath);
    bool _load(const std::string& path);

    Scene& _loadScene(std::string path, uuid id);
    Scene& _createScene(std::string path);
    void _saveScene(uuid id);

    inline std::string getSceneFullPath(std::string& scenePath) { return m_projectFolderPath + scenePath; }

    bool _save();

    UUIDv4::UUIDGenerator<std::mt19937_64> m_uuidGenerator;
    std::string m_name;
    std::string m_projectFolderPath;
    std::string m_projectPath;
    std::unordered_map<uuid, std::string> m_paths;

    std::unordered_map<uuid, Scene> m_scenes;
};
}