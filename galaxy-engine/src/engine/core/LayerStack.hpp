#pragma once

#include "Layer.hpp"

namespace Galaxy {
class LayerStack {
public:
    LayerStack();
    ~LayerStack();

    LayerStack(const LayerStack&)            = delete;
    LayerStack& operator=(const LayerStack&) = delete;
    LayerStack(LayerStack&&)                 = delete;
    LayerStack& operator=(LayerStack&&)      = delete;

    void pushLayer(Layer* layer);
    void pushOverlay(Layer* overlay);
    void popLayer(Layer* layer);
    void popOverlay(Layer* overlay);
    void clear();

    std::vector<Layer*>::iterator begin() { return m_layers.begin(); }
    std::vector<Layer*>::iterator end() { return m_layers.end(); }

private:
    std::vector<Layer*> m_layers;
    std::vector<Layer*>::iterator m_layerInsert; // points to the last layer right before overlays
};
}
