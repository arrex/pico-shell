#include "../../src/file_system/disk.h"

#include <stdint.h>
#include <string.h>

#include "unity.h"

void assert_disk_equals(const uint8_t* expected) {
    uint8_t actual[DISK_SIZE];

    TEST_ASSERT_EQUAL_INT(0, disk_read(actual, DISK_SIZE, 0));
    TEST_ASSERT_EQUAL_UINT8_ARRAY(expected, actual, DISK_SIZE);
}

void test_disk_starts_at_all_zeroes(void) {
    int err_code;

    // read should succeed
    uint8_t out[DISK_SIZE];
    err_code = disk_read(out, DISK_SIZE, 0);
    TEST_ASSERT_EQUAL_INT(0, err_code);

    // disk bytes should all be 0
    uint8_t expected[DISK_SIZE] = {0};
    TEST_ASSERT_EQUAL_UINT8_ARRAY(expected, out, DISK_SIZE);
}

void test_read_write_round_trip(void) {
    int err_code;

    // write should succeed
    uint8_t in[] = {1, 2, 3, 4};
    err_code = disk_write(in, 4, 4);
    TEST_ASSERT_EQUAL_INT(0, err_code);

    // read should succeed
    uint8_t out[4];
    err_code = disk_read(out, 4, 4);
    TEST_ASSERT_EQUAL_INT(0, err_code);

    // read data should match
    uint8_t expected[] = {1, 2, 3, 4};
    TEST_ASSERT_EQUAL_UINT8_ARRAY(expected, out, 4);

    // we wrote at offset 4, neighbouring bytes should be unchanged

    // read left 4 bytes should succeed
    err_code = disk_read(out, 4, 0);
    TEST_ASSERT_EQUAL_INT(0, err_code);

    // bytes should all be 0
    uint8_t expected2[] = {0, 0, 0, 0};
    TEST_ASSERT_EQUAL_UINT8_ARRAY(expected2, out, 4);

    // read right 4 bytes should succeed
    err_code = disk_read(out, 4, 8);
    TEST_ASSERT_EQUAL_INT(0, err_code);

    // bytes should all be 0
    TEST_ASSERT_EQUAL_UINT8_ARRAY(expected2, out, 4);
}

void test_overwrite_existing_data(void) {
    int err_code;

    // initial write should succeed
    uint8_t in1[] = {1, 2, 3, 4};
    err_code = disk_write(in1, 4, 0);
    TEST_ASSERT_EQUAL_INT(0, err_code);

    // second write should also succeed
    uint8_t in2[] = {5, 6, 7, 8};
    err_code = disk_write(in2, 4, 0);
    TEST_ASSERT_EQUAL_INT(0, err_code);

    // read should succeed
    uint8_t out[4] = {0};
    err_code = disk_read(out, 4, 0);
    TEST_ASSERT_EQUAL_INT(0, err_code);

    // data should be overwritten
    uint8_t expected[] = {5, 6, 7, 8};
    TEST_ASSERT_EQUAL_UINT8_ARRAY(expected, out, 4);
}

void test_invalid_read_inputs_leave_disk_unchanged(void) {
    uint8_t expected[DISK_SIZE] = {0};
    uint8_t out[DISK_SIZE];

    // null buffer
    TEST_ASSERT_EQUAL_INT(-1, disk_read(NULL, 1, 0));
    // negative size
    TEST_ASSERT_EQUAL_INT(-1, disk_read(out, -1, 0));
    // size greater than disk size
    TEST_ASSERT_EQUAL_INT(-1, disk_read(out, DISK_SIZE + 1, 0));
    // invalid size and offset combination
    TEST_ASSERT_EQUAL_INT(-1, disk_read(out, DISK_SIZE, 1));
    // negative offset
    TEST_ASSERT_EQUAL_INT(-1, disk_read(out, 4, -1));
    // offset greater than disk size
    TEST_ASSERT_EQUAL_INT(-1, disk_read(out, 4, DISK_SIZE));

    // check disk unchanged
    assert_disk_equals(expected);
}

void test_invalid_write_inputs_leave_disk_unchanged(void) {
    uint8_t expected[DISK_SIZE] = {0};
    uint8_t input[] = {1, 2, 3, 4};

    // write trividal data to disk
    TEST_ASSERT_EQUAL_INT(0, disk_write(input, sizeof(input), 0));
    memcpy(expected, input, sizeof(input));

    // null buffer
    TEST_ASSERT_EQUAL_INT(-1, disk_write(NULL, 1, 0));
    // negative size
    TEST_ASSERT_EQUAL_INT(-1, disk_write(input, -1, 0));
    // size greater than disk size
    TEST_ASSERT_EQUAL_INT(-1, disk_write(input, DISK_SIZE + 1, 0));
    // invalid size and offset combination
    TEST_ASSERT_EQUAL_INT(-1, disk_write(input, DISK_SIZE, 1));
    // negative offset
    TEST_ASSERT_EQUAL_INT(-1, disk_write(input, 4, -1));
    // offset greater than disk size
    TEST_ASSERT_EQUAL_INT(-1, disk_write(input, 4, DISK_SIZE));

    // check disk unchanged
    assert_disk_equals(expected);
}

void test_read_write_at_final_disk_byte(void) {
    uint8_t input = 42;
    uint8_t out = 0;

    // write at boundary
    TEST_ASSERT_EQUAL_INT(0, disk_write(&input, 1, DISK_SIZE - 1));
    // read at boundary
    TEST_ASSERT_EQUAL_INT(0, disk_read(&out, 1, DISK_SIZE - 1));
    // written data should be retrieved
    TEST_ASSERT_EQUAL_UINT8(input, out);
}
