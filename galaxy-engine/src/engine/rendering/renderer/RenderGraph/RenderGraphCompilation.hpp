#pragma once

#include "RenderGraphDeclaration.hpp"
#include "engine/core/Log.hpp"

#include <algorithm>
#include <deque>
#include <limits>
#include <optional>
#include <vector>

namespace Galaxy {

struct CompiledGraphTexture {
    inline static constexpr std::size_t NoUse = std::numeric_limits<std::size_t>::max();

    std::optional<RenderPassId> producer;
    std::vector<RenderPassId> consumers;
    GraphTextureId textureId;

    std::size_t firstUse = NoUse;
    std::size_t lastUse = NoUse;
};

struct CompiledRenderPass {
    RenderPassId id;
    std::vector<RenderPassId> dependencies;
    TargetId renderTargetId;
};

// Compilation phase: owns the dependency order and resource lifetimes.
struct CompiledRenderGraph {
    RenderGraphDeclaration declaration;

    // Passes are stored directly in execution order.
    std::vector<CompiledRenderPass> passes;
    std::vector<CompiledGraphTexture> textures;

    // Converts a declaration PassId to its position in the execution order.
    std::vector<std::size_t> executionIndexByPass;

    [[nodiscard]] const GraphTextureDesc& getTextureDescription(GraphTextureId id) const
    {
        return declaration.textures[id.index];
    }

    [[nodiscard]] const RenderPassDesc& getPassDescription(RenderPassId id) const
    {
        return declaration.passes[id.index];
    }

    CompiledRenderGraph() = default;

    CompiledRenderGraph(const RenderGraphDeclaration& graphDeclaration)
        : declaration(graphDeclaration)
    {
        const std::size_t passCount = graphDeclaration.passes.size();
        const std::size_t textureCount = graphDeclaration.textures.size();

        textures.reserve(textureCount);
        for (std::size_t textureIndex = 0; textureIndex < textureCount; ++textureIndex) {
            textures.push_back(CompiledGraphTexture {
                std::nullopt,
                {},
                GraphTextureId { static_cast<GraphTextureId::Index>(textureIndex) },
                CompiledGraphTexture::NoUse,
                CompiledGraphTexture::NoUse
            });
        }

        std::vector<bool> registeredTargets(graphDeclaration.targets.size(), false);
        for (std::size_t passIndex = 0; passIndex < passCount; ++passIndex) {
            const RenderPassId passId { static_cast<RenderPassId::Index>(passIndex) };
            const RenderPassDesc& pass = graphDeclaration.passes[passIndex];
            const RenderTargetDesc& target = graphDeclaration.targets[pass.targetId.index];

            // Several passes can share a target. Its attachments are still produced once.
            if (!registeredTargets[pass.targetId.index]) {
                registeredTargets[pass.targetId.index] = true;
                for (GraphTextureId output : target.colorAttachments) {
                    CompiledGraphTexture& texture = textures[output.index];
                    if (texture.producer.has_value()) {
                        GLX_ERROR(
                            "Texture '{0}' is written by several passes",
                            graphDeclaration.textures[output.index].name);
                    } else {
                        texture.producer = passId;
                    }
                }

                if (target.depthAttachment)
                    textures[target.depthAttachment->index].producer = passId;
            }

            for (const SampledTextureInputDesc& input : pass.inputTextures) {
                std::vector<RenderPassId>& consumers = textures[input.texture.index].consumers;
                if (std::find(consumers.begin(), consumers.end(), passId) == consumers.end())
                    consumers.push_back(passId);
            }
        }

        std::vector<std::vector<RenderPassId>> successors(passCount);
        std::vector<std::vector<RenderPassId>> dependencies(passCount);
        std::vector<std::size_t> dependencyCounts(passCount, 0);

        for (const CompiledGraphTexture& texture : textures) {
            if (!texture.producer)
                continue;

            const RenderPassId producer = *texture.producer;
            for (RenderPassId consumer : texture.consumers) {
                std::vector<RenderPassId>& consumerDependencies = dependencies[consumer.index];
                if (std::find(consumerDependencies.begin(), consumerDependencies.end(), producer)
                    != consumerDependencies.end()) {
                    continue;
                }

                consumerDependencies.push_back(producer);
                successors[producer.index].push_back(consumer);
                ++dependencyCounts[consumer.index];
            }
        }

        std::deque<RenderPassId> readyPasses;
        for (std::size_t passIndex = 0; passIndex < passCount; ++passIndex) {
            if (dependencyCounts[passIndex] == 0)
                readyPasses.push_back(RenderPassId { static_cast<RenderPassId::Index>(passIndex) });
        }

        std::vector<RenderPassId> executionOrder;
        executionOrder.reserve(passCount);

        while (!readyPasses.empty()) {
            const RenderPassId pass = readyPasses.front();
            readyPasses.pop_front();
            executionOrder.push_back(pass);

            for (RenderPassId successor : successors[pass.index]) {
                std::size_t& dependencyCount = dependencyCounts[successor.index];
                --dependencyCount;
                if (dependencyCount == 0)
                    readyPasses.push_back(successor);
            }
        }

        if (executionOrder.size() != passCount) {
            for (std::size_t passIndex = 0; passIndex < passCount; ++passIndex) {
                if (dependencyCounts[passIndex] == 0)
                    continue;

                GLX_ERROR(
                    "Pass '{0}' cannot be scheduled because of a dependency cycle",
                    graphDeclaration.passes[passIndex].name);
            }
            return;
        }

        executionIndexByPass.resize(passCount);
        passes.reserve(passCount);

        auto markTextureUse = [this](GraphTextureId texture, std::size_t executionIndex) {
            CompiledGraphTexture& compiledTexture = textures[texture.index];
            compiledTexture.firstUse = std::min(compiledTexture.firstUse, executionIndex);
            compiledTexture.lastUse = executionIndex;
        };

        for (std::size_t executionIndex = 0; executionIndex < executionOrder.size(); ++executionIndex) {
            const RenderPassId passId = executionOrder[executionIndex];
            const RenderPassDesc& pass = graphDeclaration.passes[passId.index];
            const RenderTargetDesc& target = graphDeclaration.targets[pass.targetId.index];

            executionIndexByPass[passId.index] = executionIndex;
            passes.push_back(CompiledRenderPass {
                passId,
                dependencies[passId.index],
                pass.targetId
            });

            for (const SampledTextureInputDesc& input : pass.inputTextures)
                markTextureUse(input.texture, executionIndex);
            for (GraphTextureId output : target.colorAttachments)
                markTextureUse(output, executionIndex);
            if (target.depthAttachment)
                markTextureUse(*target.depthAttachment, executionIndex);
        }
    }
};

} // namespace Galaxy
