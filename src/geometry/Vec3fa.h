//
// Created by neo on 4/19/25.
//

#ifndef VEC3FA_H
#define VEC3FA_H
#include <cmath>
#include <array>
#include <ostream>



struct Vec3fa {
    float x, y, z;
    Vec3fa() : x(0), y(0), z(0) {}
    explicit Vec3fa(const float a) : x(a), y(a), z(a) {}
    Vec3fa(const float x, const float y, const float z) : x(x), y(y), z(z) {}

    // Non-const version
    float& operator[](int idx) {
        switch (idx) {
            case 0: return x;
            case 1: return y;
            case 2: return z;
            default: throw std::out_of_range("Vec3fa index out of range");
        }
    }

    // Const version
    const float& operator[](int idx) const {
        switch (idx) {
            case 0: return x;
            case 1: return y;
            case 2: return z;
            default: throw std::out_of_range("Vec3fa index out of range");
        }
    }
};

inline Vec3fa operator+(const Vec3fa& a, const Vec3fa& b) {
    return {a.x + b.x, a.y + b.y, a.z + b.z};
}
inline Vec3fa operator-(const Vec3fa& a, const Vec3fa& b) {
    return {a.x - b.x, a.y - b.y, a.z - b.z};
}
inline Vec3fa operator*(const float s, const Vec3fa& a) {
    return {a.x * s, a.y * s, a.z * s};
}

inline Vec3fa operator*(const Vec3fa& a, const float s) {
    return {a.x * s, a.y * s, a.z * s};
}

inline Vec3fa operator/(const Vec3fa& a, const float s) {
    return {a.x / s, a.y / s, a.z / s};
}

inline float dot(const Vec3fa& a, const Vec3fa& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline Vec3fa cross(const Vec3fa& a, const Vec3fa& b)
{
    return { a.y * b.z - a.z * b.y,
             a.z * b.x - a.x * b.z,
             a.x * b.y - a.y * b.x };
}

inline Vec3fa normalize(const Vec3fa& a) {
    const float len = std::sqrt(dot(a, a));
    return a * (1.0f / len);
}

inline float get_angle(const Vec3fa& a, const Vec3fa& b)
{
    const float n = dot(a, b);
    const float normal_eucl = sqrt(dot(a,a));
    const float direction_eucl = sqrt(dot(b, b));
    const float angle = acos(n/(normal_eucl*direction_eucl));
    return angle;
}

inline Vec3fa reflect(const Vec3fa& v, const Vec3fa& n)
{
    return v - 2.0f * dot(v, n) * n;
}

inline Vec3fa get_translation(const Vec3fa& target, const Vec3fa& v)
{
    return {0 - v.x, 0 - v.y, 1 - v.z};
}

inline Vec3fa apply_rodrigues_rotation(const Vec3fa& k, const Vec3fa& v, float theta)
{
    return v * cos(theta) + cross(k, v) * sin(theta) + k * (dot(k, v)) * (1 - cos(theta));
}

inline std::ostream& operator<<(std::ostream& os, const Vec3fa v) {
    os << v.x << " " << v.y << " " << v.z;
    return os;
}

inline std::ostream& operator<<(std::ostream& os, const std::array<Vec3fa, 3> &a) {
    os << a[0][0] << " " << a[1][0] << " " << a[2][0] << std::endl;
    os << a[0][1] << " " << a[1][1] << " " << a[2][1] << std::endl;
    os << a[0][2] << " " << a[1][2] << " " << a[2][2] << std::endl;
    return os;
}

inline std::array<Vec3fa, 3> operator*(const std::array<Vec3fa, 3> &a, const std::array<Vec3fa, 3> &b)
{
    std::array<Vec3fa, 3> c = {Vec3fa{0}, Vec3fa{0}, Vec3fa{0}};
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            c[i][j] = 0;
            for (int k = 0; k < 3; k++) {
                c[i][j]+= a[i][k] * b[k][j];
            }
        }
    }
    return c;
}

inline Vec3fa operator*(const std::array<Vec3fa, 3> &a,const Vec3fa &v)
{
    Vec3fa r{0};
    for (int i = 0; i < 3; i ++) {
        for (int j = 0; j < 3; j ++) {
            r[i] += v[j] * a[j][i];
        }
    }
    return r;
}

inline Vec3fa operator*(const Vec3fa &v, const std::array<Vec3fa, 3> &a)
{
    Vec3fa r{0};
    for (int i = 0; i < 3; i ++) {
        for (int j = 0; j < 3; j ++) {
            r[i] += a[i][j] * v[j];
        }
    }
    return r;
}

inline std::array<Vec3fa, 3> get_rotation_matrix_X(double angle) {
    Vec3fa c0 = {1,0,0};
    Vec3fa c1 = {0, (float) cos(angle), (float) sin(angle)};
    Vec3fa c2 = {0, (float) -sin(angle), (float) cos(angle)};
    return {c0, c1, c2};
}

inline std::array<Vec3fa, 3> get_rotation_matrix_Y(double angle) {
    Vec3fa c0 = {(float) cos(angle), 0, (float) sin(angle)};
    Vec3fa c1 = {0,                     1, 0};
    Vec3fa c2 = {-(float) sin(angle),0, (float) cos(angle)};
    return {c0, c1, c2};
}

inline std::array<Vec3fa, 3> get_rotation_matrix(double angle_x, double angle_y)
{
    return get_rotation_matrix_X(angle_x) * get_rotation_matrix_Y(angle_y);
}

inline std::array<Vec3fa, 3> transpose(const std::array<Vec3fa, 3> &a)
{
    std::array<Vec3fa, 3> a_T = {Vec3fa{0}, Vec3fa{0}, Vec3fa{0}};
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            a_T[i][j] = a[j][i];
        }
    }
    return a_T;
}


inline Vec3fa to_local(const std::array<Vec3fa,3>& Rcols, const Vec3fa& T, const Vec3fa& p_world) {
    // columns form R; transpose(R) times (p - T)
    Vec3fa pw = p_world - T;
    return Vec3fa{
            dot(pw, Rcols[0]),  // dot with x_tilted
            dot(pw, Rcols[1]),  // dot with y_tilted
            dot(pw, Rcols[2])   // dot with z_tilted
    };
}

inline Vec3fa dir_to_local(const std::array<Vec3fa,3>& Rcols, const Vec3fa& v_world) {
    return Vec3fa{
            dot(v_world, Rcols[0]),
            dot(v_world, Rcols[1]),
            dot(v_world, Rcols[2])
    };
}

inline Vec3fa to_world(const std::array<Vec3fa,3>& Rcols, const Vec3fa& T, const Vec3fa& p_local) {
    // R * p_local + T (columns * components)
    return T + Rcols[0]*p_local.x + Rcols[1]*p_local.y + Rcols[2]*p_local.z;
}

inline Vec3fa normal_to_world(const std::array<Vec3fa,3>& Rcols, const Vec3fa& n_local) {
    // pure rotation -> normal transforms like a direction
    return Rcols[0]*n_local.x + Rcols[1]*n_local.y + Rcols[2]*n_local.z;
}


inline Vec3fa rotate_to_Z(const Vec3fa& v, const Vec3fa& n_raw) {
    const Vec3fa z(0.0f, 0.0f, 1.0f);
    const Vec3fa n = normalize(n_raw);

    float c = dot(n, z);                   // cos(theta)
    c = std::max(-1.0f, std::min(1.0f, c));
    Vec3fa axis = cross(n, z);
    float s = sqrt(dot(axis, axis));                // |sin(theta)|

    if (s < 1e-7f) {
        if (c > 0.0f) {
            // n already aligned with +Z
            return v;
        } else {
            // n is ~opposite Z: 180° around any axis ⟂ n
            axis = cross(n, Vec3fa(1,0,0));
            if (sqrt(dot(axis, axis)) < 1e-7f) axis = cross(n, Vec3fa(0,1,0));
            axis = normalize(axis);
            return axis;
        }
    }

    axis = axis / s;                             // normalize
    float theta = std::atan2(s, c);        // robust angle
    return axis;
}

#endif //VEC3FA_H
