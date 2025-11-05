/*
Copyright (C) 2025  Neo Reinmann (neoreinmann@gmail.com)
*/

#ifndef GAUSSSURFACE_H
#define GAUSSSURFACE_H

#include "geometry/Vec3fa.h"
#include "lib/random.h"
#include "SurfaceStrategy.h"


class GaussSurface final : public SurfaceStrategy {
public:
    explicit GaussSurface(const double factor) : factor_(factor) {};
    ~GaussSurface() override = default;
    bool simulate_surface(Ray &ray) const override;
    void set_surface_parameter(std::string model, std::string shadowing, double factor, double shadowing_factor) override;
private:
    double factor_;
};



#endif //GAUSSSURFACE_H
