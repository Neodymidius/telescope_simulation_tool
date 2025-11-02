//
// Created by neo on 21.02.24.
//

#ifndef SIXTE_PLANE_H
#define SIXTE_PLANE_H

#include "geometry/Ray.h"
#include "sensor/Sensor.h"
#include <embree4/rtcore.h>

struct Plane_parameters {
    double a, b, c, d, sensor_x, sensor_y;
    RTCGeometry geometry;
    unsigned int geomID;
};


class Plane {
public:
    //parameters for the plane ax + b+y + cz + d = 0
    double a_, b_, c_, d_, sensor_x_, sensor_y_;
    Plane_parameters planeParameters{};

    Plane();

    explicit Plane(double a, double b, double c, double d, double sensor_x, double sensor_y);

    static void planeBoundsFunc(const RTCBoundsFunctionArguments *args);

    static void planeIntersectFunc(const RTCIntersectFunctionNArguments *args);

    static void planeOccludedFunc(const RTCOccludedFunctionNArguments *args);

    bool isOnSensor(const RTCRayHit& rayHit) const;

    double planeIntersect(Ray &rayhit) const;

};
#endif //SIXTE_PLANE_H
