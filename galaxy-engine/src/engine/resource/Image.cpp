#include "Image.hpp"

#include "Core.hpp"
#include "Log.hpp"
#include "ResourceSerializer.hpp"
#include "project/Project.hpp"

#include <algorithm>
#include <cmath>
#include <memory>

#define STB_IMAGE_IMPLEMENTATION
#include "stbi.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize2.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stbi_image_write.h"

namespace Galaxy {
unsigned char normalizedByte(float value)
{
    if (std::isnan(value) || value <= 0.0f)
        return 0;
    if (value >= 1.0f)
        return 255;
    return static_cast<unsigned char>(value * 255.0f + 0.5f);
}

Image::Image(int width, int height, int nbChannels)
    : m_width(width)
    , m_height(height)
    , m_nbChannels(nbChannels)
{
}

bool Image::load(YAML::Node& data)
{
    if (!data["Type"] || data["Type"].as<std::string>() != std::string("Image")) {
        GLX_CORE_ERROR("File unsupported");
        return false;
    }

    if (data["ExternalFile"]) {
        return loadExtern(data["ExternalFile"].as<std::string>());
    }

    return false;
}

bool Image::save(bool recursive)
{
    return ResourceSerializer::serialize(*this);
}

bool Image::save(const std::string& path)
{
    const float* pixels = getData();
    if (pixels == nullptr || componentCount() == 0)
        return false;

    std::string extension = std::filesystem::path(path).extension().string();

    int result = 0;
    if (extension == ".hdr") {
        result = stbi_write_hdr(path.c_str(), m_width, m_height, m_nbChannels, pixels);
    } else if (extension == ".png" || extension.empty()) {
        // PNG encoding needs bytes, but the stored float pixels remain unchanged.
        std::vector<unsigned char> encodedPixels(m_data.size());
        std::transform(m_data.begin(), m_data.end(), encodedPixels.begin(), normalizedByte);
        result = stbi_write_png(path.c_str(), m_width, m_height, m_nbChannels,
            encodedPixels.data(), m_width * m_nbChannels);
    } else {
        GLX_CORE_ERROR("Unsupported image output format: '{0}'", path);
        return false;
    }

    if (result == 0) {
        GLX_CORE_ERROR("Failed to save image: '{0}'", path);
        return false;
    }
    return true;
}

void Image::destroy()
{
    freeCpuData();
}

void Image::freeCpuData()
{
    std::vector<float>().swap(m_data);
}

bool Image::loadExtern(const std::string& path)
{
    // Keep a copy: ensureCpuData may pass m_relativeExternalFilePath itself.
    const std::string externalPath = path;
    std::filesystem::path filePath(externalPath);
    if (filePath.is_relative() && !std::filesystem::exists(filePath))
        filePath = std::filesystem::path(Project::getProjectRootPath()) / filePath;
    const std::string filename = filePath.string();
    freeCpuData();
    m_relativeExternalFilePath = externalPath;
    m_isInternal = false;

    bool loaded = false;
    if (stbi_is_hdr(filename.c_str())) {
        std::unique_ptr<float, decltype(&stbi_image_free)> pixels(
            stbi_loadf(filename.c_str(), &m_width, &m_height, &m_nbChannels, 0), stbi_image_free);
        loaded = pixels != nullptr && componentCount() != 0;
        if (loaded)
            m_data.assign(pixels.get(), pixels.get() + componentCount());
    } else {
        std::unique_ptr<unsigned char, decltype(&stbi_image_free)> pixels(
            stbi_load(filename.c_str(), &m_width, &m_height, &m_nbChannels, 0), stbi_image_free);
        loaded = pixels != nullptr && componentCount() != 0;
        if (loaded) {
            // Normalize once at import; stbi_loadf would additionally apply an LDR gamma curve.
            m_data.resize(componentCount());
            std::transform(pixels.get(), pixels.get() + componentCount(), m_data.begin(),
                [](unsigned char value) { return value / 255.0f; });
        }
    }
    if (!loaded) {
        GLX_CORE_ERROR("Failed to load: '{0}'", externalPath);
        return false;
    }
    return true;
}

std::size_t Image::componentCount() const
{
    if (m_width <= 0 || m_height <= 0 || m_nbChannels < 1 || m_nbChannels > 4)
        return 0;
    const auto rowSize = static_cast<std::size_t>(m_width) * m_nbChannels;
    return rowSize * m_height;
}

bool Image::loadIntern(const float* data)
{
    const auto count = componentCount();
    if (data == nullptr || count == 0)
        return false;
    std::vector<float> pixels(data, data + count);
    freeCpuData();
    m_data = std::move(pixels);
    m_isInternal = true;
    m_relativeExternalFilePath.clear();
    return true;
}

bool Image::ensureCpuData()
{
    if (!m_data.empty())
        return true;
    if (m_relativeExternalFilePath.empty()) {
        GLX_CORE_ERROR("Image has no CPU pixels or external file to reload");
        return false;
    }
    return loadExtern(m_relativeExternalFilePath);
}

float* Image::getData()
{
    if (!ensureCpuData())
        return nullptr;
    return m_data.data();
}

bool Image::hasTransparency()
{
    if (m_nbChannels != 2 && m_nbChannels != 4)
        return false;
    const float* data = getData();
    if (data == nullptr)
        return false;
    for (std::size_t i = m_nbChannels - 1; i < componentCount(); i += m_nbChannels) {
        if (data[i] < 1.0f) {
            return true;
        }
    }
    return false;
}

}
