//
// Created by neo on 07.11.24.
//

#include "Spider.h"

#include <utility>

Spider::Spider() : filename(), position(){}

Spider::Spider(std::string  filename, const Vec3fa& position) : filename(std::move(filename)), position(position) {}


