/*
Copyright (C) 2025  Neo Reinmann (neoreinmann@gmail.com)
*/

#include <cmath>
#include <iostream>
#include "Microfacet.h"

Microfacet::Microfacet(double palpha, double palpha_shadowing, bool pggx, bool pggx_shadowing) :
alpha(palpha), alpha_shadowing(palpha_shadowing), ggx(pggx), ggx_shadowing(pggx_shadowing) {}

bool Microfacet::simulate_surface(Ray & ray) const {
    const Vec3fa z(0.0f, 0.0f, 1.0f);
    const Vec3fa n = normalize(ray.normal());
    float c = dot(n, z);                   // cos(theta)
    c = std::max(-1.0f, std::min(1.0f, c));
    Vec3fa axis = cross(n, z);
    float s = sqrt(dot(axis, axis));
    axis = axis / s;                             // normalize
    float theta = std::atan2(s, c);        // robust angle
    Vec3fa transformed_incoming = apply_rodrigues_rotation(axis, ray.direction(), theta);

    Vec3fa m;
    Vec3fa outcoming;
    double prob_shadowing, prob_masking;
    if (ggx) {
        m = get_gxx_m();
        prob_masking = ggx_shadowing_term(transformed_incoming, m);
        outcoming = reflect(transformed_incoming, m);
    } else {
        m = get_beckmann_m();
        prob_masking = beckmann_shadowing_term(transformed_incoming, m);
        outcoming = reflect(transformed_incoming, m);
    }
    if (ggx_shadowing) {
        prob_shadowing = ggx_shadowing_term(outcoming, m);
    } else {
        prob_shadowing = beckmann_shadowing_term(outcoming, m);
    }
    if (easy_uniform_random() > prob_shadowing * prob_masking) {
        return false;
    }
    m = apply_rodrigues_rotation(axis, m, -theta);

    ray.set_normal(m);
    return true;
}

Vec3fa Microfacet::get_gxx_m() const {
    double xi_1 = easy_uniform_random();
    double xi_2 = easy_uniform_random();
    double theta_m = atan((alpha * sqrt(xi_1))/ sqrt(1 - xi_1));
    double phi_m = 2*M_PI*xi_2;
    return {(float) (sin(theta_m) * cos(phi_m)), (float) (sin(theta_m) * sin(phi_m)), (float) cos(theta_m)};
}

double Microfacet::ggx_shadowing_term(const Vec3fa& v, const Vec3fa& m) const {
    return positive_characteristic_function(dot(v, m)/ dot(v, {0,0,1})) *  2 /
                                                (1 + sqrt(1+ pow(alpha_shadowing, 2) * pow(std::tan(get_angle(v, m)), 2)));
}

double Microfacet::positive_characteristic_function(double a) {
    return a > 0 ? 1 : 0 ;
}

Vec3fa Microfacet::get_beckmann_m() const {
    double xi_1 = easy_uniform_random();
    double xi_2 = easy_uniform_random();
    double theta_m = atan(sqrt(-pow(alpha, 2) * log(1 - xi_1)));
    double phi_m = 2*M_PI*xi_2;
    return {(float) (sin(theta_m) * cos(phi_m)), (float) (sin(theta_m) * sin(phi_m)), (float) cos(theta_m)};
}

double Microfacet::beckmann_shadowing_term(Vec3fa v, Vec3fa m) const {
    double aa = dot(v, m)/ dot(v, {0,0,1});
    double a = pow(alpha_shadowing * std::tan(get_angle(v, m)), -1);
    return positive_characteristic_function(aa) < 1.6 ? (3.535 * a + 2.181 * pow(a,2))/(1 + 2.276 * a + 2.577 * pow(a, 2)) : 1;
}

void Microfacet::set_surface_parameter(std::string model, std::string shadowing, double factor,
                                       double shadowing_factor) {
    if (model == "ggx")
        ggx = true;
    else
        ggx = false;

    if (shadowing == "ggx")
        ggx_shadowing = true;
    else
        ggx_shadowing = false;

    alpha = factor;
    alpha_shadowing = shadowing_factor;
}
