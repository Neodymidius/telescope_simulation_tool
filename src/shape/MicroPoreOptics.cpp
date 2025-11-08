/*
Copyright (C) 2025  Neo Reinmann (neoreinmann@gmail.com)
*/

#include "MicroPoreOptics.h"

MicroPoreOptics::MicroPoreOptics(const Micro_pore_optics_parameters &micro_pore_optics_parameters) {
    MicroPoreOptics::filename = micro_pore_optics_parameters.file_name;
    MicroPoreOptics::position = micro_pore_optics_parameters.position;
    pore = Pore(micro_pore_optics_parameters.pwidth, micro_pore_optics_parameters.plength,
                micro_pore_optics_parameters.protation,
                micro_pore_optics_parameters.ptranslation, micro_pore_optics_parameters.material_path, micro_pore_optics_parameters.material,
                micro_pore_optics_parameters.surface);
}

MicroPoreOptics::MicroPoreOptics() {
    MicroPoreOptics::filename = "";
    MicroPoreOptics::position = Vec3fa{0.f, 0.f, 0.f};
}
