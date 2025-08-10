#include "pch.hpp"

#include "Event.hpp"

namespace Galaxy {
bool Event::isCaptured()
{
    return m_captured;
}

void Event::capture()
{
    m_captured = true;
}
} // namespace Galaxy
