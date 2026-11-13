#include "EnvironmentNode.hpp"

#include "rendering/RenderGraph/RenderGraph.hpp"
#include "rendering/RenderGraph/RenderGraphExecution.hpp"
#include "rendering/Renderer.hpp"
#include "resource/ResourceManager.hpp"
#include "core/Helper.hpp"

#include <algorithm>
#include <array>
#include <cmath>

namespace Galaxy {
EnvironmentNode::~EnvironmentNode()
{
    if (!m_environmentSourceTexture && !m_captureCube && m_directionalLightID < 0)
        return;
    auto& backend = Renderer::getInstance().getBackend();
    if (backend.isValid(m_environmentSourceTexture))
        backend.clearTexture(m_environmentSourceTexture);
    if (backend.isValid(m_captureCube))
        backend.clearMesh(m_captureCube);
    if (m_directionalLightID >= 0)
        Renderer::getInstance().getLightManager().unregisterLight(m_directionalLightID);
}

void EnvironmentNode::accept(Galaxy::NodeVisitor& visitor)
{
    visitor.visit(*this);
}

void EnvironmentNode::draw()
{
    Node::draw();
}

void EnvironmentNode::enteredRoot()
{
    if (m_directionalLightID < 0)
        m_directionalLightID = Renderer::getInstance().getLightManager().registerDirectionalLight(m_direction, m_color, m_intensity);

    // A node can enter the root again without registering duplicate passes.
    if (m_passEquirectangularToCubemapId.isValid())
        return;

    auto& frontend = Renderer::getInstance().getFrontend();
    auto& backend = Renderer::getInstance().getBackend();
    const auto rectangularProgram = backend.loadShader(engineRes("shaders/filters/rectangularToCubemap.glsl"));
    const auto irradianceProgram = backend.loadShader(engineRes("shaders/filters/irradiance.glsl"));
    const auto specularFilterProgram = backend.loadShader(engineRes("shaders/filters/specular.glsl"));


    m_captureCube = backend.generateCube(2.0f, true, nullptr);

    RenderTargetDesc conversionTarget;
    conversionTarget.colorAttachments.push_back(frontend.getEnvironmentGraphTexture());

    RenderPassDesc conversion;
    conversion.name = "equirectangular_to_cubemap";
    conversion.associatedProgram = rectangularProgram;
    conversion.targetId = frontend.addGraphTarget(conversionTarget);
    conversion.state.depthTest = false;
    conversion.state.depthWrite = false;
    m_passEquirectangularToCubemapId = frontend.addGraphPass(conversion);


    SampledTextureInputDesc skyboxInput;
    skyboxInput.samplerName = "skybox";
    skyboxInput.texture = frontend.getEnvironmentGraphTexture();


    
    RenderTargetDesc irradianceTarget;
    irradianceTarget.colorAttachments.push_back(frontend.getIrradianceGraphTexture());

    RenderPassDesc irradianceFilter;
    irradianceFilter.name = "irradiance_convolution";
    irradianceFilter.associatedProgram = irradianceProgram;
    irradianceFilter.targetId = frontend.addGraphTarget(irradianceTarget);
    irradianceFilter.state.depthTest = false;
    irradianceFilter.state.depthWrite = false;
    irradianceFilter.inputTextures.push_back(skyboxInput);
    m_passIrradianceConvolutionId = frontend.addGraphPass(irradianceFilter);


    RenderTargetDesc specularTarget;
    specularTarget.colorAttachments.push_back(frontend.getSpecularGraphTexture());

    RenderPassDesc specularFilter;
    specularFilter.name = "specular_filter";
    specularFilter.associatedProgram = specularFilterProgram;
    specularFilter.targetId = frontend.addGraphTarget(specularTarget);
    specularFilter.state.depthTest = false;
    specularFilter.state.depthWrite = false;
    specularFilter.inputTextures.push_back(skyboxInput);
    m_passSpecularFilterId = frontend.addGraphPass(specularFilter);
}

void EnvironmentNode::setDirection(const vec3& direction)
{
    const float magnitude = length(direction);
    if (!std::isfinite(magnitude) || magnitude < 1e-6f)
        return;
    m_direction = direction / magnitude;
    if (m_directionalLightID >= 0)
        Renderer::getInstance().getLightManager().updateLightDirection(m_directionalLightID, m_direction);
}

void EnvironmentNode::setColor(const vec3& color)
{
    if (!std::isfinite(color.x) || !std::isfinite(color.y) || !std::isfinite(color.z))
        return;
    m_color = glm::max(color, vec3(0.0f));
    if (m_directionalLightID >= 0)
        Renderer::getInstance().getLightManager().updateLightColor(m_directionalLightID, m_color);
}

void EnvironmentNode::setIntensity(float intensity)
{
    if (!std::isfinite(intensity))
        return;
    m_intensity = std::max(0.0f, intensity);
    if (m_directionalLightID >= 0)
        Renderer::getInstance().getLightManager().updateLightIntensity(m_directionalLightID, m_intensity);
}

void EnvironmentNode::setEnvironmentMap(EnvironmentMapSource environmentMap)
{
    const auto current = getEnvironmentMapResource();
    const auto next = Galaxy::getEnvironmentMapResource(environmentMap);
    if (current.hasResource() == next.hasResource()
        && (!next.hasResource() || &current.getResource() == &next.getResource()))
        return;

    m_environmentMap = std::move(environmentMap);
    prepareEnvironmentMap();
}

void EnvironmentNode::prepareEnvironmentMap()
{
    auto& renderer = Renderer::getInstance();
    auto& backend = renderer.getBackend();
    const auto resource = getEnvironmentMapResource();
    if (!resource.hasResource()) {
        if (backend.isValid(m_environmentSourceTexture))
            backend.clearTexture(m_environmentSourceTexture);
        m_environmentSourceTexture = {};
        return;
    }

    resource.getResource().onLoaded([this, &renderer, &backend]() {
        if (auto* cubemap = std::get_if<ResourceHandle<CubemapResource>>(&m_environmentMap)) {
            renderer.getFrontend().setEnvironmentMap(*cubemap);
            return;
        }
    
        auto& image = std::get<ResourceHandle<Image>>(m_environmentMap).getResource();
        const auto* pixels = image.getData();
        if (pixels == nullptr || image.getWidth() <= 0 || image.getHeight() <= 0) {
            GLX_CORE_WARN("Cannot convert an empty environment image");
            return;
        }
    
        constexpr int resolution = 1024;
        const auto source = backend.instantiateTexture(
            TextureFormat::RGB16F, vec2(image.getWidth(), image.getHeight()));
        if (!backend.isValid(source))
            return;
        if (!backend.setTextureData(source, image)) {
            backend.clearTexture(source);
            return;
        }
        backend.setTextureWrap(source, TextureWrap::REPEAT, TextureWrap::CLAMP_TO_EDGE);
    
        if (backend.isValid(m_environmentSourceTexture))
            backend.clearTexture(m_environmentSourceTexture);
        m_environmentSourceTexture = source;
    
        const std::array<vec3, 6> directions {
            vec3(1, 0, 0), vec3(-1, 0, 0), vec3(0, 1, 0),
            vec3(0, -1, 0), vec3(0, 0, 1), vec3(0, 0, -1)
        };
        const std::array<vec3, 6> ups {
            vec3(0, -1, 0), vec3(0, -1, 0), vec3(0, 0, 1),
            vec3(0, 0, -1), vec3(0, -1, 0), vec3(0, -1, 0)
        };
        const auto projection = perspective(radians(90.0f), 1.0f, 0.1f, 10.0f);
        for (std::size_t face = 0; face < directions.size(); ++face) {
            RenderPassInvocation invocation;
            invocation.viewParameters["projection"] = projection;
            invocation.viewParameters["view"] = lookAt(vec3(0), directions[face], ups[face]);
            invocation.viewportSize = ivec2(resolution);
            invocation.clearColor = vec4(0.0f);
            invocation.targetLayer = static_cast<int>(face);
            invocation.textureBindings.push_back({ source, "environmentHDR" });
            invocation.items.push_back(RenderItem { m_captureCube });
            renderer.getFrontend().pushPassInvocation(m_passEquirectangularToCubemapId, std::move(invocation));



            RenderPassInvocation irradianceInvocation;
            irradianceInvocation.viewParameters["projection"] = projection;
            irradianceInvocation.viewParameters["view"] = lookAt(vec3(0), directions[face], ups[face]);
            irradianceInvocation.viewportSize = ivec2(resolution);
            irradianceInvocation.clearColor = vec4(0.0f);
            irradianceInvocation.targetLayer = static_cast<int>(face);
            irradianceInvocation.items.push_back(RenderItem { m_captureCube });
            renderer.getFrontend().pushPassInvocation(m_passIrradianceConvolutionId, std::move(irradianceInvocation));
        }

        int maxMipLevels = 5;
        for(int mip=0; mip < maxMipLevels; mip++)
        for (std::size_t face = 0; face < directions.size(); ++face){
            RenderPassInvocation specularInvocation;
            specularInvocation.viewParameters["projection"] = projection;
            specularInvocation.viewParameters["view"] = lookAt(vec3(0), directions[face], ups[face]);

            float roughness = (float)mip / (float)(maxMipLevels - 1);
            specularInvocation.parameters["roughness"] = roughness;
            
            specularInvocation.viewportSize = ivec2(resolution * std::pow(0.5, mip));
            specularInvocation.clearColor = vec4(0.0f);
            specularInvocation.targetLayer = static_cast<int>(face);
            specularInvocation.targetMip = mip;
            specularInvocation.items.push_back(RenderItem { m_captureCube });
            renderer.getFrontend().pushPassInvocation(m_passSpecularFilterId, std::move(specularInvocation));
        }
        
    });


}

bool EnvironmentNode::loadEnvironmentMap(const std::string& path)
{
    if (path.empty() || !Project::doesPathExist(path)) {
        GLX_CORE_WARN("Environment map resource is not registered: '{0}'", path);
        return false;
    }
    try {
        const auto type = IResourceMaker::readType((std::filesystem::path(Project::getProjectRootPath()) / path).string());
        auto& manager = ResourceManager::getInstance();
        if (type == "Image") {
            setEnvironmentMap(manager.load<Image>(path));
            return true;
        }
        if (type == "CubemapResource") {
            setEnvironmentMap(manager.load<CubemapResource>(path));
            return true;
        }
        GLX_CORE_WARN("Environment map must be an Image or CubemapResource: '{0}'", path);
    } catch (const YAML::Exception& error) {
        GLX_CORE_WARN("Cannot read environment map '{0}': {1}", path, error.what());
    }
    return false;
}

} // namespace Galaxy
