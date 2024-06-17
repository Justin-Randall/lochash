

if(NOT DEFINED HERALD_DISABLE_TESTING)
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

  include(FetchContent)
  FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG f8d7d77c06936315286eb55f8de22cd23c188571 # release-1.14.0
  )
  FetchContent_MakeAvailable(googletest)

  set(UNIT_TEST ${PROJECT_NAME}_tests)

  add_executable(${UNIT_TEST} "test_${PROJECT_NAME}.cpp"
    "test_location_hash_algorithm.cpp")

  target_compile_features(${UNIT_TEST} PRIVATE cxx_std_20)

  target_link_libraries(${UNIT_TEST} PRIVATE gtest_main)

  add_test(NAME ${UNIT_TEST} COMMAND ${UNIT_TEST})

  include_directories("${gtest_SOURCE_DIR}/include")

  find_program(OpenCppCoverage_BINARY OpenCppCoverage.exe)

  set(TEST_COMMAND_LINE_OPTIONS ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION> -R "^${UNIT_TEST}$" --output-on-failures)

  if(OpenCppCoverage_BINARY)
    add_custom_command(
      TARGET ${UNIT_TEST}
      COMMENT "Run coverage report and tests"
      POST_BUILD
      WORKING_DIRECTORY ${CMAKE_TEST_DIRECTORY}
      COMMAND ${OpenCppCoverage_BINARY} --quiet --export_type cobertura:${CMAKE_SOURCE_DIR}/build/coverage.xml --cover_children --sources herald --excluded_modules system32 --excluded_sources build\\_deps --excluded_sources googletest\\include\\gtest -- ${TEST_COMMAND_LINE_OPTIONS}
    )
  else()
    add_custom_command(
      TARGET ${UNIT_TEST}
      COMMENT "Run tests"
      POST_BUILD
      WORKING_DIRECTORY ${CMAKE_TEST_DIRECTORY}
      COMMAND ${TEST_COMMAND_LINE_OPTIONS}
    )
  endif()
endif()
