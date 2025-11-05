/*
Copyright (C) 2025  Neo Reinmann (neoreinmann@gmail.com)
*/
//  Paper to be referenced for the math: "Geometries for Grazing Incidence Mirrors" by Pivovaroff et al. 2023
//  https://link.springer.com/10.1007/978-981-16-4544-0_2-1
//

#ifndef SIXTE_WOLTER_H
#define SIXTE_WOLTER_H

#include "mirror_module/MirrorModule.h"
#include "shape/Paraboloid.h"
#include "shape/Hyperboloid.h"
#include "mirror_module/EmbreeScene.h"

#include "shape/Shape.h"
#include "surface/SurfaceModel.h"
#include "surface/GaussSurface.h"
#include "surface/Microfacet.h"
#include "surface/Dummy.h"
#include "sensor/Sensor.h"
#include "shape/Plane.h"
#include "shape/Spider.h"


class Wolter final : public virtual MirrorModule {
public:
    double focal_length;
    double outer_radius;
    double inner_radius;
    int number_of_shells;

    ~Wolter() override = default;

    explicit Wolter(const XMLData& xml_data);

    Wolter(const Wolter& o) = default;

    Wolter(Wolter&& o) noexcept = default;

    [[nodiscard]] std::unique_ptr<MirrorModule> clone() const override {
        return std::make_unique<Wolter>(*this);
    }
    std::optional<Ray> ray_trace(Ray &ray) override;
    void set_surface_parameter(std::string model, std::string shadowing, double factor, double shadowing_factor) override;
    double get_focal_length() override;
private:
    double mirror_height;
    double distance_to_mirror;
    double sensor_offset;

    EmbreeScene shapes;

    void create_parameters(double new_radius, Paraboloid_parameters &p_pars, Hyperboloid_parameters &h_pars) const;
    void create(XMLData xml_data) override;
};


#endif //SIXTE_WOLTER_H
