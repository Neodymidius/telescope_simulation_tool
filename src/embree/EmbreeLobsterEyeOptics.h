//
// Created by neo on 11/7/25.
//

#ifndef RAYTRACINGTOOLS_EMBREELOBSTEREYEOPTICS_H
#define RAYTRACINGTOOLS_EMBREELOBSTEREYEOPTICS_H

#include "embree/EmbreeScene.h"

#include "shape/MicroPoreOptics.h"


class EmbreeLobsterEyeOptics : public EmbreeScene {
public:
    EmbreeLobsterEyeOptics() = default;
    EmbreeLobsterEyeOptics(const EmbreeLobsterEyeOptics&) = default;
    EmbreeLobsterEyeOptics& operator=(const EmbreeLobsterEyeOptics&) = default;

    EmbreeLobsterEyeOptics(EmbreeLobsterEyeOptics&&) noexcept = default;
    EmbreeLobsterEyeOptics& operator=(EmbreeLobsterEyeOptics&&) noexcept = default;

    ~EmbreeLobsterEyeOptics() override = default;

    [[nodiscard]] std::optional<Ray> ray_trace(Ray &ray) override;
    void initializeScene() override;

    MicroPoreOptics micro_pore_optics;

private:
    std::shared_ptr<SurfaceModel> find_surface_model(unsigned int geomID);
    bool embree_ray_trace(Ray &ray, int depth) override;
};


#endif //RAYTRACINGTOOLS_EMBREELOBSTEREYEOPTICS_H