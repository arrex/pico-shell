#include "../../src/fs/file.h"

#include <limits.h>

#include "../../src/fs/block.h"
#include "unity.h"

// TODO: hook inode allocation and other setup into scaffold
// might break other unit tests, so wait until fs init logic is
// stable before
static void setup(void) {
    struct inode fixture = {
        .type = DIRECTORY_T,
        .addrs = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}};

    TEST_ASSERT_EQUAL_INT(0, inode_alloc(&fixture));
}

/*
 * === FILE_WRITE ===
 */

void test_file_write_with_and_without_offset(void) {
    setup();
    struct inode inode;
    TEST_ASSERT_EQUAL_INT(0, inode_read(&inode, 0));

    // write to at offset 0 first to prevent offset violation
    block data = {0};
    TEST_ASSERT_EQUAL_INT(sizeof(data),
                          file_write(&inode, (char*)&data, 0, sizeof(data)));

    // check inode metadata
    TEST_ASSERT_EQUAL_INT(sizeof(data), inode.size);

    // write a whole block with offset to force boundary cross
    TEST_ASSERT_EQUAL_INT(sizeof(data),
                          file_write(&inode, (char*)&data, 64, sizeof(data)));

    // check updated inode metadata
    TEST_ASSERT_EQUAL_INT(sizeof(data) + 64, inode.size);
}

void test_file_write_can_write_to_entire_address_space(void) {
    setup();
    struct inode inode;
    TEST_ASSERT_EQUAL_INT(0, inode_read(&inode, 0));

    // write to each direct block
    block data = {0};
    for (int i = 0; i < NDIRECT; i++) {
        int offset = i * BLOCK_SIZE;
        TEST_ASSERT_EQUAL_INT(sizeof(data), file_write(&inode, (char*)&data,
                                                       offset, sizeof(data)));
    }

    // check inode size
    TEST_ASSERT_EQUAL_INT(BLOCK_SIZE * NDIRECT, inode.size);
}

void test_file_write_with_zero_byte_write(void) {
    setup();
    struct inode inode;
    TEST_ASSERT_EQUAL_INT(0, inode_read(&inode, 0));

    // write should succeed
    block data = {0};
    TEST_ASSERT_EQUAL_INT(0, file_write(&inode, (char*)&data, 0, 0));
    // inode size should still be 0
    TEST_ASSERT_EQUAL_INT(0, inode.size);
}

void test_file_write_invalid_inputs(void) {
    setup();
    struct inode inode;
    TEST_ASSERT_EQUAL_INT(0, inode_read(&inode, 0));

    block data = {0};
    // null ptr inode
    TEST_ASSERT_EQUAL_INT(-1, file_write(NULL, (char*)&data, 0, sizeof(data)));
    // null data buffer
    TEST_ASSERT_EQUAL_INT(-1, file_write(&inode, NULL, 0, sizeof(data)));
    // offset greater than curr file size (which is 0 since we haven't written
    // anything yet)
    TEST_ASSERT_EQUAL_INT(-1,
                          file_write(&inode, (char*)&data, 1, sizeof(data)));
    // offset and byte count will cause overflow
    TEST_ASSERT_EQUAL_INT(-1, file_write(&inode, NULL, INT_MAX, sizeof(data)));
    // offset and byte count exceed maximum file size allowed
    TEST_ASSERT_EQUAL_INT(
        -1, file_write(&inode, NULL, 0, NDIRECT * BLOCK_SIZE + 1));
    // inode size should still be 0
    TEST_ASSERT_EQUAL_INT(0, inode.size);
}

/*
 * === FILE_READ ===
 */

void test_file_read_round_trip(void) {
    setup();
    struct inode inode;
    TEST_ASSERT_EQUAL_INT(0, inode_read(&inode, 0));

    // write should succeed
    block data = {1, 2, 3};  // rest 0-initialized...
    TEST_ASSERT_EQUAL_INT(sizeof(data),
                          file_write(&inode, (char*)&data, 0, sizeof(data)));

    // read should succeed
    char out[sizeof(data)];
    TEST_ASSERT_EQUAL_INT(sizeof(data),
                          file_read(&inode, out, 0, sizeof(data)));

    // data read should match
    TEST_ASSERT_EQUAL_MEMORY(&data, &out, sizeof(data));

    // new write with offset should succeed
    block new_data = {4, 5, 6};  // rest are 0's
    uint offset = 64;
    TEST_ASSERT_EQUAL_INT(
        sizeof(new_data),
        file_write(&inode, (char*)&new_data, offset, sizeof(new_data)));

    // read with offset should succeed
    TEST_ASSERT_EQUAL_INT(sizeof(new_data),
                          file_read(&inode, out, offset, sizeof(new_data)));

    // data read should match
    TEST_ASSERT_EQUAL_MEMORY(&new_data, &out, sizeof(new_data));

    // check that offset left surrounding bytes intact
    char out2[64];
    TEST_ASSERT_EQUAL_INT(64, file_read(&inode, out2, 0, sizeof(out2)));

    // should match what was previously there
    TEST_ASSERT_EQUAL_MEMORY(&data, &out2, sizeof(out2));
}

void test_file_read_can_read_from_entire_address_space(void) {
    setup();
    struct inode inode;
    TEST_ASSERT_EQUAL_INT(0, inode_read(&inode, 0));

    block data = {0};
    for (int i = 0; i < NDIRECT; i++) {
        // set first byte to ix value
        data[0] = i;
        uint offset = i * BLOCK_SIZE;

        // write should succeed
        TEST_ASSERT_EQUAL_INT(sizeof(data), file_write(&inode, (char*)&data,
                                                       offset, sizeof(data)));

        // read should succeed
        block out;
        TEST_ASSERT_EQUAL_INT(
            sizeof(data), file_read(&inode, (char*)&out, offset, sizeof(out)));

        // output read should match
        TEST_ASSERT_EQUAL_MEMORY(data, out, sizeof(data));
    }
}

void test_file_read_with_zero_byte_read(void) {
    setup();
    struct inode inode;
    TEST_ASSERT_EQUAL_INT(0, inode_read(&inode, 0));

    // read should succeed
    char out[0];
    TEST_ASSERT_EQUAL_INT(0, file_read(&inode, out, 0, 0));
}

void test_file_read_invalid_inputs(void) {
    setup();
    struct inode inode;
    TEST_ASSERT_EQUAL_INT(0, inode_read(&inode, 0));

    char out[BLOCK_SIZE];
    // null ptr inode
    TEST_ASSERT_EQUAL_INT(-1, file_read(NULL, out, 0, sizeof(out)));
    // null data buffer
    TEST_ASSERT_EQUAL_INT(-1, file_read(&inode, NULL, 0, sizeof(out)));
    // offset greater than curr file size (which is 0 since we haven't written
    // anything yet)
    TEST_ASSERT_EQUAL_INT(-1, file_read(&inode, out, 1, sizeof(out)));
    // offset and byte count will cause overflow
    TEST_ASSERT_EQUAL_INT(-1, file_read(&inode, NULL, INT_MAX, sizeof(out)));
    // offset and byte count exceed maximum file size allowed
    TEST_ASSERT_EQUAL_INT(-1,
                          file_read(&inode, NULL, 0, NDIRECT * BLOCK_SIZE + 1));
}
