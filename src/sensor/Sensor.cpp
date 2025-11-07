/*
Copyright (C) 2025  Neo Reinmann (neoreinmann@gmail.com)
*/
#include "Sensor.h"

Sensor::Sensor() {
    filename = "";
    position = Vec3fa{0.f, 0.f, 0.f};
}

Sensor::Sensor(std::string file_name, const Vec3fa &position) {
    filename = file_name;
    Sensor::position = position;
}

Sensor::Sensor(Plane_parameters plane_parameters) {
    plane_ = Plane(plane_parameters);
}
