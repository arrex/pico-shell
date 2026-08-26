#include "../../src/file_system/block_layer.h"

#include <stdint.h>
#include <string.h>

#include "unity.h"

void test_block_starts_as_zeroes(void) {
    // read should succeed
    block block;
    TEST_ASSERT_EQUAL_INT(0, block_read(&block, 0));

    // check if all 0's
    TEST_ASSERT_EACH_EQUAL_UINT8(0, block, BLOCK_SIZE);
}

void test_block_write_read_round_trip(void) {
    // write trivial value to block
    block block;
    memset(&block, 8, BLOCK_SIZE);

    // write block to disk at boundary
    TEST_ASSERT_EQUAL_INT(0, block_write(&block, NUM_BLOCKS - 1));

    // read block we just wrote to
    TEST_ASSERT_EQUAL_INT(0, block_read(&block, NUM_BLOCKS - 1));

    // check value read
    TEST_ASSERT_EACH_EQUAL_UINT8(8, block, BLOCK_SIZE);
}

void test_block_write_only_changes_target_block(void) {
    // write trivial value to block
    block block;
    memset(&block, 8, BLOCK_SIZE);

    // write block value to block 1 on disk
    TEST_ASSERT_EQUAL_INT(0, block_write(&block, 1));

    // read block 0 and check that it is unchanged
    TEST_ASSERT_EQUAL_INT(0, block_read(&block, 0));
    TEST_ASSERT_EACH_EQUAL_UINT8(0, block, BLOCK_SIZE);

    // read block 2 and check that it is unchanged
    TEST_ASSERT_EQUAL_INT(0, block_read(&block, 2));
    TEST_ASSERT_EACH_EQUAL_UINT8(0, block, BLOCK_SIZE);
}

void test_block_writes_are_independent(void) {
    // init first dummy block
    block block;
    memset(&block, 1, BLOCK_SIZE);

    // write first dummy block to disk at block 0
    TEST_ASSERT_EQUAL_INT(0, block_write(&block, 0));

    // init second dummy block using same ref
    memset(&block, 2, BLOCK_SIZE);

    // write second value block to disk at block 1
    TEST_ASSERT_EQUAL_INT(0, block_write(&block, 1));

    // read block 0 from disk and check its elmts
    TEST_ASSERT_EQUAL_INT(0, block_read(&block, 0));
    TEST_ASSERT_EACH_EQUAL_UINT8(1, block, BLOCK_SIZE);

    // read block 1 from disk and check its elmts
    TEST_ASSERT_EQUAL_INT(0, block_read(&block, 1));
    TEST_ASSERT_EACH_EQUAL_UINT8(2, block, BLOCK_SIZE);
}

void test_invalid_read_inputs_preserve_state(void) {
    // write trivial block to disk
    block expected;
    memset(&expected, 8, BLOCK_SIZE);
    // write to block 0
    TEST_ASSERT_EQUAL_INT(0, block_write(&expected, 0));

    // test with invalid inputs
    block out;
    // null buffer
    TEST_ASSERT_EQUAL_INT(-1, block_read(NULL, 0));
    // negative block number
    TEST_ASSERT_EQUAL_INT(-1, block_read(&out, -1));
    // out of bounds block number
    TEST_ASSERT_EQUAL_INT(-1, block_read(&out, NUM_BLOCKS));

    // check state preserved
    block actual;
    TEST_ASSERT_EQUAL_INT(0, block_read(&actual, 0));
    TEST_ASSERT_EQUAL_UINT8_ARRAY(expected, actual, BLOCK_SIZE);
}

void test_invalid_write_inputs_preserve_state(void) {
    // write trivial block to disk
    block expected;
    memset(expected, 8, BLOCK_SIZE);
    // write to block 2
    TEST_ASSERT_EQUAL_INT(0, block_write(&expected, 2));

    // null buffer
    TEST_ASSERT_EQUAL_INT(-1, block_write(NULL, -1));
    // negative block number
    TEST_ASSERT_EQUAL_INT(-1, block_write(&expected, -1));
    // out of bounds block number
    TEST_ASSERT_EQUAL_INT(-1, block_write(&expected, NUM_BLOCKS));

    // check state preserved
    block actual;
    TEST_ASSERT_EQUAL_INT(0, block_read(&actual, 2));
    TEST_ASSERT_EQUAL_UINT8_ARRAY(expected, actual, BLOCK_SIZE);
}
