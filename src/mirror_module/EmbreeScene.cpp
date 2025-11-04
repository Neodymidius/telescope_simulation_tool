//
// Created by neo on 28.01.25.
//

#include "EmbreeScene.h"

std::optional<Ray> EmbreeScene::ray_trace(Ray &ray) {
    if(embree_ray_trace(ray, 4)) {
        return ray;
    }
    return std::nullopt;
}

bool EmbreeScene::embree_ray_trace(Ray &ray, int depth) {
     while (depth > 0) {
        // Intersect
        rtcIntersect1(scene, &ray.rayhit);

        if (ray.rayhit.hit.geomID == RTC_INVALID_GEOMETRY_ID)
            return false;

        ray.raytracing_history.emplace_back((short) ray.rayhit.hit.geomID,
                                             ray.position(),
                                             ray.direction());
        // Check if sensor was hit
        if (sensor.isOnSensor(ray.rayhit)) {
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

EmbreeScene::EmbreeScene() {
    device = initializeDevice();
}

RTCScene EmbreeScene::initializeScene(RTCDevice device)
{

    RTCScene scene = rtcNewScene(device);
    rtcSetSceneFlags(scene, RTC_SCENE_FLAG_ROBUST);
    rtcSetSceneBuildQuality(scene, RTC_BUILD_QUALITY_HIGH);
    for  ( auto & paraboloid : paraboloids) {
        RTCGeometry geometry = rtcNewGeometry(device, RTC_GEOMETRY_TYPE_USER);
        auto* para = &paraboloid.paraboloid_parameters;

        rtcSetGeometryUserPrimitiveCount(geometry,1);
        rtcSetGeometryUserData(geometry,para);
        para->geometry = geometry;

        rtcSetGeometryBoundsFunction(geometry, Paraboloid::paraboloidBoundsFunc, nullptr);
        rtcSetGeometryIntersectFunction(geometry, Paraboloid::paraboloidIntersectFunc);
        rtcSetGeometryOccludedFunction(geometry, Paraboloid::paraboloidOccludedFunc);

        // Commit the geometry and attach it to the scene.
        rtcCommitGeometry(geometry);
        para->geomID = rtcAttachGeometry(scene,geometry);
        paraboloid.geomID = para->geomID;
        rtcReleaseGeometry(geometry);
    }

    for  ( auto & hyperboloid : hyperboloids) {
        RTCGeometry geometry = rtcNewGeometry(device, RTC_GEOMETRY_TYPE_USER);
        auto* para = &hyperboloid.hyperboloid_parameters;

        rtcSetGeometryUserPrimitiveCount(geometry,1);
        rtcSetGeometryUserData(geometry,para);
        para->geometry = geometry;

        rtcSetGeometryBoundsFunction(geometry, Hyperboloid::hyperboloidBoundsFunc, nullptr);
        rtcSetGeometryIntersectFunction(geometry, Hyperboloid::hyperboloidIntersectFunc);
        rtcSetGeometryOccludedFunction(geometry, Hyperboloid::hyperboloidOccludedFunc);

        // Commit the geometry and attach it to the scene.
        rtcCommitGeometry(geometry);
        para->geomID = rtcAttachGeometry(scene,geometry);
        hyperboloid.geomID = para->geomID;
        rtcReleaseGeometry(geometry);
    }
    {
        RTCGeometry geometry = rtcNewGeometry(device, RTC_GEOMETRY_TYPE_USER);
        auto *para = &sensor.planeParameters;

        rtcSetGeometryUserPrimitiveCount(geometry, 1);
        rtcSetGeometryUserData(geometry, para);
        para->geometry = geometry;

        rtcSetGeometryBoundsFunction(geometry, Plane::planeBoundsFunc, nullptr);
        rtcSetGeometryIntersectFunction(geometry, Plane::planeIntersectFunc);
        rtcSetGeometryOccludedFunction(geometry, Plane::planeOccludedFunc);

        // Commit the geometry and attach it to the scene.
        rtcCommitGeometry(geometry);
        para->geomID = rtcAttachGeometry(scene, geometry);
        rtcReleaseGeometry(geometry);
    }
    if (!spider.filename.empty())
        spider.geomID = addSTLMesh(spider.filename, spider.position, scene, device);
    rtcCommitScene(scene);
    return scene;
}

RTCDevice EmbreeScene::initializeDevice()
{
    RTCDevice device = rtcNewDevice(NULL);

    if (!device)
        printf("error %d: cannot create device\n", rtcGetDeviceError(NULL));

    rtcSetDeviceErrorFunction(device, errorFunction, NULL);
    return device;
}

void EmbreeScene::errorFunction([[maybe_unused]] void *userPtr, [[maybe_unused]] enum RTCError error, const char *str)
{
    printf("error %d: %s\n", error, str);
}




bool EmbreeScene::reflect_ray(Ray &ray) {
    double angle = get_angle(ray.normal(), ray.direction());
    // Return position but no direction because ray is now trapped
    if (angle - M_PI / 2 < 0) {
        return false;
    }

    ray.set_position(ray.position() + ray.rayhit.ray.tfar * ray.direction());
    ray.set_direction(reflect(ray.direction(), ray.normal()));
    ray.rayhit.ray.tnear = 0.0001;
    ray.rayhit.ray.tfar = std::numeric_limits<float>::infinity();
    ray.rayhit.ray.mask = -1;
    ray.rayhit.ray.flags = 0;
    ray.rayhit.hit.geomID = RTC_INVALID_GEOMETRY_ID;
    ray.rayhit.hit.primID = 0;
    ray.rayhit.hit.instID[0] = RTC_INVALID_GEOMETRY_ID;
    return true;
}

unsigned int EmbreeScene::addSTLMesh(const std::string& path, const Vec3fa& position, RTCScene& scene, RTCDevice& device) {
    stl_reader::StlMesh <float, unsigned int> mesh (path);
    RTCGeometry rtcMesh = rtcNewGeometry (device, RTC_GEOMETRY_TYPE_TRIANGLE);
    unsigned int geomID;
    unsigned* indices = (unsigned*) rtcSetNewGeometryBuffer(rtcMesh,
                                                            RTC_BUFFER_TYPE_INDEX,
                                                            0,
                                                            RTC_FORMAT_UINT3,
                                                            3*sizeof(unsigned),
                                                            mesh.num_tris());
    float* vertices = (float*) rtcSetNewGeometryBuffer(rtcMesh,
                                                       RTC_BUFFER_TYPE_VERTEX,
                                                       0,
                                                       RTC_FORMAT_FLOAT3,
                                                       3*sizeof(float),
                                                mesh.num_tris()*3);
    int counter_indices = 0;
    int counter_corner = 0;

    for(size_t itri = 0; itri < mesh.num_tris(); ++itri) {
        for(size_t icorner = 0; icorner < 3; ++icorner) {
            const float* c = mesh.tri_corner_coords (itri, icorner);
            if (vertices && indices)
            {
                vertices[counter_corner] = c[0] + position.x;
                counter_corner++;
                vertices[counter_corner] = c[1] + position.y;
                counter_corner++;
                vertices[counter_corner] = c[2] + position.z;
                counter_corner++;
            }
        }
        if (indices) {
            indices[counter_indices] = counter_indices;
            counter_indices++;
            indices[counter_indices] = counter_indices;
            counter_indices++;
            indices[counter_indices] = counter_indices;
            counter_indices++;
        }
    }
    rtcCommitGeometry(rtcMesh);
    geomID = rtcAttachGeometry(scene,rtcMesh);
    rtcReleaseGeometry(rtcMesh);
    return geomID;
}

std::shared_ptr<SurfaceModel> EmbreeScene::find_surface_model(unsigned int geomID) {
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










