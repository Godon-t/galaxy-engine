#pragma once

#include "Node.hpp"

namespace Galaxy {
std::unique_ptr<Node> constructNode(std::string& type);

}