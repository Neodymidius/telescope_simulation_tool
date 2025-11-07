/*
Copyright (C) 2025  Neo Reinmann (neoreinmann@gmail.com)
*/

#include "SurfaceModel.h"

#include "Dummy.h"
#include "GaussSurface.h"
#include "Microfacet.h"


bool SurfaceModel::simulate_surface(Ray &ray) const
{
    return surface_strategy_->simulate_surface(ray);
}

void
SurfaceModel::set_surface_parameter(std::string model, std::string shadowing, double factor, double shadowing_factor) {
    return surface_strategy_->set_surface_parameter(model, shadowing, factor, shadowing_factor);
}

std::shared_ptr<SurfaceModel> SurfaceModel::get_surface_model(const std::string &model) {
    if (model == "gauss") {
        return std::make_unique<SurfaceModel>(std::make_unique<GaussSurface>(0));
    }
    if (model == "microfacet") {
        return std::make_unique<SurfaceModel>(std::make_unique<Microfacet>(0, 0, false, false));;
    }
    return std::make_unique<SurfaceModel>(std::make_unique<Dummy>());
}
