/*
Copyright (C) 2025  Neo Reinmann (neoreinmann@gmail.com)
*/
#include "LobsterEyeOptic.h"

LobsterEyeOptic::LobsterEyeOptic(const XMLData &xml_data) {
    device = EmbreeScene::initializeDevice();
    LobsterEyeOptic::create(xml_data);
}

std::optional<Ray> LobsterEyeOptic::ray_trace(Ray &ray) {
    if(embree_ray_trace(ray, 5)) {
        return ray;
    }
    return std::nullopt;
}

bool LobsterEyeOptic::embree_ray_trace(Ray &ray, int depth) {
    while (depth > 0) {
        // Intersect
        rtcIntersect1(scene, &ray.rayhit);
        Vec3fa normal = Vec3fa(ray.rayhit.hit.Ng_x, ray.rayhit.hit.Ng_y, ray.rayhit.hit.Ng_z);
        ray.set_normal(normalize(normal));


        if (ray.rayhit.hit.geomID == RTC_INVALID_GEOMETRY_ID)
            return false;

        ray.raytracing_history.emplace_back((short) ray.rayhit.hit.geomID,
                                            ray.position(),
                                            ray.direction());
        // Check if sensor was hit
        if (ray.rayhit.hit.geomID == sensor.planeParameters.geomID) {
            if (depth == 5) {
                return false;
            }
            ray.set_position(ray.position() + ray.rayhit.ray.tfar * ray.direction());
            return true;
        }

        if (ray.rayhit.hit.geomID == spider.geomID) {
            return false;
        } else if(ray.rayhit.hit.geomID == opticalMesh.geomID) {
            ray.set_position(ray.position() + ray.rayhit.ray.tfar * ray.direction());
            if(!pore.ray_trace(ray, depth)){
                return false;
            }
        }

        // Decrease depth
        depth--;
    }
    return false;
}

void LobsterEyeOptic::set_surface_parameter([[maybe_unused]] std::string model, [[maybe_unused]] std::string shadowing, [[maybe_unused]] double factor,
                                            [[maybe_unused]] double shadowing_factor) {

}

void LobsterEyeOptic::create(XMLData xml_data) {
    const auto raytracing = xml_data.child("telescope").child("raytracer");

    std::string spider_flag = raytracing.child("spider").attributeAsString("spider");
    Vec3fa spider_position = {};
    spider_position.x = (float) raytracing.child("spider").attributeAsDouble("position_x");
    spider_position.y = (float) raytracing.child("spider").attributeAsDouble("position_y");
    spider_position.z = (float) raytracing.child("spider").attributeAsDouble("position_z");
    std::string spider_path = raytracing.child("spider").attributeAsString("path");

    if (spider_flag == "true")
        spider = Spider(spider_path, spider_position);

    std::string surface_model = raytracing.child("surface").attributeAsString("model");
    std::string material_path = raytracing.child("surface").attributeAsString("material_path");
    std::string material = raytracing.child("surface").attributeAsString("material");

    Vec3fa optical_position = {};
    optical_position.x = (float) raytracing.child("optical").attributeAsDouble("position_x");
    optical_position.y = (float) raytracing.child("optical").attributeAsDouble("position_y");
    optical_position.z = (float) raytracing.child("optical").attributeAsDouble("position_z");
    std::string optical_path = raytracing.child("optical").attributeAsString("path");
    opticalMesh = OpticalMesh(optical_path, optical_position);

    double pore_width;
    double pore_length;
    pore_width = raytracing.child("type").attributeAsDouble("pore_width");
    pore_length = raytracing.child("type").attributeAsDouble("pore_length");
    pore = Pore(pore_width, pore_length, Vec3fa(0,0,0), Vec3fa(0,0,0), material_path, material);

    focal_length = raytracing.child("type").attributeAsDouble("focal_length");


    double sensor_x = raytracing.child("sensor").attributeAsDouble("sensor_x");
    double sensor_y = raytracing.child("sensor").attributeAsDouble("sensor_y");
    double sensor_z = raytracing.child("sensor").attributeAsDouble("sensor_z");
    Vec3fa sensor_position = {(float) sensor_x, (float) sensor_y, (float) sensor_z};
    std::string sensor_mesh = raytracing.child("sensor").attributeAsString("mesh");
    if (sensor_mesh == "true") {
        std::string sensor_path = raytracing.child("sensor").attributeAsString("path");

        mesh_sensor = Sensor(sensor_path, sensor_position);
    }
    double sensor_offset = raytracing.child("sensor").attributeAsDouble("offset");

    sensor = Plane(0,0,1, sensor_offset, sensor_x, sensor_y);
    scene = initializeScene(device);

}

RTCScene LobsterEyeOptic::initializeScene(RTCDevice device) {
    RTCScene scene = rtcNewScene(device);
    rtcSetSceneFlags(scene, RTC_SCENE_FLAG_ROBUST);
    rtcSetSceneBuildQuality(scene, RTC_BUILD_QUALITY_HIGH);

    if (!mesh_sensor.filename.empty()) {
        mesh_sensor.geomID = EmbreeScene::addSTLMesh(mesh_sensor.filename, mesh_sensor.position, scene, device);
        sensor.planeParameters.geomID = mesh_sensor.geomID;
    } else {
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
        spider.geomID = EmbreeScene::addSTLMesh(spider.filename, spider.position, scene, device);
    opticalMesh.geomID = EmbreeScene::addSTLMesh(opticalMesh.filename, opticalMesh.position, scene, device);
    rtcCommitScene(scene);
    return scene;
}

double LobsterEyeOptic::get_focal_length() {
    return focal_length;
}
