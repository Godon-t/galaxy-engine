#pragma once

class Input {
private:
    bool captured = false;
public:
    bool isCaptured();
    void capture();
};
