//
// Created by neo on 11/7/25.
//

#include "EmbreeWolter.h"

std::shared_ptr<SurfaceModel> EmbreeWolter::find_surface_model(unsigned int geomID) {
    for (auto const &hyperboloid : hyperboloids) {
        if (hyperboloid.geomID == geomID)
            return hyperboloid.surface;
    }
    for (auto const &paraboloid : paraboloids) {
        if (paraboloid.geomID == geomID)
            return paraboloid.surface;
    }
    return nullptr;
}

bool EmbreeWolter::embree_ray_trace(Ray &ray, int depth) {
    while (depth > 0) {
        // Intersect
        rtcIntersect1(scene_, &ray.rayhit);

        if (ray.rayhit.hit.geomID == RTC_INVALID_GEOMETRY_ID)
            return false;

        ray.raytracing_history.emplace_back((short) ray.rayhit.hit.geomID,
                                             ray.position(),
                                             ray.direction());
        // Check if sensor was hit
        if (ray.rayhit.hit.geomID == sensor.geomID) {
            if (depth == 4) {
                return false;
            }
            ray.set_position(ray.position() + ray.rayhit.ray.tfar * ray.direction());
            return true;
        }

        // Check if spider was hit
        if (ray.rayhit.hit.geomID == spider.geomID) {
            return false;
        }

        // Add roughness if there is any
        surfaceModel = find_surface_model(ray.rayhit.hit.geomID);
        if (surfaceModel != nullptr)
            if(!surfaceModel->simulate_surface(ray))
                return false;
        // Reflect ray
        if(!reflect_ray(ray))
            return false;
        // Decrease depth
        depth--;
    }
    return false;
}

std::optional<Ray> EmbreeWolter::ray_trace(Ray &ray) {
    if(embree_ray_trace(ray, 4)) {
        return ray;
    }
    return std::nullopt;
}

void EmbreeWolter::initializeScene() {
    initializeDevice();
    scene_ = rtcNewScene(device_);
    rtcSetSceneFlags(scene_, RTC_SCENE_FLAG_ROBUST);
    rtcSetSceneBuildQuality(scene_, RTC_BUILD_QUALITY_HIGH);
    for  ( auto & paraboloid : paraboloids) {
        RTCGeometry geometry = rtcNewGeometry(device_, RTC_GEOMETRY_TYPE_USER);
        auto* para = &paraboloid.paraboloid_parameters;

        rtcSetGeometryUserPrimitiveCount(geometry,1);
        rtcSetGeometryUserData(geometry,para);
        para->geometry = geometry;

        rtcSetGeometryBoundsFunction(geometry, Paraboloid::paraboloidBoundsFunc, nullptr);
        rtcSetGeometryIntersectFunction(geometry, Paraboloid::paraboloidIntersectFunc);
        rtcSetGeometryOccludedFunction(geometry, Paraboloid::paraboloidOccludedFunc);

        // Commit the geometry and attach it to the scene.
        rtcCommitGeometry(geometry);
        para->geomID = rtcAttachGeometry(scene_,geometry);
        paraboloid.geomID = para->geomID;
        rtcReleaseGeometry(geometry);
    }

    for  ( auto & hyperboloid : hyperboloids) {
        RTCGeometry geometry = rtcNewGeometry(device_, RTC_GEOMETRY_TYPE_USER);
        auto* para = &hyperboloid.hyperboloid_parameters;

        rtcSetGeometryUserPrimitiveCount(geometry,1);
        rtcSetGeometryUserData(geometry,para);
        para->geometry = geometry;

        rtcSetGeometryBoundsFunction(geometry, Hyperboloid::hyperboloidBoundsFunc, nullptr);
        rtcSetGeometryIntersectFunction(geometry, Hyperboloid::hyperboloidIntersectFunc);
        rtcSetGeometryOccludedFunction(geometry, Hyperboloid::hyperboloidOccludedFunc);

        // Commit the geometry and attach it to the scene.
        rtcCommitGeometry(geometry);
        para->geomID = rtcAttachGeometry(scene_,geometry);
        hyperboloid.geomID = para->geomID;
        rtcReleaseGeometry(geometry);
    }
    {
        RTCGeometry geometry = rtcNewGeometry(device_, RTC_GEOMETRY_TYPE_USER);
        auto *para = &sensor.plane_.planeParameters;

        rtcSetGeometryUserPrimitiveCount(geometry, 1);
        rtcSetGeometryUserData(geometry, para);
        para->geometry = geometry;

        rtcSetGeometryBoundsFunction(geometry, Plane::planeBoundsFunc, nullptr);
        rtcSetGeometryIntersectFunction(geometry, Plane::planeIntersectFunc);
        rtcSetGeometryOccludedFunction(geometry, Plane::planeOccludedFunc);

        // Commit the geometry and attach it to the scene.
        rtcCommitGeometry(geometry);
        para->geomID = rtcAttachGeometry(scene_, geometry);
        sensor.geomID = para->geomID;
        rtcReleaseGeometry(geometry);
    }
    if (!spider.filename.empty())
        spider.geomID = addSTLMesh(spider.filename, spider.position);
    rtcCommitScene(scene_);
}
