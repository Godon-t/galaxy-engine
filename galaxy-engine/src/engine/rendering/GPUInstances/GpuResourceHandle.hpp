#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>

namespace Galaxy {

template<typename Tag>
class GpuResourceHandle {
public:
    using Index = uint32_t;

    inline static const Index InvalidIndex = std::numeric_limits<Index>::max();

    GpuResourceHandle() noexcept = default;
    GpuResourceHandle(Index index, uint32_t generation) noexcept
        : m_index(index)
        , m_generation(generation)
    {
    }

    [[nodiscard]] Index index() const noexcept { return m_index; }
    [[nodiscard]] uint32_t generation() const noexcept { return m_generation; }
    // This only tells whether the handle contains an index/generation pair.
    // Liveness must be checked by the registry (Backend::isValid).
    [[nodiscard]] bool hasValue() const noexcept
    {
        return m_index != InvalidIndex && m_generation != 0;
    }

    explicit operator bool() const noexcept { return hasValue(); }

    friend bool operator==(GpuResourceHandle lhs, GpuResourceHandle rhs) noexcept
    {
        return lhs.m_index == rhs.m_index && lhs.m_generation == rhs.m_generation;
    }

    friend bool operator!=(GpuResourceHandle lhs, GpuResourceHandle rhs) noexcept
    {
        return !(lhs == rhs);
    }

private:
    Index m_index = InvalidIndex;
    uint32_t m_generation = 0;
};

struct ProgramResourceTag;
struct GeometryResourceTag;
struct TextureResourceTag;
struct CubemapResourceTag;
struct MaterialResourceTag;
struct FramebufferResourceTag;
struct CubemapFramebufferResourceTag;
struct BufferResourceTag;

using ProgramHandle = GpuResourceHandle<ProgramResourceTag>;
using GeometryHandle = GpuResourceHandle<GeometryResourceTag>;
using TextureHandle = GpuResourceHandle<TextureResourceTag>;
using CubemapHandle = GpuResourceHandle<CubemapResourceTag>;
using MaterialHandle = GpuResourceHandle<MaterialResourceTag>;
using FramebufferHandle = GpuResourceHandle<FramebufferResourceTag>;
using CubemapFramebufferHandle = GpuResourceHandle<CubemapFramebufferResourceTag>;
using BufferHandle = GpuResourceHandle<BufferResourceTag>;

struct GpuResourceHandleHash {
    template<typename Tag>
    size_t operator()(GpuResourceHandle<Tag> handle) const noexcept
    {
        const uint64_t packed = (static_cast<uint64_t>(handle.generation()) << 32)
            | static_cast<uint64_t>(handle.index());
        return static_cast<size_t>(packed ^ (packed >> 33));
    }
};

static_assert(!std::is_convertible<GeometryHandle, TextureHandle>::value,
    "GPU resource handles must remain strongly typed");

} // namespace Galaxy
