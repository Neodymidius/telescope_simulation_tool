//
// Created by neo on 11/7/25.
//

#ifndef RAYTRACINGTOOLS_EMBREEWOLTER_H
#define RAYTRACINGTOOLS_EMBREEWOLTER_H

#include "embree/EmbreeScene.h"
#include "shape/Hyperboloid.h"
#include "shape/Paraboloid.h"
#include "shape/Spider.h"



class EmbreeWolter final : public EmbreeScene {
public:
    EmbreeWolter() = default;
    EmbreeWolter(const EmbreeWolter&) = default;
    EmbreeWolter& operator=(const EmbreeWolter&) = default;

    EmbreeWolter(EmbreeWolter&&) noexcept = default;
    EmbreeWolter& operator=(EmbreeWolter&&) noexcept = default;

    ~EmbreeWolter() override = default;

    [[nodiscard]] std::optional<Ray> ray_trace(Ray &ray) override;
    void initializeScene() override;

    std::vector<Hyperboloid> hyperboloids{};
    std::vector<Paraboloid> paraboloids{};
private:

    std::shared_ptr<SurfaceModel> find_surface_model(unsigned int geomID);
    bool embree_ray_trace(Ray &ray, int depth) override;

};


#endif //RAYTRACINGTOOLS_EMBREEWOLTER_H