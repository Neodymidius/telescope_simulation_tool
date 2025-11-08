FIND_PACKAGE(embree QUIET)
IF (embree_FOUND)
    LIST(APPEND CPP_TOOLS
            raytracing
    )

ENDIF ()


set(RAYTRACING_SOURCES
        raytracing/raytracing.cpp
)

foreach (TOOL ${CPP_TOOLS})
    if (TOOL STREQUAL "raytracing")
        add_executable(${TOOL} ${RAYTRACING_SOURCES}) # Add ray.cpp here
    else ()
        add_executable(${TOOL} "${TOOL}.cpp")
    endif ()
endforeach ()

foreach (TOOL ${CPP_TOOLS})
    target_include_directories(${TOOL} PUBLIC
            target_link_libraries (${TOOL} PUBLIC
            compiler_flags
    )

            install (TARGETS ${TOOL})
            if (NOT ${TOOL} IN_LIST TOOLS_WITHOUT_PARFILES)
            install (FILES ${TOOL}/${TOOL}.par DESTINATION share/sixte/pfiles)
            endif ()

            ### Apple Adjustments for missing boost headers ###
            if (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
            target_compile_definitions (${TOOL} PUBLIC unary_function=__unary_function)
            target_compile_definitions (${TOOL} PUBLIC binary_function=__binary_function)
            endif ()
            endforeach ()
