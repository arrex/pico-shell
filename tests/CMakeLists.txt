# file system tests
set(FILE_SYSTEM_TEST "${PROJECT_NAME}_fs_tests")

add_executable(${FILE_SYSTEM_TEST}
  # src files
  ${PROJECT_SOURCE_DIR}/src/fs/bitmap.c
  ${PROJECT_SOURCE_DIR}/src/fs/data.c
  ${PROJECT_SOURCE_DIR}/src/fs/block.c
  ${PROJECT_SOURCE_DIR}/src/fs/disk.c
  ${PROJECT_SOURCE_DIR}/src/fs/inode.c
  ${PROJECT_SOURCE_DIR}/src/fs/file.c
  ${PROJECT_SOURCE_DIR}/src/utils/utils.c
  # unit test files
  fs/block_test.c
  fs/disk_test.c
  fs/inode_test.c
  fs/data_test.c
  fs/file_test.c
  # runner test file
  fs/test_runner.c
)

target_compile_definitions(${FILE_SYSTEM_TEST} PRIVATE PICOSHELL_TESTING)

target_link_libraries(${FILE_SYSTEM_TEST} PRIVATE unity)

add_test(NAME file_system_tests COMMAND ${FILE_SYSTEM_TEST})
