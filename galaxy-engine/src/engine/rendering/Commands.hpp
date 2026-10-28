#pragma once

#include "rendering/renderer/resources/GpuResourceHandle.hpp"

#include <cstddef>
#include <cstring>
#include <variant>
#include <vector>

namespace Galaxy {

using RenderTargetHandle = std::variant<FramebufferHandle, CubemapFramebufferHandle>;

struct UpdateUBOCommand {
    BufferHandle ubo;
    std::vector<std::byte> data;

    template<typename T>
    static UpdateUBOCommand make(BufferHandle handle, const T& payload)
    {
        UpdateUBOCommand command;
        command.ubo = handle;
        command.data.resize(sizeof(T));
        std::memcpy(command.data.data(), &payload, sizeof(T));
        return command;
    }
};

}
