//
// Created by neo on 12/31/24.
//

#include "GaussSurface.h"


bool GaussSurface::simulate_surface(Ray &ray) const
{
    Vec3fa new_normal ={static_cast<float>(ray.normal().x + ray.normal().x * factor_ * easy_uniform_random()),
               static_cast<float>(ray.normal().y + ray.normal().y * factor_ * easy_uniform_random()),
               static_cast<float>(ray.normal().z + ray.normal().z * factor_ * easy_uniform_random())};
    ray.set_normal(new_normal);
    return true;
}

void
GaussSurface::set_surface_parameter(std::string model, std::string shadowing, double factor, double shadowing_factor) {

}
