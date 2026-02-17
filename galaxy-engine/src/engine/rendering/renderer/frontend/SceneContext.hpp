#pragma once


#include "rendering/renderer/RenderCommand.hpp"
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
        struct Visual {
            Sphere volume;
            Transform transform;
            renderID meshID;
        };
        std::unordered_map<renderID, std::vector<Visual>> renderCommandsByMaterial;
        std::unordered_map<renderID, bool> materialsTransparency;

        std::vector<RenderCommand> retrieveOpaqueRenders();
        std::vector<RenderCommand> retrieveTransparentRenders(math::vec3 camPosition);

        std::vector<RenderCommand> retrieveOpaqueRenders(const Frustum& frustum);
        std::vector<RenderCommand> retrieveTransparentRenders(const Frustum& frustum);

        void pushNewObject(renderID materialID, renderID meshID, const Sphere& volume, const Transform& transform);
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
