#pragma once

#include "Material.hpp"
#include "Resource.hpp"
#include "ResourceHandle.hpp"
#include "pch.hpp"
#include "types/Render.hpp"

#include <assimp/scene.h>

namespace Galaxy {
struct SubMesh {
    std::vector<Vertex> vertices;
    std::vector<unsigned short> indices;
    bool hasMaterial = false;
    ResourceHandle<Material> material;
    renderID visualID = 0;
};

class Mesh : public ResourceBase {
public:
    bool save(bool recursive = true) override;
    bool load(YAML::Node& data) override;

    bool loadExtern(const std::string& file);

    inline SubMesh& getSubMesh(int surface = 0) { return m_subMeshes[surface]; }
    inline const std::vector<Vertex>& getVertices(int surface = 0) const { return m_subMeshes[surface].vertices; }
    inline const std::vector<unsigned short>& getIndices(int surface = 0) const { return m_subMeshes[surface].indices; }

    inline bool hasMaterial(int surface = 0) const { return m_subMeshes[surface].hasMaterial; }
    inline const ResourceHandle<Material> getMaterial(int surface = 0) const { return m_subMeshes[surface].material; }

    inline const renderID getVisualID(int surface = 0) const { return m_subMeshes[surface].visualID; }

    inline void notifyGpuInstanceDestroyed(int surfaceIdx) { m_subMeshes[surfaceIdx].visualID = 0; }
    inline int getSubMeshesCount() const { return m_subMeshes.size(); }

    inline std::string getExternalFilePath() { return m_gltfPath; }

private:
    friend class ResourceImporter;

    void extractSubMesh(const aiScene* scene, int surface = 0);
    std::string m_gltfPath;

    std::vector<SubMesh> m_subMeshes;
};
} // namespace Galaxy
