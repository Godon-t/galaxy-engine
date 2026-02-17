#pragma once


#include "rendering/renderer/RenderCommand.hpp"

#include "types/Math.hpp"
#include "types/Render.hpp"

#include "queue"

using namespace math;

namespace Galaxy
{
    struct SceneContext
    {
        std::unordered_map<renderID, std::vector<DrawCommand>> renderCommandsByMaterial;
        std::unordered_map<renderID, bool> materialsTransparency;

        std::vector<RenderCommand> retrieveOpaqueRenders();
        std::vector<RenderCommand> retrieveTransparentRenders(math::vec3 camPosition);

        void pushCommand(renderID materialID, DrawCommand drawCommand);
        void removeMaterialID(renderID materialID);
        
        void onMaterialUpdated(renderID materialID, bool isTransparent);

        void clear();

        private:
        struct DistCompare {
            static math::vec3 camPosition;
            bool operator()(const std::pair<renderID, RenderCommand>& a, const std::pair<renderID, RenderCommand>& b) const;
        };
    };
    
} // namespace Galaxy
