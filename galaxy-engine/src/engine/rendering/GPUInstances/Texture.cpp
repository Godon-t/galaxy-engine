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

void Cubemap::activate(int textureLocation)
{
    int actInt = Texture::getAvailableActivationInt();
    glActiveTexture(GL_TEXTURE0 + actInt);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapID);
    glUniform1i(textureLocation, actInt);
}

} // namespace Galaxy
