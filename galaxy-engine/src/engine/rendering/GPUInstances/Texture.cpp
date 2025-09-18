#include "Texture.hpp"

#include "Log.hpp"
#include "rendering/OpenglHelper.hpp"

#include "gl_headers.hpp"

namespace Galaxy {
int Texture::s_currentFreeActivationInt = 0;

Texture::Texture(unsigned char* data, int width, int height, int nbChannels)
{
    init(data, width, height, nbChannels);
}

void Texture::init(unsigned char* data, int width, int height, int nbChannels)
{
    GLenum internalFormat = GL_RGB8;
    GLenum format         = GL_RGB;
    switch (nbChannels) {
    case 1:
        internalFormat = GL_R8;
        format         = GL_RED;
        break;
    case 3:
        internalFormat = GL_RGB8;
        format         = GL_RGB;
        break;
    case 4:
        internalFormat = GL_RGBA8;
        format         = GL_RGBA;
        break;
    default:
        GLX_CORE_ERROR("Warning: Unsupported texture format, defaulting to GL_RGB8\n");
    }

    glCreateTextures(GL_TEXTURE_2D, 1, &m_id);

    glTextureParameteri(m_id, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(m_id, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTextureParameteri(m_id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(m_id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTextureStorage2D(m_id, 1, internalFormat, width, height);
    glTextureSubImage2D(m_id, 0, 0, 0, width, height, format, GL_UNSIGNED_BYTE, data);
    glGenerateTextureMipmap(m_id);

    checkOpenGLErrors("Texture load");
}

void Texture::activate(int textureLocation)
{
    int actInt = getAvailableActivationInt();
    glActiveTexture(GL_TEXTURE0 + actInt);
    glBindTexture(GL_TEXTURE_2D, m_id);
    glUniform1i(textureLocation, actInt);
}

void Texture::destroy()
{
    glDeleteTextures(1, &m_id);
}

Cubemap::Cubemap()
{
    glGenTextures(1, &cubemapID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapID);
    resize(64);
}

void Cubemap::activate(unsigned int uniLoc)
{
    int actInt = Texture::getAvailableActivationInt();
    glActiveTexture(GL_TEXTURE0 + actInt);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapID);
    glUniform1i(uniLoc, actInt);
    checkOpenGLErrors("activate cubemap");
}

void Cubemap::destroy()
{
    if (cubemapID)
        glDeleteTextures(1, &cubemapID);
}

void Cubemap::allocateFaces(unsigned int res)
{
    resolution = res;
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapID);
    for (int i = 0; i < 6; i++) {
        if (useFloat)
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F,
                resolution, resolution, 0, GL_RGB, GL_FLOAT, nullptr);
        else
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB,
                resolution, resolution, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    }
    // paramétrage par défaut
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    checkOpenGLErrors("Allocate faces");
}

void Cubemap::resize(unsigned int res)
{
    if (res != resolution)
        allocateFaces(res);
}

} // namespace Galaxy
