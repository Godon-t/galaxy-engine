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
    return length(camPosition - aPosition) - a.bounds.radius < length(camPosition - bPosition) - b.bounds.radius;
}

const MaterialRenderInfo& SceneContext::getMaterialInfo(std::optional<MaterialHandle> material) const
{
    static const MaterialRenderInfo defaultMaterial;
    if (material) {
        const auto found = materials.find(*material);
        if (found != materials.end())
            return found->second;
    }
    return defaultMaterial;
}

std::vector<RenderItem> SceneContext::retrieveOpaqueRenders(const BitMask& mask)
{
    std::vector<RenderItem> items;

    for (const RenderItem& item : renderItems) {
        bool inMask = BitMask::overlap(mask, item.visibilityLayer);
        if(!inMask)
            continue;

        if (!getMaterialInfo(item.material).isTransparent) {
            items.push_back(item);
        }
    }

    return items;
}

std::vector<RenderItem> SceneContext::retrieveTransparentRenders(math::vec3 camPosition, const BitMask& mask)
{
    DistCompare::camPosition = camPosition;
    std::priority_queue<
        RenderItem,
        std::vector<RenderItem>,
        DistCompare>
        transparentItems;

    for (const RenderItem& item : renderItems) {
        bool inMask = BitMask::overlap(mask, item.visibilityLayer);
        if(!inMask)
            continue;

        if (!getMaterialInfo(item.material).isTransparent)
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

std::vector<RenderItem> SceneContext::retrieveOpaqueRenders(const Frustum& frustum, const BitMask& mask)
{
    std::vector<RenderItem> items;
    for (const RenderItem& item : renderItems) {
        bool inMask = BitMask::overlap(mask, item.visibilityLayer);
        if(!inMask)
            continue;

        if (!Frustum::isSphereInFrustum(item.bounds, frustum, item.transform))
            continue;

        if (!getMaterialInfo(item.material).isTransparent) {
            items.push_back(item);
        }
    }

    return items;
}

std::vector<RenderItem> SceneContext::retrieveTransparentRenders(const Frustum& frustum, const BitMask& mask)
{
    DistCompare::camPosition = frustum.nearFace.position;
    std::priority_queue<
        RenderItem,
        std::vector<RenderItem>,
        DistCompare>
        transparentItems;

    for (const RenderItem& item : renderItems) {
        bool inMask = BitMask::overlap(mask, item.visibilityLayer);
        if(!inMask)
            continue;

        if (!getMaterialInfo(item.material).isTransparent)
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
    materials.erase(material);
    renderItems.erase(
        std::remove_if(renderItems.begin(), renderItems.end(), [material](const RenderItem& item) {
            return item.material && *item.material == material;
        }),
        renderItems.end());
}

void SceneContext::onMaterialUpdated(MaterialHandle material, const MaterialRenderInfo& info)
{
    materials[material] = info;
}

void SceneContext::clear()
{
    renderItems.clear();
}

} // namespace Galaxy
