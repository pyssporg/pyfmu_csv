add_library(pyfmu_csv_runtime STATIC
    src/fmu_runtime.cpp
)

target_include_directories(pyfmu_csv_runtime
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(pyfmu_csv_runtime PUBLIC cxx_std_17)

if(BUILD_TESTING AND PYFMU_CSV_BUILD_TESTS)
    add_executable(pyfmu_csv_runtime_smoke_test
        tests/test_runtime_smoke.cpp
    )

    target_link_libraries(pyfmu_csv_runtime_smoke_test
        PRIVATE
            pyfmu_csv_runtime
    )

    add_test(
        NAME pyfmu_csv_runtime_smoke_test
        COMMAND pyfmu_csv_runtime_smoke_test
    )
endif()
