//
// Created by neo on 21.02.24.
//


#ifndef SIXTE_RAY_H
#define SIXTE_RAY_H

#include "Vec3fa.h"
#include <embree4/rtcore.h>
#include <vector>

struct shape_id {
    shape_id(short id, const Vec3fa &origin, const Vec3fa &direction) : id(id), origin(origin), direction(direction) {}
    short id{};
    Vec3fa origin, direction;
};

class Ray {
public:
    Ray(const Vec3fa &position, Vec3fa &direction, double energy);

    [[nodiscard]] Vec3fa direction() const;
    [[nodiscard]] Vec3fa position() const;
    [[nodiscard]] Vec3fa normal() const;
    void set_direction(const Vec3fa& v);
    void set_position(const Vec3fa& v);
    void set_normal(const Vec3fa& v);

    double energy;
    std::vector<shape_id> raytracing_history{};
    RTCRayHit rayhit{};
};
#endif //SIXTE_RAY_H
