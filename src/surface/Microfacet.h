/*
Copyright (C) 2025  Neo Reinmann (neoreinmann@gmail.com)
*/
/**
 * From Paper: Microfacet Models for Refraction through Rough Surfaces by Walter et al. (2007)
**/
#ifndef SIXTE_MICROFACET_H
#define SIXTE_MICROFACET_H


#include "SurfaceStrategy.h"
#include "lib/random.h"
#include "cmath"

class Microfacet final : public SurfaceStrategy {
public:
    explicit Microfacet() {};
    explicit Microfacet(double palpha, double palpha_shadowing, bool pggx, bool pggx_shadowing);
    ~Microfacet() override = default;
    bool simulate_surface(Ray & ray) const override;
    void set_surface_parameter(std::string model, std::string shadowing, double factor, double shadowing_factor) override;
private:

    Vec3fa get_beckmann_m() const;

    [[nodiscard]] double beckmann_shadowing_term(Vec3fa v, Vec3fa m) const ;

    [[nodiscard]] Vec3fa get_gxx_m() const;

    [[nodiscard]] double ggx_shadowing_term(const Vec3fa& v, const Vec3fa& m) const;

    static double positive_characteristic_function(double a) ;

    double alpha{};
    double alpha_shadowing{};
    bool ggx{};
    bool ggx_shadowing{};

};


#endif //SIXTE_MICROFACET_H
