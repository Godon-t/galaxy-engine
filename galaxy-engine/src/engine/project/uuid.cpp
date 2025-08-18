#include "uuid.hpp"

namespace Galaxy {
uuid uuidFromString(std::string str)
{
    return uuid(str);
}

std::string uuidToString(uuid id)
{
    return id.str();
}
}
