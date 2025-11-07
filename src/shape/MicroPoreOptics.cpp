/*
Copyright (C) 2025  Neo Reinmann (neoreinmann@gmail.com)
*/

#include "MicroPoreOptics.h"

MicroPoreOptics::MicroPoreOptics(const Micro_pore_optics_parameters &micro_pore_optics_parameters) {
    MicroPoreOptics::filename = micro_pore_optics_parameters.file_name;
    MicroPoreOptics::position = micro_pore_optics_parameters.position;
}

MicroPoreOptics::MicroPoreOptics() {
    MicroPoreOptics::filename = "";
    MicroPoreOptics::position = Vec3fa{0.f, 0.f, 0.f};
}
