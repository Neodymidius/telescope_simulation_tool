/*
Copyright (C) 2025  Neo Reinmann (neoreinmann@gmail.com)
*/

#ifndef SIXTE_EMBREESCENE_H
#define SIXTE_EMBREESCENE_H

#include <embree4/rtcore.h>
#include <optional>

#include "lib/stl_reader.h"
#include "geometry/Ray.h"

#include "sensor/Sensor.h"
#include "surface/SurfaceModel.h"
#include "shape/Spider.h"


class EmbreeScene {
public:
    EmbreeScene() = default;
    EmbreeScene(const EmbreeScene&) = default;
    EmbreeScene& operator=(const EmbreeScene&) = default;

    EmbreeScene(EmbreeScene&&) noexcept = default;
    EmbreeScene& operator=(EmbreeScene&&) noexcept = default;

    virtual ~EmbreeScene() = default;
    virtual void initializeScene() = 0;

    unsigned int addSTLMesh(const std::string& path, const Vec3fa& position);

    [[nodiscard]] virtual std::optional<Ray> ray_trace(Ray &ray) = 0;

    void initializeDevice();

    Sensor sensor;
    std::shared_ptr<SurfaceModel> surfaceModel = nullptr;
    Spider spider;

protected:
    RTCScene scene_{};
    RTCDevice device_{};

    virtual bool embree_ray_trace(Ray &ray, int depth) = 0;

    static void errorFunction(void* userPtr, enum RTCError error, const char* str);
    static bool reflect_ray(Ray &ray);

};


#endif //SIXTE_EMBREESCENE_H
