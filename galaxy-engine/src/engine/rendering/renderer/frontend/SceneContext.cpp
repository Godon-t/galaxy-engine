#include "SceneContext.hpp"

#include "Log.hpp"

#include <algorithm>

namespace Galaxy {

vec3 SceneContext::DistCompare::camPosition = vec3(0);

bool SceneContext::DistCompare::operator()(
    const std::pair<MaterialHandle, RenderCommand>& a,
    const std::pair<MaterialHandle, RenderCommand>& b) const
{
    try {
        return (camPosition - vec3(std::get<DrawCommand>(a.second).model[3])).length()
            < (camPosition - vec3(std::get<DrawCommand>(b.second).model[3])).length();
    } catch (const std::bad_variant_access&) {
        GLX_CORE_ERROR("Wrong command type when drawing according to distance");
        return false;
    }
}

namespace {

bool isTransparent(
    const std::unordered_map<MaterialHandle, bool, GpuResourceHandleHash>& transparencies,
    MaterialHandle material)
{
    const auto found = transparencies.find(material);
    return found != transparencies.end() && found->second;
}

void appendDraw(std::vector<RenderCommand>& commands, const RenderItem& item)
{
    DrawCommand draw;
    draw.geometry = item.geometry;
    draw.model = item.transform.getGlobalModelMatrix();
    commands.emplace_back(std::move(draw));
}

} // namespace

std::vector<RenderCommand> SceneContext::retrieveOpaqueRenders()
{
    std::vector<RenderCommand> commands;
    std::unordered_map<MaterialHandle, std::vector<const RenderItem*>, GpuResourceHandleHash> groupedItems;

    for (const RenderItem& item : renderItems) {
        if (!item.material) {
            appendDraw(commands, item);
            continue;
        }

        if (!isTransparent(materialsTransparency, *item.material))
            groupedItems[*item.material].push_back(&item);
    }

    for (const auto& [material, items] : groupedItems) {
        commands.emplace_back(BindMaterialCommand { material });
        for (const RenderItem* item : items)
            appendDraw(commands, *item);
    }

    return commands;
}

std::vector<RenderCommand> SceneContext::retrieveTransparentRenders(math::vec3 camPosition)
{
    DistCompare::camPosition = camPosition;
    std::priority_queue<
        std::pair<MaterialHandle, RenderCommand>,
        std::vector<std::pair<MaterialHandle, RenderCommand>>,
        DistCompare>
        transparentItems;

    for (const RenderItem& item : renderItems) {
        if (!item.material || !isTransparent(materialsTransparency, *item.material))
            continue;

        DrawCommand draw;
        draw.geometry = item.geometry;
        draw.model = item.transform.getGlobalModelMatrix();
        transparentItems.emplace(*item.material, std::move(draw));
    }

    std::vector<RenderCommand> commands;
    while (!transparentItems.empty()) {
        commands.emplace_back(BindMaterialCommand { transparentItems.top().first });
        commands.push_back(transparentItems.top().second);
        transparentItems.pop();
    }
    return commands;
}

std::vector<RenderCommand> SceneContext::retrieveOpaqueRenders(const Frustum& frustum)
{
    std::vector<RenderCommand> commands;
    std::unordered_map<MaterialHandle, std::vector<const RenderItem*>, GpuResourceHandleHash> groupedItems;

    for (const RenderItem& item : renderItems) {
        if (!Frustum::isSphereInFrustum(item.bounds, frustum, item.transform))
            continue;

        if (!item.material) {
            appendDraw(commands, item);
            continue;
        }

        if (!isTransparent(materialsTransparency, *item.material))
            groupedItems[*item.material].push_back(&item);
    }

    for (const auto& [material, items] : groupedItems) {
        commands.emplace_back(BindMaterialCommand { material });
        for (const RenderItem* item : items)
            appendDraw(commands, *item);
    }

    return commands;
}

std::vector<RenderCommand> SceneContext::retrieveTransparentRenders(const Frustum& frustum)
{
    DistCompare::camPosition = frustum.nearFace.position;
    std::priority_queue<
        std::pair<MaterialHandle, RenderCommand>,
        std::vector<std::pair<MaterialHandle, RenderCommand>>,
        DistCompare>
        transparentItems;

    for (const RenderItem& item : renderItems) {
        if (!item.material || !isTransparent(materialsTransparency, *item.material))
            continue;
        if (!Frustum::isSphereInFrustum(item.bounds, frustum, item.transform))
            continue;

        DrawCommand draw;
        draw.geometry = item.geometry;
        draw.model = item.transform.getGlobalModelMatrix();
        transparentItems.emplace(*item.material, std::move(draw));
    }

    std::vector<RenderCommand> commands;
    while (!transparentItems.empty()) {
        commands.emplace_back(BindMaterialCommand { transparentItems.top().first });
        commands.push_back(transparentItems.top().second);
        transparentItems.pop();
    }
    return commands;
}

void SceneContext::push(RenderItem item)
{
    renderItems.push_back(std::move(item));
}

void SceneContext::removeMaterial(MaterialHandle material)
{
    materialsTransparency.erase(material);
    renderItems.erase(
        std::remove_if(renderItems.begin(), renderItems.end(), [material](const RenderItem& item) {
            return item.material && *item.material == material;
        }),
        renderItems.end());
}

void SceneContext::onMaterialUpdated(MaterialHandle material, bool transparent)
{
    materialsTransparency[material] = transparent;
}

void SceneContext::clear()
{
    renderItems.clear();
}

} // namespace Galaxy
