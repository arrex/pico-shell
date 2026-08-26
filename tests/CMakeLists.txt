# file system tests
set(FILE_SYSTEM_TEST "${PROJECT_NAME}_file_system_tests")

add_executable(${FILE_SYSTEM_TEST}
  ${PROJECT_SOURCE_DIR}/src/file_system/disk.c
  file_system/disk_test.c
)

target_compile_definitions(${FILE_SYSTEM_TEST} PRIVATE PICOSHELL_TESTING)

target_link_libraries(${FILE_SYSTEM_TEST} PRIVATE unity)

add_test(NAME file_system_tests COMMAND ${FILE_SYSTEM_TEST})
