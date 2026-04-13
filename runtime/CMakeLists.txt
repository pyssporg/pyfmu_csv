set(PYFMU_CSV_RUNTIME_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(PYFMU_CSV_FMI_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/3rd_party/fmi_2/headers)

add_library(pyfmu_csv_runtime_core STATIC
    src/fmu_runtime.cpp
)

target_include_directories(pyfmu_csv_runtime_core
    PUBLIC
        ${PYFMU_CSV_RUNTIME_INCLUDE_DIR}
)

target_compile_features(pyfmu_csv_runtime_core PUBLIC cxx_std_17)
set_target_properties(pyfmu_csv_runtime_core PROPERTIES POSITION_INDEPENDENT_CODE ON)

add_library(pyfmu_csv_fmi2_cs SHARED
    src/fmi2_cs_api.cpp
)

target_include_directories(pyfmu_csv_fmi2_cs
    PUBLIC
        ${PYFMU_CSV_RUNTIME_INCLUDE_DIR}
        ${PYFMU_CSV_FMI_INCLUDE_DIR}
)

target_link_libraries(pyfmu_csv_fmi2_cs
    PRIVATE
        pyfmu_csv_runtime_core
)

target_compile_features(pyfmu_csv_fmi2_cs PUBLIC cxx_std_17)

if(BUILD_TESTING AND PYFMU_CSV_BUILD_TESTS)
    add_executable(pyfmu_csv_runtime_smoke_test
        tests/test_runtime_smoke.cpp
    )

    target_link_libraries(pyfmu_csv_runtime_smoke_test
        PRIVATE
            pyfmu_csv_runtime_core
    )

    target_include_directories(pyfmu_csv_runtime_smoke_test
        PRIVATE
            ${PYFMU_CSV_RUNTIME_INCLUDE_DIR}
    )

    add_test(
        NAME pyfmu_csv_runtime_smoke_test
        COMMAND pyfmu_csv_runtime_smoke_test
    )

    add_executable(pyfmu_csv_fmi2_api_smoke_test
        tests/test_fmi2_api_smoke.cpp
    )

    target_link_libraries(pyfmu_csv_fmi2_api_smoke_test
        PRIVATE
            pyfmu_csv_fmi2_cs
    )

    target_include_directories(pyfmu_csv_fmi2_api_smoke_test
        PRIVATE
            ${PYFMU_CSV_RUNTIME_INCLUDE_DIR}
            ${PYFMU_CSV_FMI_INCLUDE_DIR}
    )

    add_test(
        NAME pyfmu_csv_fmi2_api_smoke_test
        COMMAND pyfmu_csv_fmi2_api_smoke_test
    )
endif()
