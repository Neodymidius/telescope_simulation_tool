//
// Created by neo on 21.02.24.
//

#include <iostream>
#include <iomanip>
#include "Paraboloid.h"


Paraboloid::Paraboloid(Paraboloid_parameters paraboloid_parameters)
    : theta(paraboloid_parameters.theta), p(paraboloid_parameters.p), Yp_min(paraboloid_parameters.Yp_min),
      Xp_min(paraboloid_parameters.Xp_min), Xp_max(paraboloid_parameters.Xp_max), Yp_max(paraboloid_parameters.Yp_max), surface(paraboloid_parameters.surface), paraboloid_parameters(paraboloid_parameters)
{}

void Paraboloid::paraboloidBoundsFunc(const RTCBoundsFunctionArguments *args) {
    const auto* para = (const Paraboloid_parameters*) args->geometryUserPtr;
    RTCBounds* b = args->bounds_o;

    const float pad = 0.5f; // mm safety

    // axial (Z) from paper’s Xp
    b->lower_z = (float)para->Xp_min - pad;
    b->upper_z = (float)para->Xp_max + pad;

    // radial circle (X,Y) using the true max radius at Zmax
    float rmax = (float)para->Yp_max;

    b->lower_x = -rmax - pad;
    b->upper_x =  rmax + pad;
    b->lower_y = -rmax - pad;
    b->upper_y =  rmax + pad;
}

void Paraboloid::paraboloidIntersectFunc(const RTCIntersectFunctionNArguments *args) {
    auto* rayhit = (RTCRayHit*) args->rayhit;
    RTCRay& ray = rayhit->ray;
    const auto* para  = (const Paraboloid_parameters*) args->geometryUserPtr;

    // Build rotated frame
    auto R = get_rotation_matrix(para->angle_x, para->angle_y); // columns
    Vec3fa z_tilted = R * Vec3fa{0,0,1};
    Vec3fa x_tilted = normalize(cross(z_tilted, Vec3fa{0,1,0}));
    Vec3fa y_tilted = cross(z_tilted, x_tilted);
    std::array<Vec3fa,3> basis = {x_tilted, y_tilted, z_tilted};

    // World-space ray
    Vec3fa pos{ray.org_x, ray.org_y, ray.org_z};
    Vec3fa dir{ray.dir_x, ray.dir_y, ray.dir_z};

    // subtract translation before rotating
    Vec3fa pos_local = to_local(basis, para->origin, pos);
    Vec3fa dir_local = dir_to_local(basis, dir);

    // Ray components in LOCAL coordinates
    float p_x = pos_local.x, p_y = pos_local.y, p_z = pos_local.z;
    float v_x = dir_local.x, v_y = dir_local.y, v_z = dir_local.z;

    // Paraboloid intersection in local frame
    double A = double(v_x)*v_x + double(v_y)*v_y;
    double B = 2.0 * (double(p_x)*v_x + double(p_y)*v_y - double(para->p)*v_z);
    double C = double(p_x)*p_x + double(p_y)*p_y - double(para->p)*para->p - 2.0*double(para->p)*p_z;

    const double eps = 1e-12;
    double t = std::numeric_limits<double>::infinity();

    if (std::abs(A) < eps) { // locally parallel to local z
        if (std::abs(v_z) >= eps) {
            t = (-double(para->p)*(double(para->p) + 2.0*double(p_z)) + double(p_x)*p_x + double(p_y)*p_y)
                / (2.0*double(para->p)*double(v_z));
        }
    } else {
        double D = B*B - 4.0*A*C;
        if (D >= 0.0) {
            double sD = std::sqrt(D);
            double t1 = (-B + sD) / (2.0*A);
            double t2 = (-B - sD) / (2.0*A);
            t = std::min(t1, t2);
            if (t < 0.0) t = std::max(t1, t2);
        }
    }

    if (!std::isfinite(t)) return;

    // z check in LOCAL space
    float zhit = float(p_z + t * v_z);
    if (zhit < para->Xp_min || zhit > para->Xp_max) return;

    // t-range check in WORLD space (same t)
    if (t < ray.tnear || t > ray.tfar) return;

    // Commit the hit
    ray.tfar = float(t);
    rayhit->hit.primID = para->geomID;
    rayhit->hit.geomID = para->geomID;

    // Local hit point and normal
    float hx = float(p_x + t * v_x);
    float hy = float(p_y + t * v_y);

    float nx =  hx / float(para->p);
    float ny =  hy / float(para->p);
    float nz = -1.0f;
    float invLen = 1.0f / std::sqrt(nx*nx + ny*ny + nz*nz);
    nx *= invLen; ny *= invLen; nz *= invLen;

    // Back to WORLD for the geometric normal
    Vec3fa Nw = normal_to_world(basis, Vec3fa{nx, ny, nz});
    // Keep your original flip if needed
    rayhit->hit.Ng_x = -Nw.x;
    rayhit->hit.Ng_y = -Nw.y;
    rayhit->hit.Ng_z = -Nw.z;
}


void Paraboloid::paraboloidOccludedFunc(const RTCOccludedFunctionNArguments *args) { }


