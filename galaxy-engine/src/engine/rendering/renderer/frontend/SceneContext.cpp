#include "SceneContext.hpp"

#include "Log.hpp"


namespace Galaxy
{
    vec3 SceneContext::DistCompare::camPosition = vec3(0);
    bool SceneContext::DistCompare::operator()(const std::pair<renderID, RenderCommand>& a, const std::pair<renderID, RenderCommand>& b) const
    {
        try {
            return (camPosition - vec3(std::get<DrawCommand>(a.second).model[3])).length() < (camPosition - vec3(std::get<DrawCommand>(b.second).model[3])).length();
        } catch (const std::bad_variant_access& ex) {
            GLX_CORE_ERROR("Wrong command type when drawing according to distance");
            return false;
        }
    }

    std::vector<RenderCommand> SceneContext::retrieveOpaqueRenders()
    {
        std::vector<RenderCommand> res;
        for (auto& queue : renderCommandsByMaterial) {
            auto matID = queue.first;
            if (!materialsTransparency[matID]) {
                BindMaterialCommand bindMaterialCommand;
                bindMaterialCommand.materialRenderID = matID;

                res.push_back(bindMaterialCommand);

                for (auto& meshCommand : queue.second) {
                    res.push_back(meshCommand);
                }
            }
        }

        return res;
    }

    std::vector<RenderCommand> SceneContext::retrieveTransparentRenders(math::vec3 camPosition)
    {
        DistCompare::camPosition = math::vec3(camPosition);

        std::priority_queue<std::pair<renderID, RenderCommand>, std::vector<std::pair<renderID, RenderCommand>>, DistCompare> transparentPQ;

        std::vector<RenderCommand> res;
        for (auto& queue : renderCommandsByMaterial) {
            auto matID = queue.first;
            if (materialsTransparency[matID]) {
                for (auto& meshCommand : queue.second) {
                    auto elem = std::make_pair(matID, meshCommand);
                    transparentPQ.push(elem);
                }
            }
        }

        // DepthMaskCommand depthMask;
        // depthMask.state = false;

        // res.push_back(depthMask);

        while (!transparentPQ.empty()) {
            BindMaterialCommand bindMaterialCommand;
            bindMaterialCommand.materialRenderID = transparentPQ.top().first;

            res.push_back(bindMaterialCommand);
            res.push_back(transparentPQ.top().second);

            transparentPQ.pop();
        }

        // depthMask.state = true;
        // res.push_back(depthMask);

        return res;
    }

    void SceneContext::pushCommand(renderID materialID, DrawCommand drawCommand)
    {
        renderCommandsByMaterial[materialID].push_back(drawCommand);
    }

    void SceneContext::removeMaterialID(renderID materialID)
    {
        materialsTransparency.erase(materialID);
        renderCommandsByMaterial.erase(materialID);
    }

    void SceneContext::onMaterialUpdated(renderID materialID, bool isTransparent)
    {
        materialsTransparency[materialID] = isTransparent;
    }

    void SceneContext::clear()
    {
        for(auto& ite: renderCommandsByMaterial){
            ite.second.clear();
        }
    }

} // namespace Galaxy