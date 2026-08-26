# file system tests
set(FILE_SYSTEM_TEST "${PROJECT_NAME}_file_system_tests")

add_executable(${FILE_SYSTEM_TEST}
  file_system/test.c
)

add_test(NAME file_system_tests COMMAND ${FILE_SYSTEM_TEST})
