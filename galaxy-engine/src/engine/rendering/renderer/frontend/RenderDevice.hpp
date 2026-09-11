#pragma once

#include "types/Math.hpp"
#include "pch.hpp"
#include "rendering/renderer/commands/RenderCommand.hpp"
#include "rendering/renderer/resources/GpuResourceHandle.hpp"
#include "SceneContext.hpp"
#include "rendering/CameraManager.hpp"

using namespace math;

namespace Galaxy
{
    struct RenderDevice {
        bool renderScene = false;
        // GLX-TODO: not ideal to have 2 targets at the same time
        FramebufferHandle targetFramebuffer;
        CubemapFramebufferHandle targetCubemapFramebuffer;
        int targetDepthLayer = -1;
        vec2 viewportPosition = vec2(0);
        vec2 viewportDimmension = vec2(512);

        bool noClear = false;
        bool frustumCulling = true;


        std::vector<RenderCommand> customPostCommands;

        bool useBuffer() const { return targetFramebuffer || targetCubemapFramebuffer; }

        virtual mat4 getView(){
            vec3 pos = vec3(0,0,0);
            vec3 target = vec3(0,0,1);
            vec3 up = vec3(0,1,0);
            static mat4 view = lookAt(pos, target, up);
            return view;
        }
        virtual mat4 getProjection(){
            return CameraManager::processProjectionMatrix(viewportDimmension);
        }
        // virtual void fillCommandBuffer(std::vector<RenderCommand>& buffer, SceneContext& context){
        //     if(renderScene){
        //         auto opaques = context.retrieveOpaqueRenders();
        //         auto transparents = context.retrieveTransparentRenders(vec3(transform[3]));

        //         buffer.insert(buffer.end(), opaques.begin(), opaques.end());
        //         buffer.insert(buffer.end(), transparents.begin(), transparents.end());
        //     }
        //     buffer.insert(buffer.end(), customPostCommands.begin(), customPostCommands.end());
        // }
    };

    struct RenderCamera: public RenderDevice {
        std::shared_ptr<Camera> camera = std::make_shared<Camera>();

        mat4 getView() override {
            return CameraManager::processViewMatrix(camera);
        }

        mat4 getProjection() override{
            return CameraManager::processProjectionMatrix(camera->dimmensions);
        }
    };

    struct RenderCameraTransform : public RenderCamera {
        mat4 transform;

        mat4 getView() override {
            return CameraManager::processViewMatrix(transform);
        }
    };

    struct RenderPoint: public RenderDevice {
        std::shared_ptr<Camera> camera = std::make_shared<Camera>();

        std::vector<mat4> getViews() {
            static vec3 s_cubemap_orientations[6] = {{1, 0, 0}, {-1, 0, 0}, {0, 1, 0}, {0, -1, 0}, {0, 0, 1}, {0, 0, -1}};
            static vec3 s_cubemap_ups[6] = {{0, -1, 0}, {0, -1, 0}, {0, 0, 1}, {0, 0, -1}, {0, -1, 0}, {0, -1, 0}};

            std::vector<mat4> res;

            vec3 position = vec3(camera->position);
            for(int i=0; i<6; i++){
                auto viewMatrix = lookAt(position, position + s_cubemap_orientations[i], s_cubemap_ups[i]);
                res.push_back(viewMatrix);
            }
            return res;
        }
    };
} // namespace Galaxy
