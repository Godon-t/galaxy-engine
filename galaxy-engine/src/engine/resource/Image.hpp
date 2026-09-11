#pragma once

#include "Resource.hpp"
#include "rendering/renderer/resources/GpuResourceHandle.hpp"
#include "types/Render.hpp"

namespace Galaxy {
struct Image : public ResourceBase {
    Image() {};

    Image(int width, int height, int nbChannels);

    bool load(YAML::Node& data) override;
    bool save(bool recursive = true) override;

    bool loadExtern(const std::string& path);

    inline int getWidth() const { return m_width; }
    inline int getHeight() const { return m_height; }
    inline int getNbChannels() const { return m_nbChannels; }
    unsigned char* getData();

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
    int m_width;
    int m_height;
    unsigned char* m_data;
    int m_nbChannels;

    std::string m_relativeExternalFilePath;

    TextureHandle m_textureHandle;

    bool m_freed = true;
};
}
