/*
Copyright (C) 2025  Neo Reinmann (neoreinmann@gmail.com)
*/

#ifndef SIXTE_SENSOR_H
#define SIXTE_SENSOR_H

#include "geometry/Ray.h"
#include "shape/Plane.h"

class Sensor {
public:
    Sensor();
    Sensor(std::string file_name, const Vec3fa& position);
    Sensor(Plane_parameters plane_parameters);
    std::string filename;
    Vec3fa position;
    unsigned int geomID = -1;
    Plane plane_;
};


#endif //SIXTE_SENSOR_H
