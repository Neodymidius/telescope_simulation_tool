/*
Copyright (C) 2025  Neo Reinmann (neoreinmann@gmail.com)
*/

#ifndef SIXTE_MIRRORMODULE_H
#define SIXTE_MIRRORMODULE_H


#include "geometry/Ray.h"
#include "lib/XMLData.h"
#include <memory>

class MirrorModule {
public:
    virtual ~MirrorModule() = default;
    [[nodiscard]] virtual std::unique_ptr<MirrorModule> clone() const = 0;
    virtual std::optional<Ray> ray_trace(Ray &ray) = 0;
    virtual void set_surface_parameter(std::string model, std::string shadowing, double factor, double shadowing_factor) = 0;
    virtual double get_focal_length() = 0;
private:
    virtual void create(XMLData xml_data) = 0;
};


#endif //SIXTE_MIRRORMODULE_H
