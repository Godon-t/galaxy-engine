#pragma once

#include "Image.hpp"
#include "Resource.hpp"
#include "ResourceHandle.hpp"

#include <array>
#include <string>

namespace Galaxy {
class CubemapResource : public ResourceBase {
public:
    inline static constexpr std::array<const char*, 6> FaceNames { "px", "nx", "py", "ny", "pz", "nz" };
    using FacePaths = std::array<std::string, 6>;
    using Faces = std::array<ResourceHandle<Image>, 6>;

    CubemapResource() = default;
    CubemapResource(CubemapResource&& other) noexcept;

    bool load(YAML::Node& data) override;
    bool save(bool recursive = true) override;
    bool loadFaces(const FacePaths& paths);
    void removeFiles() const;

    const Faces& getFaces() const { return m_faces; }
    const FacePaths& getFacePaths() const { return m_facePaths; }
    int getResolution() const { return m_resolution; }

private:
    Faces m_faces;
    FacePaths m_facePaths;
    int m_resolution = 0;
};
} // namespace Galaxy
