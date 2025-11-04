//
// Created by neo on 28.05.25.
//

#include "Dummy.h"

bool Dummy::simulate_surface([[maybe_unused]] Ray &ray) const {
    return true;
}

void Dummy::set_surface_parameter([[maybe_unused]] std::string model, [[maybe_unused]] std::string shadowing, [[maybe_unused]] double factor, [[maybe_unused]] double shadowing_factor) {

}
