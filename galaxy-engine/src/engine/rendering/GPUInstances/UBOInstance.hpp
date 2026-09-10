#pragma once

namespace Galaxy {
class UBOInstance {
public:
    UBOInstance() = default;
    ~UBOInstance();

    UBOInstance(const UBOInstance&)            = delete;
    UBOInstance& operator=(const UBOInstance&) = delete;
    UBOInstance(UBOInstance&& other) noexcept;
    UBOInstance& operator=(UBOInstance&& other) noexcept;

    void init(size_t dataSize);
    void destroy();

    void bind(unsigned int idx);

    void update(const void* data, size_t dataSize);

private:
    unsigned int m_buffer = 0;
    size_t m_size         = 0;
};
} // namespace Galaxy
