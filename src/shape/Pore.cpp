/*
Copyright (C) 2025  Neo Reinmann (neoreinmann@gmail.com)
*/

#include <iostream>
#include "Pore.h"

Pore::Pore() {

}

Pore::Pore(double pwidth, double plength, Vec3fa protation, Vec3fa ptranslation,  [[maybe_unused]] std::string material_path, [[maybe_unused]] std::string material) {
    width = pwidth;
    length = plength;
    rotation = protation;
    translation = ptranslation;
    wall1 = Plane(0, 1, 0, 0, 0, 0);
    wall2 = Plane(1, 0, 0, -width, 0, 0);
    wall3 = Plane(0, 1, 0, -width, 0, 0);
    wall4 = Plane(1, 0, 0, 0, 0, 0);
    floor = Plane(0, 0, 1, 0, 0, 0);

}

void Pore::set_rotation(Vec3fa protation) {
    rotation = protation;
}

void Pore::set_translation(Vec3fa ptranslation) {
    translation = ptranslation;
}

void Pore::set_width(double pwidth) {
    width = pwidth;
}

void Pore::set_length(double plength) {
    length = plength;
}

int Pore::findInterection(Ray &ray) {
    double t = std::numeric_limits<double>::infinity();
    int wall_number = -1;
    Vec3fa dir = ray.direction();
    Vec3fa pos = ray.position();

    double t_temp = wall1.planeIntersect(ray);
    Vec3fa hit = pos + t_temp * dir;
    if (!(hit.z < 0 || hit.z > length || hit.x < 0 || hit.x > width || t_temp > t || t_temp < ray.rayhit.ray.tnear)) {
        t = t_temp;
        wall_number = 1;
    }

    t_temp = wall2.planeIntersect(ray);
    hit = pos + t_temp * dir;
    if (!(hit.z < 0 || hit.z > length || hit.y < 0 || hit.y > width || t_temp > t || t_temp < ray.rayhit.ray.tnear)) {
        t = t_temp;
        wall_number = 2;
    }

    t_temp = wall3.planeIntersect(ray);
    hit = pos + t_temp * dir;
    if (!(hit.z < 0 || hit.z > length || hit.x < 0 || hit.x > width || t_temp > t || t_temp < ray.rayhit.ray.tnear)) {
        t = t_temp;
        wall_number = 3;
    }

    t_temp = wall4.planeIntersect(ray);
    hit = pos + t_temp * dir;
    if (!(hit.z < 0 || hit.z > length || hit.y < 0 || hit.y > width || t_temp > t || t_temp < ray.rayhit.ray.tnear)) {
        t = t_temp;
        wall_number = 4;
    }

    t_temp = floor.planeIntersect(ray);
    hit = pos + t_temp * dir;
    if (!(hit.x < 0 || hit.x > width || hit.y < 0 || hit.y > width || t_temp > t || t_temp < ray.rayhit.ray.tnear)) {
        t = t_temp;
        wall_number = 5;
    }

    ray.rayhit.ray.tfar = (float) t;
    switch (wall_number) {
        case 1:
            ray.set_normal(Vec3fa(0, 1, 0));
            break;
        case 2:
            ray.set_normal(Vec3fa(-1, 0, 0));
            break;
        case 3:
            ray.set_normal(Vec3fa(0, -1, 0));
            break;
        case 4:
            ray.set_normal(Vec3fa(1, 0, 0));
            break;
        case 5:
            ray.set_normal(Vec3fa(0, 0, 1));
            break;

    }
    return wall_number;
}

double Pore::generateRandomDouble(double m, double n) {
    double uniform_number = easy_uniform_random();
    return m + (n-m) * uniform_number;
}

bool Pore::ray_trace(Ray &ray, int depth) {
    // float theta = get_angle(ray.normal(), {0, 0, 1});
    Vec3fa hit = ray.position();
    Vec3fa normal_exact = normalize(hit);

    double alpha = atan2(-normal_exact.y, sqrt(normal_exact.x*normal_exact.x + normal_exact.z*normal_exact.z));
    double beta = atan2(normal_exact.x, normal_exact.z);
    std::array<Vec3fa,3> R_x = {Vec3fa(1,0,0), Vec3fa(0, cos(alpha), sin(alpha)), Vec3fa(0,-sin(alpha), cos(alpha))};
    std::array<Vec3fa,3> R_y = {Vec3fa(cos(beta),0,-sin(beta)), Vec3fa(0,1,0), Vec3fa(sin(beta),0, cos(beta))};
    auto R = R_y*R_x;
    Vec3fa transformed_incoming = transpose(R)*ray.direction();
    ray.set_direction(normalize(transformed_incoming));

    Vec3fa old_position = ray.position();
    double x = generateRandomDouble(width, 0);
    double y = generateRandomDouble(width, 0);
    ray.set_position(Vec3fa(x, y, length));

    depth = 10;
    while (depth > 0) {
        int wall_number = findInterection(ray);

        if (wall_number == -1) {
            return false;
        }
        ray.raytracing_history.emplace_back((short) wall_number+10,
                                            ray.position(),
                                            ray.direction());

        if (wall_number == 5) {
            old_position = old_position - normal_exact * length;
            ray.set_position(old_position);

            transformed_incoming = R*ray.direction();
            ray.set_direction(normalize(transformed_incoming));
            ray.rayhit.ray.tnear = 20;
            ray.rayhit.ray.tfar = std::numeric_limits<float>::infinity();
            ray.rayhit.ray.mask = -1;
            ray.rayhit.ray.flags = 0;
            ray.rayhit.hit.geomID = RTC_INVALID_GEOMETRY_ID;
            ray.rayhit.hit.primID = 0;
            ray.rayhit.hit.instID[0] = RTC_INVALID_GEOMETRY_ID;
            return true;
        }

        // Reflectivity
        if(get_angle(-1*ray.direction(), ray.normal()) < 0.00000001)
            return false;

        //TODO: surface roughness here before reflection.
        reflect_ray(ray);

        depth--;
    }
    return false;
}

bool Pore::reflect_ray(Ray &ray) {
    ray.set_position(ray.position() + ray.rayhit.ray.tfar * ray.direction());
    ray.set_direction(reflect(ray.direction(), ray.normal()));
    ray.rayhit.ray.tnear = 0.0001;
    ray.rayhit.ray.tfar = std::numeric_limits<float>::infinity();
    ray.rayhit.ray.mask = -1;
    ray.rayhit.ray.flags = 0;
    return true;
}

