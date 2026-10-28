#pragma once

#include "types/Math.hpp"
#include "pch.hpp"
#include "rendering/Commands.hpp"
#include "rendering/renderer/resources/GpuResourceHandle.hpp"
#include "SceneContext.hpp"
#include "rendering/CameraManager.hpp"

using namespace math;

namespace Galaxy
{
    struct RenderDevice {
        virtual ~RenderDevice() = default;

        bool renderScene = false;
        vec2 viewportPosition = vec2(0);
        vec2 viewportDimmension = vec2(512);

        bool frustumCulling = true;
        BitMask visibilityMask;
        // GLX-TODO: having a target here is ok with render graph final framebuffer ?
        std::optional<RenderTargetHandle> targetFramebuffer;

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
        CubemapFramebufferHandle targetCubemapFramebuffer;

        mat4 getProjection() override {
            return perspective(radians(90.0f), 1.0f, camera->zNear, camera->zFar);
        }

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
