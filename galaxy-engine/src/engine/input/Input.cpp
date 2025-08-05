#include "pch.hpp"

#include "Input.hpp"

bool Input::isCaptured()
{
    return m_captured;
}

void Input::capture()
{
    m_captured = true;
}