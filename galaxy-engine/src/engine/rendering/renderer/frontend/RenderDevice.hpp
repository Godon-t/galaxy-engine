#pragma once

#include "types/Math.hpp"
#include "pch.hpp"
#include "rendering/renderer/RenderCommand.hpp"
#include "SceneContext.hpp"
#include "rendering/CameraManager.hpp"

using namespace math;

namespace Galaxy
{
    struct RenderDevice {
        bool renderScene = false;
        renderID targetFramebuffer = -1;
        int targetDepthLayer = -1;
        std::shared_ptr<Camera> camera = std::make_shared<Camera>();
        vec2 viewportPosition = vec2(0);

        bool noClear = false;
        bool frustumCulling = true;


        std::vector<RenderCommand> customPostCommands;

        bool useBuffer() {return targetFramebuffer >= 0;}

        virtual std::vector<mat4> getViews(){
            vec3 pos = vec3(0,0,0);
            vec3 target = vec3(0,0,1);
            vec3 up = vec3(0,1,0);
            static mat4 view = lookAt(pos, target, up);
            std::vector<mat4> res = {view};
            return res;
        }
        virtual mat4 getProjection(){
            return CameraManager::processProjectionMatrix(vec2(256,256));
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

    struct RenderCameraTransform : public RenderDevice {
        mat4 transform;
        vec2 dimmensions;

        std::vector<mat4> getViews() override {
            std::vector<mat4> res{CameraManager::processViewMatrix(transform)};
            return res;
        }

        mat4 getProjection() override{
            return CameraManager::processProjectionMatrix(dimmensions);
        }
    };

    struct RenderCamera: public RenderDevice {
        std::vector<mat4> getViews() override {
            std::vector<mat4> res{CameraManager::processViewMatrix(camera)};
            return res;
        }

        mat4 getProjection() override{
            return CameraManager::processProjectionMatrix(camera->dimmensions);
        }
    };

    struct RenderPoint: public RenderDevice {        
        std::vector<mat4> getViews() override {
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
