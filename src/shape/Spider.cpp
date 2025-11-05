/*
Copyright (C) 2025  Neo Reinmann (neoreinmann@gmail.com)
*/

#include "Spider.h"

#include <utility>

Spider::Spider() : filename(), position(){}

Spider::Spider(std::string  filename, const Vec3fa& position) : filename(std::move(filename)), position(position) {}


