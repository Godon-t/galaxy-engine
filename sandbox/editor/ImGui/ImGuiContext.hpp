#pragma once

namespace Galaxy {
class ImGuiContext {
public:
    void onAttach();
    void onDetach();

    void begin();
    void end();
};
}