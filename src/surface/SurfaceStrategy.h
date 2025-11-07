/*
Copyright (C) 2025  Neo Reinmann (neoreinmann@gmail.com)
*/

#ifndef SURFACESTRATEGY_H
#define SURFACESTRATEGY_H

#include "geometry/Ray.h"

class SurfaceStrategy {
public:
    virtual ~SurfaceStrategy() = default;
    virtual bool simulate_surface(Ray & ray) const = 0;
    virtual void set_surface_parameter(std::string model, std::string shadowing, double factor, double shadowing_factor) = 0;
};



#endif //SURFACESTRATEGY_H
