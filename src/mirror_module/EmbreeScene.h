//
// Created by neo on 28.01.25.
//

#ifndef SIXTE_EMBREESCENE_H
#define SIXTE_EMBREESCENE_H

#include "shape/Shape.h"
#include "shape/Hyperboloid.h"
#include "shape/Paraboloid.h"
#include "shape/Plane.h"
#include "sensor/Sensor.h"
#include "shape/Spider.h"
#include "lib/stl_reader.h"
#include <embree4/rtcore.h>
#include <optional>

class EmbreeScene {
public:
    EmbreeScene();
    EmbreeScene(const EmbreeScene&) = default;
    EmbreeScene& operator=(const EmbreeScene&) = default;

    EmbreeScene(EmbreeScene&&) noexcept = default;
    EmbreeScene& operator=(EmbreeScene&&) noexcept = default;

    ~EmbreeScene() = default;

    [[nodiscard]] std::optional<Ray> ray_trace(Ray &ray);
    RTCScene initializeScene(RTCDevice device);
    static RTCDevice initializeDevice();
    static unsigned int addSTLMesh(const std::string& path, const Vec3fa& position, RTCScene& scene, RTCDevice& device);

    std::vector<Hyperboloid> hyperboloids{};
    std::vector<Paraboloid> paraboloids{};
    Spider spider{};
    Plane sensor;
    RTCScene scene;
    RTCDevice device;

private:
    static void errorFunction(void* userPtr, enum RTCError error, const char* str);
    bool embree_ray_trace(Ray &ray, int depth);
    std::shared_ptr<SurfaceModel> find_surface_model(unsigned int geomID);
    bool reflect_ray(Ray &ray);

    std::shared_ptr<SurfaceModel> surfaceModel = nullptr;

};


#endif //SIXTE_EMBREESCENE_H
