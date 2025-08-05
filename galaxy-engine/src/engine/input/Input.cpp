#include "pch.hpp"

#include "Input.hpp"

bool Input::isCaptured()
{
    return captured;
}

void Input::capture()
{
    captured = true;
}