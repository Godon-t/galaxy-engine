#pragma once

#include "GpuResourceHandle.hpp"
#include "core/Log.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <utility>
#include <vector>

namespace Galaxy {

// Owns GPU-side resource wrappers. Handles are non-owning references to slots.
// A slot's generation changes whenever it is released, making stale handles fail
// validation even after the same slot is reused.
template<typename T, typename Handle>
class GpuResourceRegistry {
public:
    explicit GpuResourceRegistry(size_t capacity = 512)
        : m_slots(capacity)
    {
        m_freeSlots.reserve(capacity);
        for (size_t index = capacity; index > 0; --index)
            m_freeSlots.push_back(static_cast<uint32_t>(index - 1));
    }

    GpuResourceRegistry(const GpuResourceRegistry&) = delete;
    GpuResourceRegistry& operator=(const GpuResourceRegistry&) = delete;

    template<typename... Args>
    [[nodiscard]] Handle create(Args&&... args)
    {
        if (m_freeSlots.empty()) {
            GLX_CORE_ERROR("No more free slots in GPU resource registry");
            return {};
        }

        const uint32_t index = m_freeSlots.back();
        m_freeSlots.pop_back();

        Slot& slot = m_slots[index];
        GLX_CORE_ASSERT(!slot.resource.has_value(), "GPU resource slot is already occupied");
        slot.resource.emplace(std::forward<Args>(args)...);
        slot.referenceCount = 1;

        return Handle(index, slot.generation);
    }

    [[nodiscard]] bool contains(Handle handle) const noexcept
    {
        if (!handle || handle.index() >= m_slots.size())
            return false;

        const Slot& slot = m_slots[handle.index()];
        return slot.resource.has_value() && slot.generation == handle.generation();
    }

    [[nodiscard]] T* tryGet(Handle handle) noexcept
    {
        if (!contains(handle))
            return nullptr;
        return &*m_slots[handle.index()].resource;
    }

    [[nodiscard]] const T* tryGet(Handle handle) const noexcept
    {
        if (!contains(handle))
            return nullptr;
        return &*m_slots[handle.index()].resource;
    }

    // Intended for code paths where the handle has just been created or already
    // validated. Queued commands should use tryGet and handle expiration.
    [[nodiscard]] T* get(Handle handle)
    {
        T* resource = tryGet(handle);
        GLX_CORE_ASSERT(resource != nullptr,
            "Invalid GPU resource handle (slot={0}, generation={1})",
            handle.index(), handle.generation());
        return resource;
    }

    void retain(Handle handle)
    {
        if (!contains(handle)) {
            GLX_CORE_ERROR("Trying to retain an invalid GPU resource handle (slot={0}, generation={1})",
                handle.index(), handle.generation());
            return;
        }
        ++m_slots[handle.index()].referenceCount;
    }

    [[nodiscard]] bool release(Handle handle)
    {
        return release(handle, [](T&) {});
    }

    template<typename BeforeRelease>
    [[nodiscard]] bool release(Handle handle, BeforeRelease&& beforeRelease)
    {
        if (!contains(handle)) {
            GLX_CORE_ERROR("Trying to release an invalid GPU resource handle (slot={0}, generation={1})",
                handle.index(), handle.generation());
            return false;
        }

        Slot& slot = m_slots[handle.index()];
        if (slot.referenceCount > 1) {
            --slot.referenceCount;
            return false;
        }

        beforeRelease(*slot.resource);
        slot.resource.reset();
        slot.referenceCount = 0;
        advanceGeneration(slot);
        m_freeSlots.push_back(handle.index());
        return true;
    }

    [[nodiscard]] bool canCreate() const noexcept { return !m_freeSlots.empty(); }

    std::vector<T*> getAll()
    {
        std::vector<T*> resources;
        resources.reserve(m_slots.size() - m_freeSlots.size());
        for (Slot& slot : m_slots) {
            if (slot.resource)
                resources.push_back(&*slot.resource);
        }
        return resources;
    }

    void clear()
    {
        m_freeSlots.clear();
        m_freeSlots.reserve(m_slots.size());

        for (size_t index = m_slots.size(); index > 0; --index) {
            Slot& slot = m_slots[index - 1];
            if (slot.resource) {
                slot.resource.reset();
                slot.referenceCount = 0;
                advanceGeneration(slot);
            }
            m_freeSlots.push_back(static_cast<uint32_t>(index - 1));
        }
    }

private:
    struct Slot {
        std::optional<T> resource;
        size_t referenceCount = 0;
        uint32_t generation = 1;
    };

    static void advanceGeneration(Slot& slot) noexcept
    {
        ++slot.generation;
        if (slot.generation == 0)
            slot.generation = 1;
    }

    std::vector<Slot> m_slots;
    std::vector<uint32_t> m_freeSlots;
};

} // namespace Galaxy
