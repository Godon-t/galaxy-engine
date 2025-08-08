#pragma once

class Input {
private:
    bool m_captured = false;

public:
    bool isCaptured();
    void capture();
};
