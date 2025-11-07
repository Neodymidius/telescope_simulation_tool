/*
Copyright (C) 2025  Neo Reinmann (neoreinmann@gmail.com)
*/

#include "Plane.h"

Plane::Plane() : a_(0), b_(0), c_(0), d_(0), sensor_x_(0), sensor_y_(0) {
}

Plane::Plane(const Plane_parameters &parameters) : a_(parameters.a), b_(parameters.b), c_(parameters.c), d_(parameters.d),
                                            sensor_x_(parameters.sensor_x), sensor_y_(parameters.sensor_y), planeParameters(parameters)
{}

Plane::Plane(const double a, const double b, const double c, const double d, const double sensor_x = -1,
             const double sensor_y = -1) : a_(a), b_(b), c_(c), d_(d), sensor_x_(sensor_x), sensor_y_(sensor_y),
                                           planeParameters({a, b, c, d, sensor_x, sensor_y, 0, 0})
{}

void Plane::planeBoundsFunc(const RTCBoundsFunctionArguments *args) {
    const auto* para = (const Plane_parameters*) args->geometryUserPtr;
    RTCBounds* bounds = args->bounds_o;


    bounds->lower_x = (float) -para->sensor_x/2;
    bounds->lower_y = (float) -para->sensor_y/2;
    bounds->lower_z = (float) -para->d-10;

    bounds->upper_x = (float) para->sensor_x/2;
    bounds->upper_y = (float) para->sensor_y/2;
    bounds->upper_z = (float) -para->d;
}

void Plane::planeIntersectFunc(const RTCIntersectFunctionNArguments *args) {
    auto* rayhit = (RTCRayHit*) args->rayhit;
    RTCRay& ray = rayhit->ray;
    const auto* para  = (const Plane_parameters*) args->geometryUserPtr;

    double a_ = para->a;
    double b_ = para->b;
    double c_ = para->c;
    double d_ = para->d;

    double A = a_*ray.org_x + b_*ray.org_y + c_*ray.org_z + d_;
    double B = a_*ray.dir_x + b_*ray.dir_y + c_*ray.dir_z;
    // A + Bt = 0 -> Bt = -A -> t = -A/B
    double t = -A/B;
    if (t < ray.tnear || t > ray.tfar)
        return;
    ray.tfar = (float) t;

    // Set hit information.
    rayhit->hit.primID = para->geomID;
    rayhit->hit.geomID = para->geomID;


    float nx = (float) a_;
    float ny = (float) b_;
    float nz = (float) c_;

    float len = std::sqrt(nx * nx + ny * ny + nz * nz);

    if (len > 0.0f) {
        nx /= len;
        ny /= len;
        nz /= len;
    }

    rayhit->hit.Ng_x = -1.0*nx;
    rayhit->hit.Ng_y = -1.0*ny;
    rayhit->hit.Ng_z = -1.0*nz;
}

void Plane::planeOccludedFunc([[maybe_unused]] const RTCOccludedFunctionNArguments *args) {

}

double Plane::planeIntersect(Ray &rayhit) const {
    RTCRay& ray = rayhit.rayhit.ray;
    double A = a_*ray.org_x + b_*ray.org_y + c_*ray.org_z + d_;
    double B = a_*ray.dir_x + b_*ray.dir_y + c_*ray.dir_z;
    // A + Bt = 0 -> Bt = -A -> t = -A/B
    double t = -A/B;
    float nx = (float) a_;
    float ny = (float) b_;
    float nz = (float) c_;

    float len = std::sqrt(nx * nx + ny * ny + nz * nz);

    if (len > 0.0f) {
        nx /= len;
        ny /= len;
        nz /= len;
    }

    rayhit.rayhit.hit.Ng_x = -1.0*nx;
    rayhit.rayhit.hit.Ng_y = -1.0*ny;
    rayhit.rayhit.hit.Ng_z = -1.0*nz;
    return t;
}
