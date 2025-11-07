/*
Copyright (C) 2025  Neo Reinmann (neoreinmann@gmail.com)
*/

#include "EmbreeScene.h"

void EmbreeScene::initializeDevice()
{
    device_ = rtcNewDevice(NULL);

    if (!device_)
        printf("error %d: cannot create device\n", rtcGetDeviceError(NULL));

    rtcSetDeviceErrorFunction(device_, errorFunction, NULL);
}

void EmbreeScene::errorFunction([[maybe_unused]] void *userPtr, [[maybe_unused]] enum RTCError error, const char *str)
{
    printf("error %d: %s\n", error, str);
}

bool EmbreeScene::reflect_ray(Ray &ray) {
    double angle = get_angle(ray.normal(), ray.direction());
    // Return position but no direction because ray is now trapped
    if (angle - M_PI / 2 < 0) {
        return false;
    }

    ray.set_position(ray.position() + ray.rayhit.ray.tfar * ray.direction());
    ray.set_direction(reflect(ray.direction(), ray.normal()));
    ray.rayhit.ray.tnear = 0.0001;
    ray.rayhit.ray.tfar = std::numeric_limits<float>::infinity();
    ray.rayhit.ray.mask = -1;
    ray.rayhit.ray.flags = 0;
    ray.rayhit.hit.geomID = RTC_INVALID_GEOMETRY_ID;
    ray.rayhit.hit.primID = 0;
    ray.rayhit.hit.instID[0] = RTC_INVALID_GEOMETRY_ID;
    return true;
}

unsigned int EmbreeScene::addSTLMesh(const std::string& path, const Vec3fa& position) {
    stl_reader::StlMesh <float, unsigned int> mesh (path);
    RTCGeometry rtcMesh = rtcNewGeometry (device_, RTC_GEOMETRY_TYPE_TRIANGLE);
    unsigned* indices = (unsigned*) rtcSetNewGeometryBuffer(rtcMesh,
                                                            RTC_BUFFER_TYPE_INDEX,
                                                            0,
                                                            RTC_FORMAT_UINT3,
                                                            3*sizeof(unsigned),
                                                            mesh.num_tris());
    float* vertices = (float*) rtcSetNewGeometryBuffer(rtcMesh,
                                                       RTC_BUFFER_TYPE_VERTEX,
                                                       0,
                                                       RTC_FORMAT_FLOAT3,
                                                       3*sizeof(float),
                                                mesh.num_tris()*3);
    int counter_indices = 0;
    int counter_corner = 0;

    for(size_t itri = 0; itri < mesh.num_tris(); ++itri) {
        for(size_t icorner = 0; icorner < 3; ++icorner) {
            const float* c = mesh.tri_corner_coords (itri, icorner);
            if (vertices && indices)
            {
                vertices[counter_corner] = c[0] + position.x;
                counter_corner++;
                vertices[counter_corner] = c[1] + position.y;
                counter_corner++;
                vertices[counter_corner] = c[2] + position.z;
                counter_corner++;
            }
        }
        if (indices) {
            indices[counter_indices] = counter_indices;
            counter_indices++;
            indices[counter_indices] = counter_indices;
            counter_indices++;
            indices[counter_indices] = counter_indices;
            counter_indices++;
        }
    }
    rtcCommitGeometry(rtcMesh);
    const unsigned int geomID = rtcAttachGeometry(scene_, rtcMesh);
    rtcReleaseGeometry(rtcMesh);
    return geomID;
}
