#include "../../src/fs/data.h"

#include "unity.h"

static block DATA1 = {"hello world"};
static block DATA2 = {1, 2, 3, 4, 5};

/*
 * === DATA_ALLOC ===
 */

void test_data_alloc_allocates_first_free_slot(void) {
    // check returned slot values
    TEST_ASSERT_EQUAL_INT(0, data_alloc());
    TEST_ASSERT_EQUAL_INT(1, data_alloc());
}

void test_data_alloc_uses_lowest_available_slot(void) {
    // alloc several data blocks
    TEST_ASSERT_EQUAL_INT(0, data_alloc());
    TEST_ASSERT_EQUAL_INT(1, data_alloc());
    TEST_ASSERT_EQUAL_INT(2, data_alloc());

    // free one in the middle
    TEST_ASSERT_EQUAL_INT(0, data_free(1));

    // allocate again, should fill the hole
    TEST_ASSERT_EQUAL_INT(1, data_alloc());
}

void test_data_alloc_can_fill_data_region(void) {
    for (int data = 0; data < NUM_DATA_BLOCKS; data++) {
        TEST_ASSERT_EQUAL_INT(data, data_alloc());
    }
}

void test_data_alloc_fails_when_data_region_full(void) {
    // fill data region
    for (int data = 0; data < NUM_DATA_BLOCKS; data++) {
        TEST_ASSERT_EQUAL_INT(data, data_alloc());
    }

    // next allocation should fail
    TEST_ASSERT_EQUAL_INT(-1, data_alloc());
}

/*
 * === DATA_FREE ===
 */

void test_data_free_marks_block_available(void) {
    // allocate should succeed
    TEST_ASSERT_EQUAL_INT(0, data_alloc());

    // free should succeed
    TEST_ASSERT_EQUAL_INT(0, data_free(0));

    // next allocation should claim previously freed block
    TEST_ASSERT_EQUAL_INT(0, data_alloc());
}

void test_data_free_fails_when_block_already_free(void) {
    // free unclaimed data block should fail
    TEST_ASSERT_EQUAL_INT(-1, data_free(0));
}

void test_data_free_invalid_inputs(void) {
    // negative data block number
    TEST_ASSERT_EQUAL_INT(-1, data_free(-1));
    // data block number out of bounds
    TEST_ASSERT_EQUAL_INT(-1, data_free(NUM_DATA_BLOCKS));
}

/*
 * === DATA_READ ===
 */

void test_data_read_reads_block_at_requested_slot(void) {
    // allocate should succeed
    TEST_ASSERT_EQUAL_INT(0, data_alloc());

    // write to block
    TEST_ASSERT_EQUAL_INT(0, data_update(&DATA1, 0));

    // read should succeed
    block out;
    TEST_ASSERT_EQUAL_INT(0, data_read(&out, 0));

    // check contents
    TEST_ASSERT_EQUAL_MEMORY(&DATA1, &out, sizeof(block));
}

void test_data_read_invalid_inputs(void) {
    block out;
    // null ptr
    TEST_ASSERT_EQUAL_INT(-1, data_read(NULL, 0));
    // negative data block
    TEST_ASSERT_EQUAL_INT(-1, data_read(&out, -1));
    // data block out of bounds
    TEST_ASSERT_EQUAL_INT(-1, data_read(&out, NUM_DATA_BLOCKS));
}

/*
 * === DATA_UPDATE ===
 */

void test_data_update_updates_only_requested_block(void) {
    // allocations should succeed
    TEST_ASSERT_EQUAL_INT(0, data_alloc());
    TEST_ASSERT_EQUAL_INT(1, data_alloc());

    // writes should succeed
    TEST_ASSERT_EQUAL_INT(0, data_update(&DATA1, 0));
    TEST_ASSERT_EQUAL_INT(0, data_update(&DATA1, 1));

    // update data block 0
    TEST_ASSERT_EQUAL_INT(0, data_update(&DATA2, 0));

    // data block 1 should remain unchanged
    block out;
    TEST_ASSERT_EQUAL_INT(0, data_read(&out, 1));
    TEST_ASSERT_EQUAL_MEMORY(&DATA1, &out, sizeof(block));

    // data block 0 should be updated
    TEST_ASSERT_EQUAL_INT(0, data_read(&out, 0));
    TEST_ASSERT_EQUAL_MEMORY(&DATA2, &out, sizeof(block));
}

void test_data_update_invalid_inputs(void) {
    // null ptr
    TEST_ASSERT_EQUAL_INT(-1, data_update(NULL, 0));
    // negative data block
    TEST_ASSERT_EQUAL_INT(-1, data_update(&DATA1, -1));
    // data block out of bounds
    TEST_ASSERT_EQUAL_INT(-1, data_update(&DATA1, NUM_DATA_BLOCKS));
}
