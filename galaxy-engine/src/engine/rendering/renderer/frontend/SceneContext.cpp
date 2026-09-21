#include "SceneContext.hpp"

#include "Log.hpp"

#include <algorithm>

namespace Galaxy {

vec3 SceneContext::DistCompare::camPosition = vec3(0);

bool SceneContext::DistCompare::operator()(
    const RenderItem& a,
    const RenderItem& b) const
{
    const vec3 aPosition = vec3(a.transform.getGlobalModelMatrix()[3]);
    const vec3 bPosition = vec3(b.transform.getGlobalModelMatrix()[3]);
    return (camPosition - aPosition).length() < (camPosition - bPosition).length();
}

namespace {

bool isTransparent(
    const std::unordered_map<MaterialHandle, bool, GpuResourceHandleHash>& transparencies,
    MaterialHandle material)
{
    const auto found = transparencies.find(material);
    return found != transparencies.end() && found->second;
}

} // namespace

std::vector<RenderItem> SceneContext::retrieveOpaqueRenders()
{
    std::vector<RenderItem> items;

    for (const RenderItem& item : renderItems) {
        if (!item.material || !isTransparent(materialsTransparency, *item.material)) {
            items.push_back(item);
        }
    }

    return items;
}

std::vector<RenderItem> SceneContext::retrieveTransparentRenders(math::vec3 camPosition)
{
    DistCompare::camPosition = camPosition;
    std::priority_queue<
        RenderItem,
        std::vector<RenderItem>,
        DistCompare>
        transparentItems;

    for (const RenderItem& item : renderItems) {
        if (!item.material || !isTransparent(materialsTransparency, *item.material))
            continue;

        transparentItems.emplace(item);
    }

    std::vector<RenderItem> items;
    while (!transparentItems.empty()) {
        items.emplace_back(transparentItems.top());
        transparentItems.pop();
    }
    return items;
}

std::vector<RenderItem> SceneContext::retrieveOpaqueRenders(const Frustum& frustum)
{
    std::vector<RenderItem> items;
    for (const RenderItem& item : renderItems) {
        if (!Frustum::isSphereInFrustum(item.bounds, frustum, item.transform))
            continue;

        if (!item.material || !isTransparent(materialsTransparency, *item.material)) {
            items.push_back(item);
        }
    }

    return items;
}

std::vector<RenderItem> SceneContext::retrieveTransparentRenders(const Frustum& frustum)
{
    DistCompare::camPosition = frustum.nearFace.position;
    std::priority_queue<
        RenderItem,
        std::vector<RenderItem>,
        DistCompare>
        transparentItems;

    for (const RenderItem& item : renderItems) {
        if (!item.material || !isTransparent(materialsTransparency, *item.material))
            continue;
        if (!Frustum::isSphereInFrustum(item.bounds, frustum, item.transform))
            continue;

        transparentItems.emplace(item);
    }

    std::vector<RenderItem> items;
    while (!transparentItems.empty()) {
        items.emplace_back(transparentItems.top());
        transparentItems.pop();
    }
    return items;
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
