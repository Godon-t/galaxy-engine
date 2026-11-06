#pragma once

#include "Resource.hpp"
#include "rendering/GPUInstances/GpuResourceHandle.hpp"
#include "types/Render.hpp"

#include <vector>

namespace Galaxy {
struct Image : public ResourceBase {
    Image() = default;

    Image(int width, int height, int nbChannels);

    bool load(YAML::Node& data) override;
    bool save(bool recursive = true) override;
    bool save(const std::string& path);
    bool save(const char* path) { return path != nullptr && save(std::string(path)); }

    bool loadExtern(const std::string& path);
    bool loadIntern(const float* data);

    inline int getWidth() const { return m_width; }
    inline int getHeight() const { return m_height; }
    inline int getNbChannels() const { return m_nbChannels; }
    float* getData();

    inline void notifyGpuInstanceDestroyed(TextureHandle expectedHandle)
    {
        if (m_textureHandle == expectedHandle)
            m_textureHandle = {};
    }
    inline TextureHandle getGpuTextureHandle() const { return m_textureHandle; }
    inline void setGpuTextureHandle(TextureHandle handle) { m_textureHandle = handle; }

    inline std::string getExternalFilePath() { return m_relativeExternalFilePath; }
    bool hasTransparency();

    void destroy();

    // Free space in ram
    void freeCpuData();

private:
    std::size_t componentCount() const;
    bool ensureCpuData();

    int m_width = 0;
    int m_height = 0;
    int m_nbChannels = 0;
    std::vector<float> m_data;

    std::string m_relativeExternalFilePath;

    TextureHandle m_textureHandle;
};
}
