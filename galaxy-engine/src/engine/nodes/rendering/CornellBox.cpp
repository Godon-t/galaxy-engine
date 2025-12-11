#include "CornellBox.hpp"

#include "rendering/renderer/Renderer.hpp"
#include "resource/ResourceManager.hpp"

namespace Galaxy {

CornellBox::~CornellBox()
{
    auto& ri = Renderer::getInstance();
    for (int i = 0; i < 6; i++) {
        if (m_facesID[i])
            ri.clearMesh(m_facesID[i]);
        if (m_materialsID[i])
            ri.clearMaterial(m_materialsID[i]);
    }
}

void CornellBox::draw()
{
    auto& ri = Renderer::getInstance();
    for (int i = 0; i < 6; i++) {
        if (m_facesID[i] && m_materialsID[i]) {
            // Chaque face a sa propre transformation
            Transform faceTransform = m_transform;

            // Appliquer les transformations spécifiques à chaque face
            vec3 position(0);
            vec3 rotation(0);
            float size = 5.0f; // Taille de la Cornell Box

            switch (i) {
            case 0: // Sol (bas)
                position = vec3(0, -size, 0);
                rotation = vec3(radians(-90.0f), 0, 0);
                break;
            case 1: // Plafond (haut)
                position = vec3(0, size, 0);
                rotation = vec3(radians(90.0f), 0, 0);
                break;
            case 2: // Mur gauche (rouge)
                position = vec3(-size, 0, 0);
                rotation = vec3(0, radians(90.0f), 0);
                break;
            case 3: // Mur droit (vert)
                position = vec3(size, 0, 0);
                rotation = vec3(0, radians(-90.0f), 0);
                break;
            case 4: // Mur fond
                position = vec3(0, 0, -size);
                rotation = vec3(0, 0, 0);
                break;
            case 5: // Mur avant (optionnel, souvent omis)
                position = vec3(0, 0, size);
                rotation = vec3(0, radians(180.0f), 0);
                break;
            }

            faceTransform.translate(position);
            faceTransform.rotate(rotation);
            faceTransform.computeModelMatrix(m_transform.getGlobalModelMatrix());

            ri.submitPBR(m_facesID[i], m_materialsID[i], faceTransform);
        }
    }
}

void CornellBox::lightPassDraw()
{
    auto& ri = Renderer::getInstance();
    for (int i = 0; i < 6; i++) {
        if (m_facesID[i]) {
            Transform faceTransform;

            vec3 position(0);
            vec3 rotation(0);
            vec3 scale = m_transform.getLocalScale();

            float size = 5.0;

            switch (i) {
            case 0: // Sol
                position = vec3(0, -size, 0);
                rotation = vec3(radians(-90.0f), 0, 0);
                break;
            case 1: // Plafond
                position = vec3(0, size, 0);
                rotation = vec3(radians(90.0f), 0, 0);
                break;
            case 2: // Mur gauche
                position = vec3(-size, 0, 0);
                rotation = vec3(0, radians(90.0f), 0);
                break;
            case 3: // Mur droit
                position = vec3(size, 0, 0);
                rotation = vec3(0, radians(-90.0f), 0);
                break;
            case 4: // Mur fond
                position = vec3(0, 0, -size);
                rotation = vec3(0, 0, 0);
                break;
            case 5: // Mur avant
                position = vec3(0, 0, size);
                rotation = vec3(0, radians(180.0f), 0);
                break;
            }

            faceTransform.rotate(rotation);
            faceTransform.translate(position);
            faceTransform.computeModelMatrix(m_transform.getGlobalModelMatrix());

            ri.submit(m_facesID[i], faceTransform);
        }
    }
}

void CornellBox::accept(NodeVisitor& visitor)
{
    visitor.visit(*this);
}

void CornellBox::enteredRoot()
{
    auto& ri   = Renderer::getInstance();
    auto& rm   = ResourceManager::getInstance();
    float size = 5.0f; // Taille des quads (la moitié de la taille de la boîte)

    // Créer les 6 quads pour les faces
    for (int i = 0; i < 6; i++) {
        m_facesID[i] = ri.generateQuad(vec2(size * 2, size * 2), [] {});
    }

    // Créer les matériaux avec des couleurs Cornell Box classiques
    // Utiliser registerNewResource pour créer des matériaux en mémoire

    // Face 0: Sol - Blanc
    {
        Material mat;
        mat.setAlbedo(vec3(0.73f, 0.73f, 0.73f));
        mat.setMetallic(0.0f);
        mat.setRoughness(1.0f);
        mat.setTransparency(1.0f);
        auto materialHandle = rm.registerNewResource(mat, "internal://cornell_floor", true);
        m_materialsID[0]    = ri.instanciateMaterial(materialHandle);
    }

    // Face 1: Plafond - Blanc
    {
        Material mat;
        mat.setAlbedo(vec3(0.73f, 0.73f, 0.73f));
        mat.setMetallic(0.0f);
        mat.setRoughness(1.0f);
        mat.setTransparency(1.0f);
        auto materialHandle = rm.registerNewResource(mat, "internal://cornell_ceiling", true);
        m_materialsID[1]    = ri.instanciateMaterial(materialHandle);
    }

    // Face 2: Mur gauche - Rouge
    {
        Material mat;
        mat.setAlbedo(vec3(0.63f, 0.06f, 0.04f));
        mat.setMetallic(0.0f);
        mat.setRoughness(1.0f);
        mat.setTransparency(1.0f);
        auto materialHandle = rm.registerNewResource(mat, "internal://cornell_left", true);
        m_materialsID[2]    = ri.instanciateMaterial(materialHandle);
    }

    // Face 3: Mur droit - Vert
    {
        Material mat;
        mat.setAlbedo(vec3(0.15f, 0.48f, 0.09f));
        mat.setMetallic(0.0f);
        mat.setRoughness(1.0f);
        mat.setTransparency(1.0f);
        auto materialHandle = rm.registerNewResource(mat, "internal://cornell_right", true);
        m_materialsID[3]    = ri.instanciateMaterial(materialHandle);
    }

    // Face 4: Mur fond - Blanc
    {
        Material mat;
        mat.setAlbedo(vec3(0.73f, 0.73f, 0.73f));
        mat.setMetallic(0.0f);
        mat.setRoughness(1.0f);
        mat.setTransparency(1.0f);
        auto materialHandle = rm.registerNewResource(mat, "internal://cornell_back", true);
        m_materialsID[4]    = ri.instanciateMaterial(materialHandle);
    }

    // Face 5: Mur avant - Blanc (souvent omis dans une Cornell Box)
    {
        Material mat;
        mat.setAlbedo(vec3(0.73f, 0.73f, 0.73f));
        mat.setMetallic(0.0f);
        mat.setRoughness(1.0f);
        mat.setTransparency(1.0f);
        auto materialHandle = rm.registerNewResource(mat, "internal://cornell_front", true);
        m_materialsID[5]    = ri.instanciateMaterial(materialHandle);
    }
}

} // namespace Galaxy