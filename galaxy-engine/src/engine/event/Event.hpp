#pragma once

namespace Galaxy {
// enum EventType {

// }

class Event {
private:
    bool m_captured = false;

public:
    bool isCaptured();
    void capture();
};
}
