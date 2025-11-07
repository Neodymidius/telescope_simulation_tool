/*
Copyright (C) 2025  Neo Reinmann (neoreinmann@gmail.com)
*/
#include "LobsterEyeOptics.h"

LobsterEyeOptics::LobsterEyeOptics(const XMLData &xml_data) {
    LobsterEyeOptics::create(xml_data);
}

std::optional<Ray> LobsterEyeOptics::ray_trace(Ray &ray) {
    return embree_scene_.ray_trace(ray);
}

void LobsterEyeOptics::set_surface_parameter([[maybe_unused]] std::string model, [[maybe_unused]] std::string shadowing, [[maybe_unused]] double factor,
                                            [[maybe_unused]] double shadowing_factor) {

}

void LobsterEyeOptics::create(XMLData xml_data) {
    const auto raytracing = xml_data.child("telescope").child("raytracer");

    std::string spider_flag = raytracing.child("spider").attributeAsString("spider");
    Vec3fa spider_position = {};
    spider_position.x = (float) raytracing.child("spider").attributeAsDouble("position_x");
    spider_position.y = (float) raytracing.child("spider").attributeAsDouble("position_y");
    spider_position.z = (float) raytracing.child("spider").attributeAsDouble("position_z");
    std::string spider_path = raytracing.child("spider").attributeAsString("path");

    if (spider_flag == "true")
        embree_scene_.spider = Spider(spider_path, spider_position);

    Micro_pore_optics_parameters micro_pore_optics_parameters{};

    micro_pore_optics_parameters.position.x = (float) raytracing.child("optical").attributeAsDouble("position_x");
    micro_pore_optics_parameters.position.y = (float) raytracing.child("optical").attributeAsDouble("position_y");
    micro_pore_optics_parameters.position.z = (float) raytracing.child("optical").attributeAsDouble("position_z");
    micro_pore_optics_parameters.file_name = raytracing.child("optical").attributeAsString("path");

    micro_pore_optics_parameters.pwidth = raytracing.child("type").attributeAsDouble("pore_width");
    micro_pore_optics_parameters.plength = raytracing.child("type").attributeAsDouble("pore_length");

    std::string surface_model = raytracing.child("surface").attributeAsString("model");
    micro_pore_optics_parameters.surface = SurfaceModel::get_surface_model(surface_model);
    if (surface_model == "gauss") {
        double factor = raytracing.child("surface").attributeAsDouble("roughness");
        micro_pore_optics_parameters.surface->set_surface_parameter("gauss", "", factor, factor);

    } else if (surface_model == "microfacet") {
        double factor = raytracing.child("surface").attributeAsDouble("roughness");
        double factor_shadowing = raytracing.child("surface").attributeAsDouble("shadowing_alpha");
        std::string mf_type = raytracing.child("surface").attributeAsString("type");
        std::string mf_shadowing = raytracing.child("surface").attributeAsString("shadowing");
        micro_pore_optics_parameters.surface->set_surface_parameter(mf_type, mf_shadowing, factor, factor_shadowing);
    }
    micro_pore_optics_parameters.material_path = raytracing.child("surface").attributeAsString("material_path");
    micro_pore_optics_parameters.material = raytracing.child("surface").attributeAsString("material");
    embree_scene_.micro_pore_optics = MicroPoreOptics(micro_pore_optics_parameters);

    focal_length = raytracing.child("type").attributeAsDouble("focal_length");


    double sensor_x = raytracing.child("sensor").attributeAsDouble("sensor_x");
    double sensor_y = raytracing.child("sensor").attributeAsDouble("sensor_y");
    double sensor_z = raytracing.child("sensor").attributeAsDouble("sensor_z");
    Vec3fa sensor_position = {(float) sensor_x, (float) sensor_y, (float) sensor_z};
    std::string sensor_mesh = raytracing.child("sensor").attributeAsString("mesh");
    if (sensor_mesh == "true") {
        std::string sensor_path = raytracing.child("sensor").attributeAsString("path");

        embree_scene_.sensor = Sensor(sensor_path, sensor_position);
    } else {
        double sensor_offset = raytracing.child("sensor").attributeAsDouble("offset");

        embree_scene_.sensor = Sensor(Plane_parameters{0,0,1, sensor_offset, sensor_x, sensor_y, {}, 0});
    }
    embree_scene_.initializeScene();

}

double LobsterEyeOptics::get_focal_length() {
    return focal_length;
}
