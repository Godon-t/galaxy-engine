#pragma once

#include "Resource.hpp"
#include "engine/types/Render.hpp"
#include "pch.hpp"

namespace Galaxy {
class Mesh : public ResourceBase {
public:
    bool load(const std::string& file) override;

    inline const std::vector<Vertex>& getVertices() const { return m_vertices; }
    inline const std::vector<unsigned short>& getIndices() const { return m_indices; }

private:
    bool readGltf(const std::string& filePath);
    std::string m_gltfPath;

    std::vector<Vertex> m_vertices;
    std::vector<unsigned short> m_indices;
};
} // namespace Galaxy
