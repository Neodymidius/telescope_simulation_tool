//
// Created by neo on 28.05.25.
//

#ifndef SIXTE_DUMMY_H
#define SIXTE_DUMMY_H
#include "geometry/Vec3fa.h"
#include "SurfaceStrategy.h"

class Dummy final : public SurfaceStrategy {
public:
    explicit Dummy() {};
    ~Dummy() override = default;
    bool simulate_surface(Ray &ray) const override;
    void set_surface_parameter(std::string model, std::string shadowing, double factor, double shadowing_factor) override;
};


#endif //SIXTE_DUMMY_H
