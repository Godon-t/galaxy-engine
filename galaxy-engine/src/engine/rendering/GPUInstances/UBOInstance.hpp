#pragma once

namespace Galaxy {
struct UBOInstance {
    unsigned int buffer = 0;
    size_t size         = 0;

    void init(size_t dataSize);
    void destroy();

    void bind(unsigned int idx);

    void update(const void* data, size_t dataSize);
};
} // namespace Galaxy
