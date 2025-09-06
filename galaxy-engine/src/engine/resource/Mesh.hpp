#pragma once

#include "Resource.hpp"
#include "pch.hpp"
#include "types/Render.hpp"

#include <assimp/scene.h>

namespace Galaxy {
struct SubMesh {
    std::vector<Vertex> vertices;
    std::vector<unsigned short> indices;
    renderID visualID = 0;
};

class Mesh : public ResourceBase {
public:
    bool load(const std::string& file) override;
    bool load(const unsigned char* data, size_t size) override;

    inline const std::vector<Vertex>& getVertices(int surface = 0) const { return m_subMeshes[surface].vertices; }
    inline const std::vector<unsigned short>& getIndices(int surface = 0) const { return m_subMeshes[surface].indices; }
    inline const renderID getVisualID(int surface = 0) const
    {
        return m_subMeshes[surface].visualID;
    }

    inline void notifyGpuInstanceDestroyed(int surfaceIdx) { m_subMeshes[surfaceIdx].visualID = 0; }
    inline int getSubMeshesCount() const { return m_subMeshes.size(); }

private:
    bool readGltf(const std::string& filePath);
    void extractSubMesh(const aiScene* scene, int surface = 0);
    std::string m_gltfPath;

    std::vector<SubMesh> m_subMeshes;
};
} // namespace Galaxy
