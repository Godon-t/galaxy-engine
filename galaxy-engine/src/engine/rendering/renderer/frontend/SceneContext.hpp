#pragma once


#include "RenderItem.hpp"
#include "rendering/renderer/commands/RenderCommand.hpp"
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

        std::vector<RenderCommand> retrieveOpaqueRenders();
        std::vector<RenderCommand> retrieveTransparentRenders(math::vec3 camPosition);

        std::vector<RenderCommand> retrieveOpaqueRenders(const Frustum& frustum);
        std::vector<RenderCommand> retrieveTransparentRenders(const Frustum& frustum);

        void push(RenderItem item);
        void removeMaterial(MaterialHandle material);
        
        void onMaterialUpdated(MaterialHandle material, bool isTransparent);

        void clear();

        private:
        struct DistCompare {
            static math::vec3 camPosition;
            bool operator()(const std::pair<MaterialHandle, RenderCommand>& a, const std::pair<MaterialHandle, RenderCommand>& b) const;
        };
    };
    
} // namespace Galaxy
