#include "RenderGraphExecution.hpp"

#include "RenderGraph.hpp"
#include "engine/rendering/Program.hpp"

namespace Galaxy {

bool applyShaderParameter(
    const Program& program,
    const std::string& name,
    const ShaderParameterValue& parameter)
{
    if (!program.hasUniform(name))
        return false;

    return std::visit(
        [&](const auto& typedValue) {
            return program.setUniform(name, typedValue);
        },
        parameter);
}

RenderGraphExecution::RenderGraphExecution(RenderGraph& renderGraph)
    : graph(renderGraph)
{
    passes.reserve(renderGraph.compilation.passes.size());
    for (const CompiledRenderPass& pass : renderGraph.compilation.passes)
        passes.push_back(RenderPassExecution { pass.id, {} });
}

RenderPassExecution& RenderGraphExecution::getPass(RenderPassId pass)
{
    return passes.at(graph.getPassIndex(pass));
}

RenderPassInvocation& RenderGraphExecution::addInvocation(RenderPassId pass)
{
    auto& invocations = getPass(pass).invocations;
    invocations.emplace_back();
    return invocations.back();
}

} // namespace Galaxy
