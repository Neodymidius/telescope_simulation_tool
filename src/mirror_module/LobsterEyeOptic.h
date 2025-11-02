//
// Created by neo on 25.08.25.
//

#ifndef SIXTE_LOBSTEREYEOPTIC_H
#define SIXTE_LOBSTEREYEOPTIC_H


#include "MirrorModule.h"
#include "shape/Spider.h"
#include "shape/OpticalMesh.h"
#include "shape/Plane.h"
#include "shape/Pore.h"

#include "EmbreeScene.h"

class LobsterEyeOptic final : public virtual MirrorModule {
public:
    explicit LobsterEyeOptic(const XMLData& xml_data);

    ~LobsterEyeOptic() override = default;

    LobsterEyeOptic(const LobsterEyeOptic &o) = default;

    LobsterEyeOptic(LobsterEyeOptic&& o) noexcept = default;

    [[nodiscard]] std::unique_ptr<MirrorModule> clone() const override {
        return std::make_unique<LobsterEyeOptic>(*this);
    }

    std::optional<Ray> ray_trace(Ray& ray) override;

    void set_surface_parameter(std::string model, std::string shadowing, double factor, double shadowing_factor) override;
    double get_focal_length() override;
private:
    Spider spider;
    OpticalMesh opticalMesh;
    Plane sensor;
    Sensor mesh_sensor;
    Pore pore;

    double focal_length;

    RTCScene scene;
    RTCDevice device;

    RTCScene initializeScene(RTCDevice device);
    bool embree_ray_trace(Ray &ray, int depth);
    void create(XMLData xml_data) override;

};


#endif //SIXTE_LOBSTEREYEOPTIC_H
