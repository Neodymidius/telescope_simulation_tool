/*
Copyright (C) 2025  Neo Reinmann (neoreinmann@gmail.com)
*/

#include <iostream>
#include <iomanip>
#include "Hyperboloid.h"

Hyperboloid::Hyperboloid(Hyperboloid_parameters hyperboloid_parameters) : a(hyperboloid_parameters.a),
    b(hyperboloid_parameters.b), c(hyperboloid_parameters.c), Xh_max(hyperboloid_parameters.Xh_max),
    Xh_min(hyperboloid_parameters.Xh_min), Yh_max(hyperboloid_parameters.Yh_max), Yh_min(hyperboloid_parameters.Yh_min),
    theta(hyperboloid_parameters.theta), surface(hyperboloid_parameters.surface), hyperboloid_parameters(hyperboloid_parameters),
    geomID(hyperboloid_parameters.geomID)
{}

void Hyperboloid::hyperboloidBoundsFunc(const RTCBoundsFunctionArguments *args) {
    const auto* para = (const Hyperboloid_parameters*) args->geometryUserPtr;
    RTCBounds* b = args->bounds_o;

    const float pad = 0.5f; // mm safety

    // axial (Z)
    b->lower_z = (float)para->Xh_min - pad;
    b->upper_z = (float)para->Xh_max + pad;

    // radial circle (X,Y), symmetric padding
    float rmin = (float)para->Yh_min; // these are radii in your code
    float rmax = (float)para->Yh_max;
    float R    = std::max(rmin, rmax);

    b->lower_x = -R - pad;
    b->upper_x =  R + pad;
    b->lower_y = -R - pad;
    b->upper_y =  R + pad;
}

void Hyperboloid::hyperboloidIntersectFunc(const RTCIntersectFunctionNArguments* args) {
    auto* rh = (RTCRayHit*)args->rayhit;
    RTCRay& ray = rh->ray;
    const auto* para = (const Hyperboloid_parameters*)args->geometryUserPtr;

    // build basis (same as paraboloid)
    auto Rm = get_rotation_matrix(para->angle_x, para->angle_y);
    Vec3fa z_tilted = Rm * Vec3fa{0,0,1};
    Vec3fa x_tilted = normalize(cross(z_tilted, Vec3fa{0,1,0}));
    Vec3fa y_tilted = cross(z_tilted, x_tilted);
    std::array<Vec3fa,3> basis = {x_tilted, y_tilted, z_tilted};

    // world ray -> local ray
    Vec3fa pW{ray.org_x, ray.org_y, ray.org_z};
    Vec3fa vW{ray.dir_x, ray.dir_y, ray.dir_z};
    Vec3fa pL = to_local(basis, para->origin, pW);
    Vec3fa vL = dir_to_local(basis, vW);

    // doubles for robustness
    const double px = pL.x, py = pL.y, pz = pL.z;
    const double dx = vL.x, dy = vL.y, dz = vL.z;

    const double a  = para->a;
    const double b  = para->b;
    const double c  = para->c;
    const double a2 = a*a;
    const double b2 = b*b;

    // x^2/b^2 + y^2/b^2 - (z-c)^2/a^2 = -1  -> A t^2 + B t + C = 0
    const double A = (dx*dx + dy*dy)/b2 - (dz*dz)/a2;
    const double B = 2.0 * ((px*dx + py*dy)/b2 - ((pz - c)*dz)/a2);
    const double C = (px*px + py*py)/b2 - ((pz - c)*(pz - c))/a2 + 1.0;

    auto accept = [&](double t)->bool {
        if (!(t > (double)ray.tnear && t < (double)ray.tfar)) return false;
        double zh = pz + dz*t; // local z!
        return (zh >= para->Xh_min && zh <= para->Xh_max);
    };

    const double eps = 1e-18;
    double tHit = std::numeric_limits<double>::infinity();

    if (std::abs(A) < eps) {
        if (std::abs(B) < eps) return; // no solution
        double t = -C / B;
        if (!accept(t)) return;
        tHit = t;
    } else {
        double disc = B*B - 4.0*A*C;
        if (disc < 0.0) return;
        double sd = std::sqrt(disc);
        double t0 = (-B - sd) / (2.0*A);
        double t1 = (-B + sd) / (2.0*A);
        if (t0 > t1) std::swap(t0, t1);

        if      (accept(t0)) tHit = t0;
        else if (accept(t1)) tHit = t1;
        else return;
    }

    // commit hit (t is the same in world)
    ray.tfar       = (float)tHit;
    rh->hit.geomID = para->geomID;
    rh->hit.primID = para->geomID;

    // local hit point (for normal)
    const float hx = (float)(px + dx*tHit);
    const float hy = (float)(py + dy*tHit);
    const float hz = (float)(pz + dz*tHit);

    // inward geometric normal in LOCAL space:
    // n ∝ ( x/b^2, y/b^2, -(z-c)/a^2 )
    double nx =  hx / b2;
    double ny =  hy / b2;
    double nz = -(hz - (float)c) / a2;
    double nlen = std::sqrt(nx*nx + ny*ny + nz*nz);
    if (nlen > 0.0) { nx /= nlen; ny /= nlen; nz /= nlen; }

    // transform normal to world and keep your sign convention
    Vec3fa Nw = normal_to_world(basis, Vec3fa{(float)nx,(float)ny,(float)nz});
    rh->hit.Ng_x = -Nw.x;
    rh->hit.Ng_y = -Nw.y;
    rh->hit.Ng_z = -Nw.z;
}

void Hyperboloid::hyperboloidOccludedFunc([[maybe_unused]] const RTCOccludedFunctionNArguments *args) {

}
