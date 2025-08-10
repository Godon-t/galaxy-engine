#pragma once

#include "event/Event.hpp"
#include "pch.hpp"

namespace Galaxy {
class Layer {
public:
    Layer(const std::string& name = "Layer");
    virtual ~Layer();

    virtual void onAttach() { }
    virtual void onDetach() { }
    virtual void onUpdate() { }
    virtual void onEvent(Event& evt) { }

    inline const std::string& getName() const { return m_debugName; }

protected:
    std::string m_debugName;
};
}