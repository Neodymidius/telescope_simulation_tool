/*
Copyright (C) 2025  Neo Reinmann (neoreinmann@gmail.com)
*/

#ifndef SIXTE_OPTICALMESH_H
#define SIXTE_OPTICALMESH_H

#include "geometry/Vec3fa.h"
#include <embree4/rtcore.h>
#include <string>

#include "Pore.h"
#include "surface/SurfaceModel.h"

struct Micro_pore_optics_parameters {
    std::string file_name{};
    Vec3fa position = Vec3fa{0.f, 0.f, 0.f};
    double pwidth, plength;
    Vec3fa protation, ptranslation;
    std::string material_path;
    std::string material;
    std::shared_ptr<SurfaceModel> surface;
};

class MicroPoreOptics {
public:
    MicroPoreOptics();
    explicit MicroPoreOptics(const Micro_pore_optics_parameters &micro_pore_optics_parameters);
    std::string filename;
    Vec3fa position;
    unsigned int geomID = -1;

    Pore pore;

};


#endif //SIXTE_OPTICALMESH_H
