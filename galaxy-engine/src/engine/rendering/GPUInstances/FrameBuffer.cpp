#include "FrameBuffer.hpp"

#include "Core.hpp"
#include "gl_headers.hpp"
#include "rendering/OpenglHelper.hpp"

#include <fstream>

namespace Galaxy {
FrameBuffer::FrameBuffer()
    : FrameBuffer(2, 2)
{
}

FrameBuffer::FrameBuffer(int width, int height)
    : m_width(width)
    , m_height(height)
    , m_fbo(0)
{
    invalidate();
}

void FrameBuffer::bind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    // Réattacher les textures à chaque bind (nécessaire après resize/invalidate)
    std::vector<GLenum> attachments;
    for (const auto& pair : m_attachedColors) {
        if (pair.second && pair.second->getId() != 0) {
            attachments.push_back(GL_COLOR_ATTACHMENT0 + pair.first);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + pair.first,
                GL_TEXTURE_2D, pair.second->getId(), 0);
        }
    }

    if (m_attachedDepth && m_attachedDepth->getId() != 0) {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
            GL_TEXTURE_2D, m_attachedDepth->getId(), 0);
    }

    // Configurer les draw buffers
    if (!attachments.empty()) {
        glDrawBuffers(attachments.size(), attachments.data());
    } else if (m_attachedDepth) {
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
    }

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        GLX_CORE_ERROR("Framebuffer not complete after bind! Status: {}", status);
    }
}

void FrameBuffer::unbind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FrameBuffer::resize(unsigned int newWidth, unsigned int newHeight)
{
    if (m_width == newWidth && m_height == newHeight)
        return;

    m_width  = newWidth;
    m_height = newHeight;

    // Redimensionner toutes les textures attachées
    for (auto& pair : m_attachedColors) {
        if (pair.second) {
            pair.second->resize(m_width, m_height);
        }
    }

    if (m_attachedDepth) {
        m_attachedDepth->resize(m_width, m_height);
    }

    // Le FBO lui-même n'a pas besoin d'être recréé
}

void FrameBuffer::attachColorTexture(Texture& texture, int idx)
{
    texture.resize(m_width, m_height);
    m_attachedColors[idx] = &texture;

    // Immédiatement attacher si le FBO est déjà créé
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + idx,
        GL_TEXTURE_2D, texture.getId(), 0);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    GLX_CORE_ASSERT(status == GL_FRAMEBUFFER_COMPLETE,
        "Framebuffer not complete after attaching color texture");
}

void FrameBuffer::attachDepthTexture(Texture& texture)
{
    texture.resize(m_width, m_height);
    m_attachedDepth = &texture;

    // Immédiatement attacher si le FBO est déjà créé
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
        GL_TEXTURE_2D, texture.getId(), 0);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    GLX_CORE_ASSERT(status == GL_FRAMEBUFFER_COMPLETE,
        "Framebuffer not complete after attaching depth texture");
}

void FrameBuffer::setAsTextureUniform(unsigned int uniLocation, int textureIdx)
{
    if (textureIdx >= 0) {
        auto it = m_attachedColors.find(textureIdx);
        if (it != m_attachedColors.end() && it->second) {
            it->second->activate(uniLocation);
        }
    } else if (m_attachedDepth) {
        m_attachedDepth->activate(uniLocation);
    }
}

void FrameBuffer::invalidate()
{
    if (m_fbo != 0) {
        destroy();
    }

    // Créer uniquement le FBO, pas de textures
    glCreateFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    // Configuration par défaut : pas de draw buffer tant qu'aucune texture n'est attachée
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    unbind();
    checkOpenGLErrors("Frame buffer creation");
}

void FrameBuffer::destroy()
{
    if (m_fbo != 0) {
        glDeleteFramebuffers(1, &m_fbo);
        m_fbo = 0;
    }

    // Ne pas supprimer les textures - on ne les possède pas
    m_attachedColors.clear();
    m_attachedDepth = nullptr;
}

void FrameBuffer::savePPM(char* filename)
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    // Sauvegarder chaque color attachment
    for (const auto& pair : m_attachedColors) {
        if (!pair.second)
            continue;

        std::string outputPath = std::string(filename) + "_c" + std::to_string(pair.first) + ".ppm";
        std::ofstream output_image(outputPath.c_str());

        glReadBuffer(GL_COLOR_ATTACHMENT0 + pair.first);

        float* pixels = new float[m_width * m_height * 4];
        glReadPixels(0, 0, m_width, m_height, GL_RGBA, GL_FLOAT, pixels);

        output_image << "P3" << std::endl;
        output_image << m_width << " " << m_height << std::endl;
        output_image << "255" << std::endl;

        int k = 0;
        for (int i = 0; i < m_width; i++) {
            for (int j = 0; j < m_height; j++) {
                output_image << (unsigned int)(255 * pixels[k]) << " "
                             << (unsigned int)(255 * pixels[k + 1]) << " "
                             << (unsigned int)(255 * pixels[k + 2]) << " ";
                k = k + 4;
            }
            output_image << std::endl;
        }
        delete[] pixels;
        output_image.close();
    }

    // Sauvegarder la profondeur
    if (m_attachedDepth) {
        std::string outputPath = std::string(filename) + "_d.pgm";
        std::ofstream outputImage(outputPath.c_str());

        float* d_pixels = new float[m_width * m_height];
        glReadPixels(0, 0, m_width, m_height, GL_DEPTH_COMPONENT, GL_FLOAT, d_pixels);

        outputImage << "P2" << std::endl;
        outputImage << m_width << " " << m_height << std::endl;
        outputImage << "255" << std::endl;

        for (int y = 0; y < m_height; ++y) {
            for (int x = 0; x < m_width; ++x) {
                int k            = y * m_width + x;
                unsigned int val = (unsigned int)(255.0f * d_pixels[k]);
                outputImage << val << " ";
            }
            outputImage << std::endl;
        }
        delete[] d_pixels;
        outputImage.close();
    }
}

CubemapFrameBuffer::CubemapFrameBuffer()
    : CubemapFrameBuffer(512)
{
}

CubemapFrameBuffer::CubemapFrameBuffer(int size)
{
    m_size = size;
    m_depthCubemap.setFormat(TextureFormat::DEPTH);
    invalidate();
}

void CubemapFrameBuffer::attachDepthCubemap(Cubemap cubemap)
{
    m_depthCubemap = cubemap;
    m_depthCubemap.resize(m_size);
}

void CubemapFrameBuffer::attachColorCubemap(Cubemap cubemap, int idx)
{
    if (idx >= m_colorCubemaps.size())
        m_colorCubemaps.resize(idx + 1);

    m_colorCubemaps[idx] = cubemap;
    m_colorCubemaps[idx].resize(m_size);
}

void CubemapFrameBuffer::bind(int idx)
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    // TODO: Depend on cubemap mode: color, depth or both
    std::vector<GLenum> attachments;
    for (int i = 0; i < m_colorCubemaps.size(); i++) {
        if (m_colorCubemaps[i].cubemapID == 0)
            continue;
        attachments.push_back(GL_COLOR_ATTACHMENT0 + i);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_CUBE_MAP_POSITIVE_X + idx, m_colorCubemaps[i].cubemapID, 0);

        GLX_CORE_ASSERT(glIsTexture(m_colorCubemaps[i].cubemapID), "Cubemap attachement texture not valid");
    }

    if (m_depthCubemap.cubemapID) {
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_CUBE_MAP_POSITIVE_X + idx, m_depthCubemap.cubemapID, 0);
    }

    glDrawBuffers(attachments.size(), attachments.data());

    checkOpenGLErrors("Bind framebuffer face idx");
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    GLX_CORE_ASSERT(status == GL_FRAMEBUFFER_COMPLETE, "Cubemap framebuffer not complete after bind face {0}", idx);
}

void CubemapFrameBuffer::unbind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void CubemapFrameBuffer::destroy()
{
    glDeleteFramebuffers(1, &m_fbo);

    m_fbo = 0;
}

void CubemapFrameBuffer::resize(unsigned int newSize)
{
    m_size = newSize;
    invalidate();
}

void CubemapFrameBuffer::invalidate()
{
    if (m_fbo != 0)
        destroy();

    glGenFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glDrawBuffer(GL_NONE);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    checkOpenGLErrors("Cubemap frame buffer initialization");

    for (auto& cm : m_colorCubemaps) {
        cm.resize(m_size);
    }

    if (m_depthCubemap.cubemapID != 0)
        m_depthCubemap.resize(m_size);
}
}
