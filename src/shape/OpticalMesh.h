/*
Copyright (C) 2025  Neo Reinmann (neoreinmann@gmail.com)
*/

#ifndef SIXTE_OPTICALMESH_H
#define SIXTE_OPTICALMESH_H

#include "geometry/Vec3fa.h"
#include <embree4/rtcore.h>
#include <string>
#include "surface/SurfaceModel.h"


class OpticalMesh {
public:
    OpticalMesh();
    explicit OpticalMesh(std::string file_name, const Vec3fa& position);
    std::string filename;
    Vec3fa position;
    unsigned int geomID = -1;
};


#endif //SIXTE_OPTICALMESH_H
