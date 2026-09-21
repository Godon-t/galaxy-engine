#pragma once


#include "RenderItem.hpp"
#include "common/geometry/Shapes.hpp"
#include "Frustum.hpp"

#include "types/Math.hpp"
#include "types/Render.hpp"

#include "queue"

using namespace math;

namespace Galaxy
{
    struct SceneContext
    {
        std::vector<RenderItem> renderItems;
        std::unordered_map<MaterialHandle, bool, GpuResourceHandleHash> materialsTransparency;

        std::vector<RenderItem> retrieveOpaqueRenders();
        std::vector<RenderItem> retrieveTransparentRenders(math::vec3 camPosition);

        std::vector<RenderItem> retrieveOpaqueRenders(const Frustum& frustum);
        std::vector<RenderItem> retrieveTransparentRenders(const Frustum& frustum);

        void push(RenderItem item);
        void removeMaterial(MaterialHandle material);
        
        void onMaterialUpdated(MaterialHandle material, bool isTransparent);

        void clear();

        private:
        struct DistCompare {
            static math::vec3 camPosition;
            bool operator()(const RenderItem& a, const RenderItem& b) const;
        };
    };
    
} // namespace Galaxy
