#pragma once
#include "uuid_v4.h"

namespace Galaxy {
using uuid = UUIDv4::UUID;

uuid uuidFromString(std::string str);

std::string uuidToString(uuid id);
}
