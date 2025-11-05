/*
Copyright (C) 2025  Neo Reinmann (neoreinmann@gmail.com)
*/


#ifndef SIXTE_PARABOLOID_H
#define SIXTE_PARABOLOID_H
#if defined(RTC_NAMESPACE_USE)
RTC_NAMESPACE_USE
#endif

#include "geometry/Ray.h"
#include "surface/SurfaceModel.h"
#include "surface/GaussSurface.h"
#include "surface/Microfacet.h"
#include <embree4/rtcore.h>
#include <array>

struct Paraboloid_parameters {
    double p, theta, Yp_min, Xp_min, Xp_max, Yp_max;
    std::shared_ptr<SurfaceModel> surface;
    RTCGeometry geometry;
    unsigned int geomID;
    double angle_x, angle_y;
    Vec3fa origin = Vec3fa{0.f, 0.f, 0.f};
};

class Paraboloid {
    // class for paraboloid equation:
    // z = (x²+y²)/2p - p/2
public:
    double theta;
    double p;
    double Yp_min;
    double Xp_min;
    double Xp_max;
    double Yp_max;
    std::shared_ptr<SurfaceModel> surface;
    unsigned int geomID;
    Paraboloid_parameters paraboloid_parameters;

    explicit Paraboloid(Paraboloid_parameters paraboloid_parameters);

    static void paraboloidBoundsFunc(const RTCBoundsFunctionArguments *args);

    static void paraboloidIntersectFunc(const RTCIntersectFunctionNArguments *args);

    static void paraboloidOccludedFunc(const RTCOccludedFunctionNArguments *args);

};
#endif //SIXTE_PARABOLOID_H
