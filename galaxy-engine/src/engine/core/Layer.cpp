#include "Layer.hpp"

namespace Galaxy {
Galaxy::Layer::Layer(const std::string& name)
    : m_debugName(name)
{
}

Layer::~Layer() { }
}