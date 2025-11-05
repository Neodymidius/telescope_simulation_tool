/*
Copyright (C) 2025  Neo Reinmann (neoreinmann@gmail.com)
*/

#ifndef SIXTE_PORE_H
#define SIXTE_PORE_H

#include "geometry/Vec3fa.h"
#include "geometry/Ray.h"
#include "Plane.h"
#include "lib/random.h"
#include "surface/SurfaceStrategy.h"


class Pore {
public:
    Pore();

    Pore(double pwidth, double plength, Vec3fa protation, Vec3fa ptranslation, std::string material_path, std::string material);

    void set_rotation(Vec3fa protation);

    void set_translation(Vec3fa ptranslation);

    void set_width(double width);

    void set_length(double length);

    bool ray_trace(Ray &ray, int depth);

    double generateRandomDouble(double m, double n);


private:
    double width, length;
    Vec3fa rotation{};
    Vec3fa translation{};
    Plane wall1, wall2, wall3, wall4, floor;


    int findInterection(Ray &ray);
    bool reflect_ray(Ray &ray);
};


#endif //SIXTE_PORE_H
