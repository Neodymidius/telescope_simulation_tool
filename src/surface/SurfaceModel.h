//
// Created by neo on 12/25/24.
//



#ifndef SURFACEMODEL_H
#define SURFACEMODEL_H

#include <memory>
#include "SurfaceStrategy.h"
#include "geometry/Ray.h"

class SurfaceModel {
public:
    explicit SurfaceModel(std::unique_ptr<SurfaceStrategy> surface_strategy)
        : surface_strategy_(std::move(surface_strategy)) {}
    bool simulate_surface(Ray &ray) const;
    void set_surface_parameter(std::string model, std::string shadowing, double factor, double shadowing_factor);
private:
    std::unique_ptr<SurfaceStrategy> surface_strategy_;
};



#endif //SURFACEMODEL_H
