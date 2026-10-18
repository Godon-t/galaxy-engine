#pragma once


#include "RenderItem.hpp"
#include "common/BitMask.hpp"
#include "common/geometry/Shapes.hpp"
#include "Frustum.hpp"

#include "types/Math.hpp"
#include "types/Render.hpp"
#include "types/Material.hpp"

#include "queue"

using namespace math;

namespace Galaxy
{
    struct SceneContext
    {
        std::vector<RenderItem> renderItems;
        std::unordered_map<MaterialHandle, MaterialRenderInfo, GpuResourceHandleHash> materials;

        std::vector<RenderItem> retrieveOpaqueRenders(const BitMask& mask);
        std::vector<RenderItem> retrieveTransparentRenders(math::vec3 camPosition, const BitMask& mask);

        std::vector<RenderItem> retrieveOpaqueRenders(const Frustum& frustum, const BitMask& mask);
        std::vector<RenderItem> retrieveTransparentRenders(const Frustum& frustum, const BitMask& mask);

        void push(RenderItem item);
        void removeMaterial(MaterialHandle material);
        
        void onMaterialUpdated(MaterialHandle material, const MaterialRenderInfo& info);
        const MaterialRenderInfo& getMaterialInfo(std::optional<MaterialHandle> material) const;

        void clear();

        private:
        struct DistCompare {
            static math::vec3 camPosition;
            bool operator()(const RenderItem& a, const RenderItem& b) const;
        };
    };
    
} // namespace Galaxy
