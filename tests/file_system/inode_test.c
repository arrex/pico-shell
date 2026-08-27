#include "../../src/file_system/inode.h"

#include "unity.h"

struct inode DUMMY_INODE1 = {.file_type = DIRECTORY_T,
                             .parent_inode = 0,
                             .size = 0,
                             .blocks_occupied = 1,
                             .extent_count = 1,
                             .extents = {{
                                 .logical_start = 0,
                                 .data_start = DATA_REGION_START,
                                 .block_count = 1,
                             }}};

struct inode DUMMY_INODE2 = {.file_type = FILE_T,
                             .parent_inode = 0,
                             .size = 0,
                             .blocks_occupied = 0,
                             .extent_count = 0,
                             .extents = {}};

/*
 * === INODE_ALLOC ===
 */

void test_inode_alloc_allocates_first_free_slot(void) {
    // check returned slot values
    TEST_ASSERT_EQUAL_INT(0, inode_alloc(&DUMMY_INODE1));
    TEST_ASSERT_EQUAL_INT(1, inode_alloc(&DUMMY_INODE1));

    // read inodes to check that they are non null
    struct inode out;
    TEST_ASSERT_EQUAL_INT(0, inode_read(&out, 0));
    TEST_ASSERT_EQUAL_MEMORY(&DUMMY_INODE1, &out, sizeof(struct inode));
    TEST_ASSERT_EQUAL_INT(0, inode_read(&out, 1));
    TEST_ASSERT_EQUAL_MEMORY(&DUMMY_INODE1, &out, sizeof(struct inode));
}

void test_inode_alloc_uses_lowest_available_slot(void) {
    // alloc several inodes
    TEST_ASSERT_EQUAL_INT(0, inode_alloc(&DUMMY_INODE1));
    TEST_ASSERT_EQUAL_INT(1, inode_alloc(&DUMMY_INODE1));
    TEST_ASSERT_EQUAL_INT(2, inode_alloc(&DUMMY_INODE1));

    // free one in the middle
    TEST_ASSERT_EQUAL_INT(1, inode_free(1));

    // allocate again, should fill the hole
    TEST_ASSERT_EQUAL_INT(1, inode_alloc(&DUMMY_INODE1));
}

void test_inode_alloc_preserves_existing_inode_entries(void) {
    // allocate 2 diff inodes
    TEST_ASSERT_EQUAL_INT(0, inode_alloc(&DUMMY_INODE1));
    TEST_ASSERT_EQUAL_INT(1, inode_alloc(&DUMMY_INODE2));

    // reads should succeed
    struct inode out;
    TEST_ASSERT_EQUAL_INT(0, inode_read(&out, 0));
    TEST_ASSERT_EQUAL_MEMORY(&DUMMY_INODE1, &out, sizeof(struct inode));
    TEST_ASSERT_EQUAL_INT(0, inode_read(&out, 1));
    TEST_ASSERT_EQUAL_MEMORY(&DUMMY_INODE2, &out, sizeof(struct inode));
}

void test_inode_alloc_crosses_inode_table_block_boundary(void) {
    // allocate inodes to fill an entire block and then one more
    for (int slot = 0; slot < (BLOCK_SIZE / INODE_SIZE) + 1; slot++) {
        TEST_ASSERT_EQUAL_INT(slot, inode_alloc(&DUMMY_INODE1));
    }

    // read inode at end boundary of first block and validate
    struct inode out;
    int last_ix = (BLOCK_SIZE / INODE_SIZE) - 1;
    TEST_ASSERT_EQUAL_INT(0, inode_read(&out, last_ix));
    TEST_ASSERT_EQUAL_MEMORY(&DUMMY_INODE1, &out, sizeof(struct inode));

    // read inode at starting boundary of second block and validate
    int first_ix = (BLOCK_SIZE / INODE_SIZE);
    TEST_ASSERT_EQUAL_INT(0, inode_read(&out, first_ix));
    TEST_ASSERT_EQUAL_MEMORY(&DUMMY_INODE1, &out, sizeof(struct inode));
}

void test_inode_alloc_returns_failure_when_inode_table_is_full(void) {
    // allocate to all slots
    for (int slot = 0; slot < NUM_INODES; slot++) {
        TEST_ASSERT_EQUAL_INT(slot, inode_alloc(&DUMMY_INODE1));
    }

    // next allocation should fail since table is full
    TEST_ASSERT_EQUAL_INT(-1, inode_alloc(&DUMMY_INODE2));
}

void test_inode_alloc_invalid_inputs(void) {
    // null ptr
    TEST_ASSERT_EQUAL_INT(-1, inode_alloc(NULL));
}

/*
 * === INODE_FREE
 */

void test_inode_free_marks_inode_available(void) {
    // allocate inode should succeed
    TEST_ASSERT_EQUAL_INT(0, inode_alloc(&DUMMY_INODE1));

    // free inode should succeed
    TEST_ASSERT_EQUAL_INT(0, inode_free(0));

    // next allocation should claim free slot
    TEST_ASSERT_EQUAL_INT(0, inode_alloc(&DUMMY_INODE2));
}

void test_inode_free_fails_when_slot_already_free(void) {
    TEST_ASSERT_EQUAL_INT(-1, inode_free(0));
}

void test_inode_free_invalid_inputs(void) {
    // negative slot number
    TEST_ASSERT_EQUAL_INT(-1, inode_free(-1));
    // out of bounds slot number
    TEST_ASSERT_EQUAL_INT(-1, inode_free(NUM_INODES));
}

/*
 * === INODE_READ ===
 */

void test_inode_read_reads_inode_at_requested_slot(void) {
    // allocate inode should succeed
    TEST_ASSERT_EQUAL_INT(0, inode_alloc(&DUMMY_INODE1));

    // read inode at that slot should succeed
    struct inode out;
    TEST_ASSERT_EQUAL_INT(0, inode_read(&out, 0));

    // check if contents are same
    TEST_ASSERT_EQUAL_MEMORY(&DUMMY_INODE1, &out, sizeof(struct inode));
}

void test_inode_read_invalid_inputs(void) {
    struct inode out;
    // null ptr
    TEST_ASSERT_EQUAL_INT(-1, inode_read(NULL, 0));
    // negative slot number
    TEST_ASSERT_EQUAL_INT(-1, inode_read(&out, -1));
    // out of bounds slot number
    TEST_ASSERT_EQUAL_INT(-1, inode_read(&out, NUM_INODES));
}

/*
 * === INODE_WRITE ===
 */

void test_inode_write_updates_only_requested_inode(void) {
    // allocate inode should succeed
    TEST_ASSERT_EQUAL_INT(0, inode_alloc(&DUMMY_INODE1));
    TEST_ASSERT_EQUAL_INT(1, inode_alloc(&DUMMY_INODE1));

    // update inode in slot 1
    TEST_ASSERT_EQUAL_INT(0, inode_write(&DUMMY_INODE2, 1));

    // first inode should remain unchanged
    struct inode out;
    TEST_ASSERT_EQUAL_INT(0, inode_read(&out, 0));
    TEST_ASSERT_EQUAL_MEMORY(&DUMMY_INODE1, &out, sizeof(struct inode));

    // check that second inode was properly updated
    TEST_ASSERT_EQUAL_INT(0, inode_read(&out, 1));
    TEST_ASSERT_EQUAL_MEMORY(&DUMMY_INODE2, &out, sizeof(struct inode));
}

void test_inode_write_invalid_inputs(void) {
    // null ptr
    TEST_ASSERT_EQUAL_INT(-1, inode_write(NULL, 0));
    // negative slot number
    TEST_ASSERT_EQUAL_INT(-1, inode_write(&DUMMY_INODE1, -1));
    // out of bounds slot number
    TEST_ASSERT_EQUAL_INT(-1, inode_write(&DUMMY_INODE1, NUM_INODES));
}
