#include "FrameBuffer.hpp"

#include "Core.hpp"
#include "gl_headers.hpp"
#include "rendering/OpenglHelper.hpp"
#include "core/Log.hpp"

#include <fstream>

namespace Galaxy {
void bindColorAttachmentTexture(GLuint* id, int width, int height, GLenum internalFormat, GLenum format, int idx)
{
    glCreateTextures(GL_TEXTURE_2D, 1, id);
    glBindTexture(GL_TEXTURE_2D, *id);

    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + idx, GL_TEXTURE_2D, *id, 0);
}

void bindDepthAttachment(GLuint* id, int width, int height, GLenum format)
{
    glGenTextures(1, id);
    glBindTexture(GL_TEXTURE_2D, *id);

    // Decide pixel format and type based on requested internal format
    GLenum pixelFormat = GL_DEPTH_COMPONENT;
    GLenum pixelType   = GL_UNSIGNED_BYTE;

    if (format == GL_DEPTH24_STENCIL8) {
        pixelFormat = GL_DEPTH_STENCIL;
        pixelType   = GL_UNSIGNED_INT_24_8;
    } else if (format == GL_DEPTH_COMPONENT24) {
        pixelFormat = GL_DEPTH_COMPONENT;
        pixelType   = GL_UNSIGNED_INT;
    }

    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0,
        pixelFormat, pixelType, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
        GL_TEXTURE_2D, *id, 0);

    glBindTexture(GL_TEXTURE_2D, 0);
}

FrameBuffer::FrameBuffer()
    : FrameBuffer(2, 2, FramebufferTextureFormat::RGBA8)
{
}

FrameBuffer::FrameBuffer(int width, int height, FramebufferTextureFormat format)
{
    m_format      = format;
    m_width       = width;
    m_height      = height;
    m_fbo         = 0;
    m_colorsCount = 1;
    invalidate();
}
void FrameBuffer::bind()
{

    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
}

void FrameBuffer::unbind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FrameBuffer::resize(unsigned int newWidth, unsigned int newHeight)
{
    m_width  = newWidth;
    m_height = newHeight;
    invalidate();
}

void FrameBuffer::savePPM(char* filename)
{
    for (int i = 0; i < m_colorsCount; i++) {
        std::string outputPath = std::string(filename) + "_c" + std::to_string(i) + ".ppm";
        std::ofstream output_image(outputPath.c_str());

        /// READ THE CONTENT FROM THE FBO
        glReadBuffer(GL_COLOR_ATTACHMENT0 + i);
        float* pixels = new float[m_width * m_height * 4];
        glReadPixels(0, 0, m_width, m_height, GL_RGBA, GL_FLOAT, pixels);

        output_image << "P3" << std::endl;
        output_image << m_width << " " << m_height << std::endl;
        output_image << "255" << std::endl;

        int k = 0;
        for (int i = 0; i < m_width; i++) {
            for (int j = 0; j < m_height; j++) {
                output_image << (unsigned int)(255 * pixels[k]) << " " << (unsigned int)(255 * pixels[k + 1]) << " " << (unsigned int)(255 * pixels[k + 2]) << " ";
                k = k + 4;
            }
            output_image << std::endl;
        }
        delete[] pixels;
        output_image.close();
    }

    // If we have a depth component alongside color, save a _d.pgm file
    if ((m_format == FramebufferTextureFormat::DEPTH24RGBA8 || m_format == FramebufferTextureFormat::DEPTH24STENCIL8) && m_attachedDepth != 0) {
        std::string outputPath = std::string(filename) + "_d.pgm";
        std::ofstream outputImage(outputPath.c_str());

        /// READ THE DEPTH CONTENT FROM THE FBO
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

void FrameBuffer::attachColorTexture(Texture& texture, int idx)
{
    if (idx >= m_colorsCount) {
        GLX_CORE_ERROR("Can't bind texture to framebuffer's color attachment: {0}", idx);
        return;
    }

    unsigned int textureId = texture.getId();
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    if (!m_externalColors[idx])
        glDeleteTextures(1, &m_attachedColors[idx]);

    texture.resize(m_width, m_height);

    m_externalColors[idx] = true;
    m_attachedColors[idx] = texture.getId();
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + idx, GL_TEXTURE_2D, m_attachedColors[idx], 0);

    bool complete = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
    GLX_CORE_ASSERT(complete, "Framebuffer not complete after texture attach");
}

void FrameBuffer::attachDepthTexture(Texture& texture)
{
    unsigned int textureId = texture.getId();
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    if (!m_externalDepth)
        glDeleteTextures(1, &m_attachedDepth);

    m_externalDepth = true;

    texture.resize(m_width, m_height);
    texture.setFormat(TextureFormat::DEPTH);

    m_attachedDepth = texture.getId();

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_attachedDepth, 0);

    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    bool complete = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
    GLX_CORE_ASSERT(complete, "Framebuffer not complete after texture attach");
}

// TODO: Should work with texture bound
void FrameBuffer::setAsTextureUniform(unsigned int uniLocation, int textureIdx)
{
    if (textureIdx >= 0) {
        Texture tex;
        tex.m_id = m_attachedColors[textureIdx];
        tex.activate(uniLocation);
    } else {
        Texture tex;
        tex.m_id = m_attachedDepth;
        tex.activate(uniLocation);
    }
}

void FrameBuffer::invalidate()
{
    if (m_fbo != 0)
        destroy();

    glCreateFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    m_externalColors.resize(m_colorsCount);
    m_attachedColors.resize(m_colorsCount);

    // TODO: 3 cases, rgba, depth or rgba and depth
    if (m_format == FramebufferTextureFormat::RGBA8) {
        for (int i = 0; i < m_colorsCount; i++)
            bindColorAttachmentTexture(&m_attachedColors[i], m_width, m_height, GL_RGBA8, GL_RGBA, i);
    } else if (m_format == FramebufferTextureFormat::DEPTH24STENCIL8) {
        bindDepthAttachment(&m_attachedDepth, m_width, m_height, GL_DEPTH24_STENCIL8);
    } else if (m_format == FramebufferTextureFormat::DEPTH24RGBA8) {
        for (int i = 0; i < m_colorsCount; i++)
            bindColorAttachmentTexture(&m_attachedColors[i], m_width, m_height, GL_RGBA8, GL_RGBA, i);
        bindDepthAttachment(&m_attachedDepth, m_width, m_height, GL_DEPTH24_STENCIL8);
    } else if (m_format == FramebufferTextureFormat::DEPTH) {
        bindDepthAttachment(&m_attachedDepth, m_width, m_height, GL_DEPTH_COMPONENT24);
    } else {
        GLX_CORE_ASSERT(false, "Framebuffer format not handled");
    }

    if (m_format == FramebufferTextureFormat::DEPTH || m_format == FramebufferTextureFormat::DEPTH24STENCIL8) {
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
    }

    if (m_format == FramebufferTextureFormat::DEPTH24RGBA8 || m_format == FramebufferTextureFormat::RGBA8) {
        std::vector<GLenum> drawBuffers;
        for (int i = 0; i < m_colorsCount; i++) {
            drawBuffers.push_back(GL_COLOR_ATTACHMENT0 + i);
        }
        glDrawBuffers(drawBuffers.size(), drawBuffers.data());
    }

    bool complete = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
    GLX_CORE_ASSERT(complete, "Framebuffer not complete");

    unbind();
    checkOpenGLErrors("Frame buffer creation");
}

void FrameBuffer::destroy()
{
    glDeleteTextures(1, &m_attachedColors[0]);
    glDeleteTextures(1, &m_attachedDepth);
    glDeleteFramebuffers(1, &m_fbo);
    m_attachedColors[0] = 0;
    m_attachedDepth     = 0;
    m_fbo               = 0;
}

CubemapFrameBuffer::CubemapFrameBuffer()
    : CubemapFrameBuffer(512)
{
}

CubemapFrameBuffer::CubemapFrameBuffer(int size)
{
    m_depthCubemap.setFormat(TextureFormat::DEPTH);
    resize(size);
}

void CubemapFrameBuffer::attachDepthCubemap(Cubemap cubemap)
{
    m_depthCubemap = cubemap;
    m_depthCubemap.setFormat(TextureFormat::DEPTH);
    m_depthCubemap.resize(m_size);
}

void CubemapFrameBuffer::attachColorCubemap(Cubemap cubemap, int idx)
{
    if (idx >= m_colorCubemaps.size())
        m_colorCubemaps.resize(idx + 1);

    m_colorCubemaps[idx] = cubemap;
    m_colorCubemaps[idx].setFormat(TextureFormat::RGB);
    m_colorCubemaps[idx].resize(m_size);
}

void CubemapFrameBuffer::bind(int idx)
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    // TODO: Depend on cubemap mode: color, depth or both
    std::vector<GLenum> attachments(m_colorCubemaps.size());
    for (int i = 0; i < m_colorCubemaps.size(); i++) {
        attachments[i] = GL_COLOR_ATTACHMENT0 + i;
        glFramebufferTexture2D(GL_FRAMEBUFFER, attachments[i], GL_TEXTURE_CUBE_MAP_POSITIVE_X + idx, m_colorCubemaps[i].cubemapID, 0);
    }

    if (m_depthCubemap.cubemapID) {
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_CUBE_MAP_POSITIVE_X + idx, m_depthCubemap.cubemapID, 0);
    }

    glDrawBuffers(m_colorCubemaps.size(), attachments.data());

    checkOpenGLErrors("Bind framebuffer face idx");
    bool complete = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
    GLX_CORE_ASSERT(complete, "Cubemap framebuffer not complete after bind face {0}", idx);
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
    if (m_depthCubemap.cubemapID != 0)
        m_depthCubemap.resize(newSize);
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
}
}
