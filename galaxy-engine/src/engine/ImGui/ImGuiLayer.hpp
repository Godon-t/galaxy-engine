#pragma once

#include "Layer.hpp"

namespace Galaxy {
class ImGuiLayer : public Layer {
public:
    ImGuiLayer()
        : Layer("ImGui")
    {
    }

    ~ImGuiLayer() override;

    void onAttach() override;
    void onDetach() override;
    void onUpdate() override;

    void onEvent(Event& evt) override;
};
}