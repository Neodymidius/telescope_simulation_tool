//
// Created by neo on 11/7/25.
//

#include "EmbreeLobsterEyeOptics.h"

std::optional<Ray> EmbreeLobsterEyeOptics::ray_trace(Ray &ray) {
    if(embree_ray_trace(ray, 5)) {
        return ray;
    }
    return std::nullopt;
}

void EmbreeLobsterEyeOptics::initializeScene() {
    initializeDevice();
    scene_ = rtcNewScene(device_);
    rtcSetSceneFlags(scene_, RTC_SCENE_FLAG_ROBUST);
    rtcSetSceneBuildQuality(scene_, RTC_BUILD_QUALITY_HIGH);

    if (!sensor.filename.empty()) {
        sensor.geomID = EmbreeScene::addSTLMesh(sensor.filename, sensor.position);
    } else {
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
        spider.geomID = EmbreeScene::addSTLMesh(spider.filename, spider.position);
    micro_pore_optics.geomID = EmbreeScene::addSTLMesh(micro_pore_optics.filename, micro_pore_optics.position);
    rtcCommitScene(scene_);
}

bool EmbreeLobsterEyeOptics::embree_ray_trace(Ray &ray, int depth) {
    while (depth > 0) {
        // Intersect
        rtcIntersect1(scene_, &ray.rayhit);
        Vec3fa normal = Vec3fa(ray.rayhit.hit.Ng_x, ray.rayhit.hit.Ng_y, ray.rayhit.hit.Ng_z);
        ray.set_normal(normalize(normal));


        if (ray.rayhit.hit.geomID == RTC_INVALID_GEOMETRY_ID)
            return false;

        ray.raytracing_history.emplace_back((short) ray.rayhit.hit.geomID,
                                            ray.position(),
                                            ray.direction());
        // Check if sensor was hit
        if (ray.rayhit.hit.geomID == sensor.geomID) {
            if (depth == 5) {
                return false;
            }
            ray.set_position(ray.position() + ray.rayhit.ray.tfar * ray.direction());
            return true;
        }

        if (ray.rayhit.hit.geomID == spider.geomID) {
            return false;
        }
        if(ray.rayhit.hit.geomID == micro_pore_optics.geomID) {
            ray.set_position(ray.position() + ray.rayhit.ray.tfar * ray.direction());
            if(!micro_pore_optics.pore.ray_trace(ray, depth)){
                return false;
            }
        }

        // Decrease depth
        depth--;
    }
    return false;
}
