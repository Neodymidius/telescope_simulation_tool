//
// Created by neo on 25.08.25.
//

#include "OpticalMesh.h"

OpticalMesh::OpticalMesh(std::string file_name, const Vec3fa &position) {
    OpticalMesh::filename = file_name;
    OpticalMesh::position = position;
}

OpticalMesh::OpticalMesh() {
    OpticalMesh::filename = "";
    OpticalMesh::position = Vec3fa{0.f, 0.f, 0.f};
}
