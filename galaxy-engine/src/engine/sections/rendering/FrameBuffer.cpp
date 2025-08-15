#include "FrameBuffer.hpp"

#include "Core.hpp"
#include "OpenglHelper.hpp"
#include "gl_headers.hpp"

namespace Galaxy {
void bindColorAttachmentTexture(GLuint* id, int width, int height, GLenum internalFormat, GLenum format)
{
    glCreateTextures(GL_TEXTURE_2D, 1, id);
    glBindTexture(GL_TEXTURE_2D, *id);

    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, *id, 0);
}

FrameBuffer::FrameBuffer(int width, int height, FramebufferTextureFormat format)
{
    m_format = format;
    m_width  = width;
    m_height = height;
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

void FrameBuffer::invalidate()
{
    if (m_fbo != 0)
        destroy();

    glCreateFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    if (m_format == FramebufferTextureFormat::RGBA8) {
        bindColorAttachmentTexture(&m_attachedColor, m_width, m_height, GL_RGBA8, GL_RGBA);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_attachedColor, 0);
        bool complete = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
        GLX_CORE_ASSERT(complete, "Framebuffer not complete");
    } else {
        GLX_CORE_ASSERT(false, "Framebuffer format not handled");
    }

    checkOpenGLErrors("Frame buffer creation");
}

void FrameBuffer::destroy()
{
    glDeleteTextures(1, &m_attachedColor);
    glDeleteFramebuffers(1, &m_fbo);
}
}
