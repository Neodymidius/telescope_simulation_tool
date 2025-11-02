//
// Created by neo on 31.01.25.
//

#ifndef SIXTE_SENSOR_H
#define SIXTE_SENSOR_H

#include "geometry/Ray.h"

class Sensor {
public:
    Sensor();
    Sensor(std::string file_name, const Vec3fa& position);
    std::string filename;
    Vec3fa position;
    unsigned int geomID = -1;
};


#endif //SIXTE_SENSOR_H
