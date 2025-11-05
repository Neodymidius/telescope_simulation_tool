/*
Copyright (C) 2025  Neo Reinmann (neoreinmann@gmail.com)
*/

#include "SurfaceModel.h"


bool SurfaceModel::simulate_surface(Ray &ray) const
{
    return surface_strategy_->simulate_surface(ray);
}

void
SurfaceModel::set_surface_parameter(std::string model, std::string shadowing, double factor, double shadowing_factor) {
    return surface_strategy_->set_surface_parameter(model, shadowing, factor, shadowing_factor);
}
