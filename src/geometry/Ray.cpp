/*
Copyright (C) 2025  Neo Reinmann (neoreinmann@gmail.com)
*/
#include "Ray.h"

Ray::Ray(const Vec3fa &position, Vec3fa& direction, double energy)
: energy(energy) {
    if (not (direction.x == 0 and  direction.y == 0 and direction.z == 0)) {
        direction = normalize(direction);
    }
    rayhit.ray.org_x = position.x;
    rayhit.ray.org_y = position.y;
    rayhit.ray.org_z = position.z;
    rayhit.ray.dir_x = direction.x;
    rayhit.ray.dir_y = direction.y;
    rayhit.ray.dir_z = direction.z;
    rayhit.ray.tnear = 0.0001;
    rayhit.ray.tfar = std::numeric_limits<float>::infinity();
    rayhit.ray.mask = -1;
    rayhit.ray.flags = 0;
    rayhit.hit.geomID = RTC_INVALID_GEOMETRY_ID;
    rayhit.hit.instID[0] = RTC_INVALID_GEOMETRY_ID;
}

Vec3fa Ray::direction() const {
    return {rayhit.ray.dir_x, rayhit.ray.dir_y, rayhit.ray.dir_z};
}

Vec3fa Ray::position() const {
    return {rayhit.ray.org_x, rayhit.ray.org_y, rayhit.ray.org_z};
}

Vec3fa Ray::normal() const {
    return {rayhit.hit.Ng_x, rayhit.hit.Ng_y, rayhit.hit.Ng_z};
}

void Ray::set_direction(const Vec3fa &v) {
    rayhit.ray.dir_x = v.x;
    rayhit.ray.dir_y = v.y;
    rayhit.ray.dir_z = v.z;
}

void Ray::set_position(const Vec3fa &v) {
    rayhit.ray.org_x = v.x;
    rayhit.ray.org_y = v.y;
    rayhit.ray.org_z = v.z;
}

void Ray::set_normal(const Vec3fa &v) {
    rayhit.hit.Ng_x = v.x;
    rayhit.hit.Ng_y = v.y;
    rayhit.hit.Ng_z = v.z;
}
