#include <src/input/Input.hpp>

bool Input::isCaptured()
{
    return captured;
}

void Input::capture()
{
    captured = true;
}