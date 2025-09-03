#pragma once

#include "Resource.hpp"
#include "engine/types/Render.hpp"
#include "pch.hpp"

#include <assimp/scene.h>

namespace Galaxy {
struct SubMesh {
    std::vector<Vertex> m_vertices;
    std::vector<unsigned short> m_indices;
};

class Mesh : public ResourceBase {
public:
    bool load(const std::string& file) override;

    inline const std::vector<Vertex>& getVertices(int surface = 0) const { return m_subMeshes[surface].m_vertices; }
    inline const std::vector<unsigned short>& getIndices(int surface = 0) const { return m_subMeshes[surface].m_indices; }
    inline int getSubMeshesCount() const { return m_subMeshes.size(); }

private:
    bool readGltf(const std::string& filePath);
    void extractSubMesh(const aiScene* scene, int surface = 0);
    std::string m_gltfPath;

    std::vector<SubMesh> m_subMeshes;
};
} // namespace Galaxy
