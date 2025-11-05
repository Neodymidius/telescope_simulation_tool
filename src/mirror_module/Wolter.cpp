/*
Copyright (C) 2025  Neo Reinmann (neoreinmann@gmail.com)
*/

#include "Wolter.h"
#include <string>

Wolter::Wolter(const XMLData& xml_data) {
    shapes = EmbreeScene();
    Wolter::create(xml_data);
}

std::optional<Ray> Wolter::ray_trace(Ray &ray) {
    return shapes.ray_trace(ray);
}


void Wolter::create_parameters(const double new_radius, Paraboloid_parameters &p_pars, Hyperboloid_parameters &h_pars) const {
    const double local_theta = asin(new_radius/focal_length)/4;
    p_pars.Xp_min = focal_length * cos(4 * local_theta) + 2 * h_pars.c;
    p_pars.Yp_min = new_radius;
    p_pars.p = p_pars.Yp_min * tan(local_theta);
    p_pars.Yp_max = sqrt(p_pars.p *(2*p_pars.Xp_max + p_pars.p));
    h_pars.a = focal_length * (2 * cos(2*local_theta) - 1) / 2;
    h_pars.b = sqrt(pow(h_pars.c, 2) - pow(h_pars.a, 2));
}

std::vector<std::string> split(std::string s, const std::string& delimiter) {
    std::vector<std::string> tokens;
    size_t pos = 0;
    std::string token;
    while ((pos = s.find(delimiter)) != std::string::npos) {
        token = s.substr(0, pos);
        tokens.push_back(token);
        s.erase(0, pos + delimiter.length());
    }
    tokens.push_back(s);

    return tokens;
}


void Wolter::create(XMLData xml_data) {
    Paraboloid_parameters p_pars{};
    Hyperboloid_parameters h_pars{};

    const auto raytracing = xml_data.child("telescope").child("raytracer");
    std::string telescope_type = raytracing.child("type").attributeAsString("type");
    focal_length = raytracing.child("type").attributeAsDouble("focal_length");
    outer_radius = raytracing.child("type").attributeAsDouble("outer_diameter") / 2;
    inner_radius = raytracing.child("type").attributeAsDouble("inner_diameter") / 2;
    number_of_shells = raytracing.child("type").attributeAsInt("mirror_shells");
    mirror_height = raytracing.child("type").attributeAsDouble("mirror_height");
    sensor_offset = raytracing.child("sensor").attributeAsDouble("offset");
    double sensor_x = raytracing.child("sensor").attributeAsDouble("sensor_x");
    double sensor_y = raytracing.child("sensor").attributeAsDouble("sensor_y");

    std::string spider_flag = raytracing.child("spider").attributeAsString("spider");
    Vec3fa spider_position = {};
    spider_position.x = (float) raytracing.child("spider").attributeAsDouble("position_x");
    spider_position.y = (float) raytracing.child("spider").attributeAsDouble("position_y");
    spider_position.z = (float) raytracing.child("spider").attributeAsDouble("position_z");
    std::string spider_path = raytracing.child("spider").attributeAsString("path");


    if (spider_flag == "true")
        shapes.spider = Spider(spider_path, spider_position);

    std::string surface_model = raytracing.child("surface").attributeAsString("model");
    std::string material_path = raytracing.child("surface").attributeAsString("material_path");
    std::string material = raytracing.child("surface").attributeAsString("material");


    const auto mirror = raytracing.child("mirror");
    std::string mirror_flag = mirror.attributeAsString("exact");

    bool first_shell = true;
    double first_shell_height = 0;
    if (mirror_flag == "true") {
        auto shell_positions = split(mirror.attributeAsString("positions"), ",");
        for (const auto& shell : shell_positions) {
            auto Ypmin = std::stod(shell);
            p_pars.theta = asin(Ypmin / focal_length) / 4;
            h_pars.c = focal_length / 2;
            p_pars.Xp_min = focal_length * cos(4 * p_pars.theta) + 2 * h_pars.c;
            p_pars.Xp_max = mirror_height + p_pars.Xp_min;
            p_pars.Yp_min = Ypmin;
            p_pars.p = p_pars.Yp_min * tan(p_pars.theta);
            p_pars.Yp_max = sqrt(p_pars.p * (2 * p_pars.Xp_max + p_pars.p));

            h_pars.theta = p_pars.theta;
            h_pars.a = focal_length * (2 * cos(2*h_pars.theta) - 1) / 2;
            h_pars.b = sqrt(pow(h_pars.c, 2) - pow(h_pars.a, 2));
            h_pars.Xh_max = p_pars.Xp_min;
            h_pars.Xh_min = p_pars.Xp_min - mirror_height;
            h_pars.Yh_max = p_pars.Yp_min;
            h_pars.Yh_min = h_pars.b * sqrt(pow(h_pars.Xh_min - h_pars.c, 2) / pow(h_pars.a, 2) - 1);

            if (first_shell) {
                /*p_pars.angle_x = 0.03 * M_PI / 180;
                p_pars.angle_y = 0.03 * M_PI / 180;;
                */
                p_pars.origin = Vec3fa(0,0, 0);
                first_shell_height = p_pars.Xp_max;
                first_shell=false;
            } else {
                auto z_offset = (float) (first_shell_height - p_pars.Xp_max);
                p_pars.origin = Vec3fa(0, 0, z_offset);
                h_pars.origin = Vec3fa(0, 0, z_offset);
            }

            if (surface_model == "gauss") {
                double factor = raytracing.child("surface").attributeAsDouble("roughness");
                p_pars.surface = std::make_unique<SurfaceModel>(std::make_unique<GaussSurface>(factor));
                h_pars.surface = std::make_unique<SurfaceModel>(std::make_unique<GaussSurface>(factor));


            } else if (surface_model == "microfacet") {
                double factor = raytracing.child("surface").attributeAsDouble("roughness");
                double factor_shadowing = raytracing.child("surface").attributeAsDouble("shadowing_alpha");
                std::string mf_type = raytracing.child("surface").attributeAsString("type");
                std::string mf_shadowing = raytracing.child("surface").attributeAsString("shadowing");
                bool ggx = false;
                bool ggx_shadowing = false;
                if (mf_type == "ggx")
                    ggx = true;
                if (mf_shadowing == "ggx")
                    ggx_shadowing = true;

                p_pars.surface = std::make_unique<SurfaceModel>(std::make_unique<Microfacet>(factor, factor_shadowing, ggx, ggx_shadowing));;
                h_pars.surface = std::make_unique<SurfaceModel>(std::make_unique<Microfacet>(factor, factor_shadowing, ggx, ggx_shadowing));
            } else {
                p_pars.surface = std::make_unique<SurfaceModel>(std::make_unique<Dummy>());
                h_pars.surface = std::make_unique<SurfaceModel>(std::make_unique<Dummy>());
            }

            shapes.hyperboloids.emplace_back(h_pars);
            shapes.paraboloids.emplace_back(p_pars);

        }

    } else {
        h_pars.c = focal_length / 2;
        p_pars.theta = asin(outer_radius / focal_length) / 4 * 180 / M_PI;
        h_pars.theta = p_pars.theta;

        // Most outer paraboloid
        p_pars.Xp_min = focal_length * cos(4 * p_pars.theta) + 2 * h_pars.c;
        p_pars.Xp_max = mirror_height + p_pars.Xp_min;
        p_pars.Yp_min = outer_radius;
        p_pars.p = p_pars.Yp_min * tan(p_pars.theta);
        p_pars.Yp_max = sqrt(p_pars.p * (2 * p_pars.Xp_max + p_pars.p));

        distance_to_mirror = (outer_radius - inner_radius) / (number_of_shells - 1);

        for (int i = 0; i < number_of_shells; i++) {

            create_parameters(outer_radius - distance_to_mirror * i, p_pars, h_pars);
            // p_pars.Xp_min = focal_length * cos(4 * p_pars.theta) + 2 * h_pars.c;
            p_pars.Xp_max = mirror_height + p_pars.Xp_min;
            h_pars.Xh_max = p_pars.Xp_min;
            h_pars.Xh_min = p_pars.Xp_min - mirror_height;
            //    p_pars.id = shape_id{'p', (short) i};
            //    h_pars.id = shape_id{'h', (short) i};
            h_pars.Yh_max = p_pars.Yp_min;
            h_pars.Yh_min = h_pars.b * sqrt(pow(h_pars.Xh_min - h_pars.c, 2) / pow(h_pars.a, 2) - 1);

            if (surface_model == "gauss") {
                double factor = raytracing.child("surface").attributeAsDouble("roughness");
                p_pars.surface = std::make_unique<SurfaceModel>(std::make_unique<GaussSurface>(factor));
                h_pars.surface = std::make_unique<SurfaceModel>(std::make_unique<GaussSurface>(factor));


            } else if (surface_model == "microfacet") {
                double factor = raytracing.child("surface").attributeAsDouble("roughness");
                double factor_shadowing = raytracing.child("surface").attributeAsDouble("shadowing_alpha");
                std::string mf_type = raytracing.child("surface").attributeAsString("type");
                std::string mf_shadowing = raytracing.child("surface").attributeAsString("shadowing");
                bool ggx = false;
                bool ggx_shadowing = false;
                if (mf_type == "ggx")
                    ggx = true;
                if (mf_shadowing == "ggx")
                    ggx_shadowing = true;

                p_pars.surface = std::make_unique<SurfaceModel>(std::make_unique<Microfacet>(factor, factor_shadowing, ggx, ggx_shadowing));;
                h_pars.surface = std::make_unique<SurfaceModel>(std::make_unique<Microfacet>(factor, factor_shadowing, ggx, ggx_shadowing));
            } else {
                p_pars.surface = std::make_unique<SurfaceModel>(std::make_unique<Dummy>());
                h_pars.surface = std::make_unique<SurfaceModel>(std::make_unique<Dummy>());
            }

            shapes.hyperboloids.emplace_back(h_pars);
            shapes.paraboloids.emplace_back(p_pars);

        }
    }
    shapes.sensor = Plane{0, 0, 1, -h_pars.c * 2 + sensor_offset, sensor_x, sensor_y};
    shapes.scene = shapes.initializeScene(shapes.device);

}

void Wolter::set_surface_parameter(std::string model, std::string shadowing, double factor, double shadowing_factor) {
    for (auto &paraboloid : shapes.paraboloids) {
        paraboloid.surface->set_surface_parameter(model, shadowing, factor, shadowing_factor);
    }
    for (auto &hyperboloid : shapes.hyperboloids) {
        hyperboloid.surface->set_surface_parameter(model, shadowing, factor, shadowing_factor);
    }

}

double Wolter::get_focal_length() {
    return focal_length;
}

