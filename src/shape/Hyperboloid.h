/*
Copyright (C) 2025  Neo Reinmann (neoreinmann@gmail.com)
*/

#ifndef SIXTE_HYPERBOLOID_H
#define SIXTE_HYPERBOLOID_H

#include "geometry/Ray.h"
#include "surface/SurfaceModel.h"
#include "surface/GaussSurface.h"
#include <embree4/rtcore.h>

struct Hyperboloid_parameters {
    double a, b, c, Xh_max, Xh_min, Yh_max, Yh_min, theta;
    std::shared_ptr<SurfaceModel> surface;
    RTCGeometry geometry;
    unsigned int geomID;
    double angle_x, angle_y;
    Vec3fa origin = Vec3fa{0.f, 0.f, 0.f};
};


class Hyperboloid {
public:
    double a, b, c, Xh_max, Xh_min, Yh_max, Yh_min, theta;
    std::shared_ptr<SurfaceModel> surface;
    Hyperboloid_parameters hyperboloid_parameters{};
    unsigned int geomID;

    explicit Hyperboloid(Hyperboloid_parameters hyperboloid_parameters);

    static void hyperboloidBoundsFunc(const RTCBoundsFunctionArguments *args);

    static void hyperboloidIntersectFunc(const RTCIntersectFunctionNArguments *args);

    static void hyperboloidOccludedFunc(const RTCOccludedFunctionNArguments *args);
};


#endif //SIXTE_HYPERBOLOID_H
