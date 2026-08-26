#include "../../src/file_system/data_block.h"

#include "unity.h"

block DUMMY_DATA_BLOCK1 = {"hello world"};
block DUMMY_DATA_BLOCK2 = {1, 2, 3, 4, 5};

/*
 * === DATA_BLOCK_ALLOC ===
 */

void test_data_block_alloc_allocates_first_free_slot(void) {
    // check returned slot values
    TEST_ASSERT_EQUAL_INT(0, data_block_alloc());
    TEST_ASSERT_EQUAL_INT(1, data_block_alloc());
}

void test_data_block_alloc_uses_lowest_available_slot(void) {
    // alloc several data blocks
    TEST_ASSERT_EQUAL_INT(0, data_block_alloc());
    TEST_ASSERT_EQUAL_INT(1, data_block_alloc());
    TEST_ASSERT_EQUAL_INT(2, data_block_alloc());

    // free one in the middle
    TEST_ASSERT_EQUAL_INT(1, data_block_free(1));

    // allocate again, should fill the hole
    TEST_ASSERT_EQUAL_INT(1, data_block_alloc());
}

void test_data_block_alloc_can_fill_data_region(void) {
    for (int data_block = 0; data_block < NUM_DATA_BLOCKS; data_block++) {
        TEST_ASSERT_EQUAL_INT(data_block, data_block_alloc());
    }
}

void test_data_block_alloc_fails_when_data_region_full(void) {
    // fill data region
    for (int data_block = 0; data_block < NUM_DATA_BLOCKS; data_block++) {
        TEST_ASSERT_EQUAL_INT(data_block, data_block_alloc());
    }

    // next allocation should fail
    TEST_ASSERT_EQUAL_INT(-1, data_block_alloc());
}

/*
 * === DATA_BLOCK_FREE ===
 */

void test_data_block_free_marks_block_available(void) {
    // allocate should succeed
    TEST_ASSERT_EQUAL_INT(0, data_block_alloc());

    // free should succeed
    TEST_ASSERT_EQUAL_INT(0, data_block_free(0));

    // next allocation should claim previously freed block
    TEST_ASSERT_EQUAL_INT(0, data_block_alloc());
}

void test_data_block_free_fails_when_block_already_free(void) {
    // free unclaimed data block should fail
    TEST_ASSERT_EQUAL_INT(-1, data_block_free(0));
}

void test_data_block_free_invalid_inputs(void) {
    // negative data block number
    TEST_ASSERT_EQUAL_INT(-1, data_block_free(-1));
    // data block number out of bounds
    TEST_ASSERT_EQUAL_INT(-1, data_block_free(NUM_DATA_BLOCKS));
}

/*
 * === DATA_BLOCK_READ ===
 */

void test_data_block_read_reads_block_at_requested_slot(void) {
    // allocate should succeed
    TEST_ASSERT_EQUAL_INT(0, data_block_alloc());

    // write to block
    TEST_ASSERT_EQUAL_INT(0, data_block_write(&DUMMY_DATA_BLOCK1, 0));

    // read should succeed
    block out;
    TEST_ASSERT_EQUAL_INT(0, data_block_read(&out, 0));

    // check contents
    TEST_ASSERT_EQUAL_MEMORY(&DUMMY_DATA_BLOCK1, &out, sizeof(block));
}

void test_data_block_read_invalid_inputs(void) {
    block out;
    // null ptr
    TEST_ASSERT_EQUAL_INT(-1, data_block_read(NULL, 0));
    // negative data block
    TEST_ASSERT_EQUAL_INT(-1, data_block_read(&out, -1));
    // data block out of bounds
    TEST_ASSERT_EQUAL_INT(-1, data_block_read(&out, NUM_DATA_BLOCKS));
}

/*
 * === DATA_BLOCK_WRITE ===
 */

void test_data_block_write_updates_only_requested_block(void) {
    // allocations should succeed
    TEST_ASSERT_EQUAL_INT(0, data_block_alloc());
    TEST_ASSERT_EQUAL_INT(1, data_block_alloc());

    // writes should succeed
    TEST_ASSERT_EQUAL_INT(0, data_block_write(&DUMMY_DATA_BLOCK1, 0));
    TEST_ASSERT_EQUAL_INT(0, data_block_write(&DUMMY_DATA_BLOCK1, 1));

    // update data block 0
    TEST_ASSERT_EQUAL_INT(0, data_block_write(&DUMMY_DATA_BLOCK2, 0));

    // data block 1 should remain unchanged
    block out;
    TEST_ASSERT_EQUAL_INT(0, data_block_read(&out, 1));
    TEST_ASSERT_EQUAL_MEMORY(&DUMMY_DATA_BLOCK1, &out, sizeof(block));

    // data block 0 should be updated
    TEST_ASSERT_EQUAL_INT(0, data_block_read(&out, 0));
    TEST_ASSERT_EQUAL_MEMORY(&DUMMY_DATA_BLOCK2, &out, sizeof(block));
}

void test_data_block_write_invalid_inputs(void) {
    // null ptr
    TEST_ASSERT_EQUAL_INT(-1, data_block_write(NULL, 0));
    // negative data block
    TEST_ASSERT_EQUAL_INT(-1, data_block_write(&DUMMY_DATA_BLOCK1, -1));
    // data block out of bounds
    TEST_ASSERT_EQUAL_INT(
        -1, data_block_write(&DUMMY_DATA_BLOCK1, NUM_DATA_BLOCKS));
}
