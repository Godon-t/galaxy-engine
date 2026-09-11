#pragma once

#include "Material.hpp"
#include "Resource.hpp"
#include "ResourceHandle.hpp"
#include "rendering/renderer/resources/GpuResourceHandle.hpp"
#include "pch.hpp"
#include "types/Math.hpp"
#include "types/Render.hpp"

#include <assimp/scene.h>

using namespace math;

namespace Galaxy {
struct SubMeshTree {
    std::vector<SubMeshTree> childs;
    vec3 translation;
    quat rotation;
    vec3 scale;
    std::vector<int> subMeshes;
};

struct SubMesh {
    std::vector<Vertex> vertices;
    std::vector<unsigned short> indices;
    bool hasMaterial = false;
    ResourceHandle<Material> material;
    GeometryHandle geometry;
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

    inline GeometryHandle getGpuGeometryHandle(int surface = 0) const { return m_subMeshes[surface].geometry; }

    inline void setGpuGeometryHandle(int surfaceIdx, GeometryHandle geometry) { m_subMeshes[surfaceIdx].geometry = geometry; }
    inline void notifyGpuInstanceDestroyed(int surfaceIdx, GeometryHandle expectedHandle)
    {
        if (m_subMeshes[surfaceIdx].geometry == expectedHandle)
            m_subMeshes[surfaceIdx].geometry = {};
    }
    inline int getSubMeshesCount() const { return m_subMeshes.size(); }

    inline std::string getExternalFilePath() { return m_gltfPath; }

    SubMeshTree& getMeshTree(int surfaceIdx);
    SubMeshTree& getRootTree();

private:
    friend class ResourceImporter;

    void extractSubMesh(const aiScene* scene, int surface = 0);
    SubMeshTree buildTree(const aiNode* node);
    SubMeshTree* findCorrespondingTree(SubMeshTree* currentTree, int surfaceIdx);
    std::string m_gltfPath;

    std::vector<SubMesh> m_subMeshes;
    SubMeshTree m_subMeshTree;
};
} // namespace Galaxy
