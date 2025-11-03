# raytracing/CMakeLists.txt

# Define the sources and headers for the raytracing components.
set(RAYTRACING_SOURCES
        shape/Hyperboloid.cpp
        shape/Paraboloid.cpp
        shape/Spider.cpp
        shape/Shape.cpp
        shape/Plane.cpp
        geometry/Ray.cpp
        mirror_module/MirrorModule.cpp
        mirror_module/Wolter.cpp
        surface/GaussSurface.cpp
        surface/SurfaceModel.cpp
        surface/SurfaceStrategy.cpp
        mirror_module/EmbreeScene.cpp
        sensor/Sensor.cpp
        surface/Dummy.cpp
        mirror_module/LobsterEyeOptic.cpp
        surface/Microfacet.cpp
        shape/OpticalMesh.cpp
        lib/XMLData.cpp

)

set(RAYTRACING_HEADERS
        shape/Plane.h
        geometry/Ray.h
        mirror_module/MirrorModule.h
        mirror_module/Wolter.h
        Raytracing.h
        shape/Hyperboloid.h
        shape/Paraboloid.h
        shape/Spider.h
        shape/Shape.h
        surface/GaussSurface.h
        surface/SurfaceModel.h
        surface/SurfaceStrategy.h
        mirror_module/EmbreeScene.h
        sensor/Sensor.h
        lib/stl_reader.h
        surface/Dummy.h
        surface/Microfacet.h
        mirror_module/LobsterEyeOptic.h
        shape/OpticalMesh.h
        shape/Pore.cpp
        shape/Pore.h
        lib/random.h
        lib/XMLData.h

)

# Create a shared library target for raytracing components.
add_library(raytracing_objects SHARED  ${RAYTRACING_SOURCES} ${RAYTRACING_HEADERS})
set_property(TARGET raytracing_objects PROPERTY POSITION_INDEPENDENT_CODE ON)

target_include_directories(raytracing_objects PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}       # the raytracing directory itself
)
target_link_libraries(raytracing_objects PUBLIC
        compiler_flags
        )

find_package(pugixml REQUIRED)
IF (pugixml_FOUND)
    message(STATUS "✔ Found pugixml ${MyLib_VERSION}")
ENDIF ()

FIND_PACKAGE(embree QUIET)
IF (embree_FOUND)
    message(STATUS "✔ Found EMBREE ${MyLib_VERSION}")
ENDIF ()
message(STATUS ${CMAKE_INSTALL_LIBDIR})
install(TARGETS raytracing_objects
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
target_link_libraries(raytracing_objects PUBLIC embree pugixml::pugixml)
