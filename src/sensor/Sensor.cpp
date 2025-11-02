//
// Created by neo on 31.01.25.
//

#include "Sensor.h"

Sensor::Sensor() {
    filename = "";
    position = Vec3fa{0.f, 0.f, 0.f};
}

Sensor::Sensor(std::string file_name, const Vec3fa &position) {
    filename = file_name;
    Sensor::position = position;
}
