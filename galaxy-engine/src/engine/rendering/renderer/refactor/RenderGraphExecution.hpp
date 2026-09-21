#pragma once

#include "rendering/renderer/refactor/RenderGraphDeclaration.hpp"
#include "rendering/renderer/frontend/RenderItem.hpp"
#include "engine/types/Math.hpp"
#include "rendering/renderer/commands/RenderCommand.hpp"

#include <string>
#include <unordered_map>
#include <vector>

namespace Galaxy
{
    class RenderGraph;
    class Program;

    struct UniformBufferBinding {
        BufferHandle buffer;
        std::uint32_t bindingPoint;
    };

    using UniformBufferBindingSet =
        std::vector<UniformBufferBinding>;

    bool applyShaderParameter(
        const Program& program,
        const std::string& name,
        const ShaderParameterValue& parameter);

    using ShaderParameterSet = std::unordered_map<std::string, ShaderParameterValue>;

    // One concrete rendering of a pass: one camera, light, cubemap face, etc.
    struct RenderPassInvocation {
        std::vector<RenderItem> items;
        ShaderParameterSet parameters;
        ShaderParameterSet viewParameters;
        UniformBufferBindingSet uniformBindings;
        std::vector<UpdateUBOCommand> updates;

        vec4 clearColor = vec4(1.0, 0.0, 0.0, 1.0);
        ivec2 viewportSize = ivec2(512);
        ivec2 viewportPosition = ivec2(0);

        // -1 selects a regular 2D attachment. Non-negative values select an array layer.
        int targetLayer = -1;
    };

    struct RenderPassExecution {
        RenderPassId pass;

        // Zero invocations skip this pass for the frame; several repeat only this pass.
        std::vector<RenderPassInvocation> invocations;
    };

    // Execution phase: dynamic work submitted for one frame to the unique render graph.
    struct RenderGraphExecution {
        explicit RenderGraphExecution(RenderGraph& renderGraph);

        [[nodiscard]] RenderPassExecution& getPass(RenderPassId pass);
        [[nodiscard]] RenderPassInvocation& addInvocation(RenderPassId pass);

        std::vector<RenderPassExecution> passes;
        RenderGraph& graph;
    };

} // namespace Galaxy
