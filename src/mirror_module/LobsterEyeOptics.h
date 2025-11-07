/*
Copyright (C) 2025  Neo Reinmann (neoreinmann@gmail.com)
*/

#ifndef SIXTE_LOBSTEREYEOPTIC_H
#define SIXTE_LOBSTEREYEOPTIC_H


#include "MirrorModule.h"
#include "embree/EmbreeLobsterEyeOptics.h"
#include "shape/Plane.h"
#include "shape/Pore.h"
#include "surface/SurfaceModel.h"

#include "embree/EmbreeScene.h"

class LobsterEyeOptics final : public virtual MirrorModule {
public:
    explicit LobsterEyeOptics(const XMLData& xml_data);

    ~LobsterEyeOptics() override = default;

    LobsterEyeOptics(const LobsterEyeOptics &o) = default;

    LobsterEyeOptics(LobsterEyeOptics&& o) noexcept = default;

    [[nodiscard]] std::unique_ptr<MirrorModule> clone() const override {
        return std::make_unique<LobsterEyeOptics>(*this);
    }

    std::optional<Ray> ray_trace(Ray& ray) override;

    void set_surface_parameter(std::string model, std::string shadowing, double factor, double shadowing_factor) override;
    double get_focal_length() override;
private:
    double focal_length;

    EmbreeLobsterEyeOptics embree_scene_;

    void create(XMLData xml_data) override;

};


#endif //SIXTE_LOBSTEREYEOPTIC_H
