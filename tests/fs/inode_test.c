#include "../../src/fs/inode.h"

#include "unity.h"

struct inode INODE1 = {
    .type = DIRECTORY_T,
};

struct inode INODE2 = {
    .type = FILE_T,
};

/*
 * === INODE_ALLOC ===
 */

void test_inode_alloc_can_allocate_all_slots(void) {
    for (int i = 0; i < NUM_INODES; i++) {
        TEST_ASSERT_EQUAL_INT(0, inode_alloc(&INODE1));
    }

    // read inodes to check that they are non null
    struct inode out;
    for (int inum = 0; inum < NUM_INODES; inum++) {
        TEST_ASSERT_EQUAL_INT(0, inode_read(&out, inum));
    }
}

void test_inode_alloc_uses_lowest_available_slot(void) {
    // alloc several inodes
    TEST_ASSERT_EQUAL_INT(0, inode_alloc(&INODE1));
    TEST_ASSERT_EQUAL_INT(0, inode_alloc(&INODE1));
    TEST_ASSERT_EQUAL_INT(0, inode_alloc(&INODE1));

    // free one in the middle
    TEST_ASSERT_EQUAL_INT(0, inode_free(1));

    // allocate again, should fill the hole
    TEST_ASSERT_EQUAL_INT(0, inode_alloc(&INODE2));

    // read out to see if newly written inode filled lowest hole
    struct inode out;
    TEST_ASSERT_EQUAL_INT(0, inode_read(&out, 1));
    TEST_ASSERT_EQUAL_MEMORY(&INODE2, &out, sizeof(out));
}

void test_inode_alloc_preserves_existing_inode_entries(void) {
    // allocate 2 diff inodes
    TEST_ASSERT_EQUAL_INT(0, inode_alloc(&INODE1));
    TEST_ASSERT_EQUAL_INT(0, inode_alloc(&INODE2));

    // reads should succeed
    struct inode out;
    TEST_ASSERT_EQUAL_INT(0, inode_read(&out, 0));
    TEST_ASSERT_EQUAL_MEMORY(&INODE1, &out, sizeof(out));
    TEST_ASSERT_EQUAL_INT(0, inode_read(&out, 1));
    TEST_ASSERT_EQUAL_MEMORY(&INODE2, &out, sizeof(out));
}

void test_inode_alloc_crosses_inode_table_block_boundary(void) {
    // allocate inodes to fill first inode table block
    for (int i = 0; i < INODES_PER_BLOCK; i++) {
        TEST_ASSERT_EQUAL_INT(0, inode_alloc(&INODE1));
    }

    // allocate one more inode, this one will be the singular entry on 2nd inode
    // table block
    TEST_ASSERT_EQUAL_INT(0, inode_alloc(&INODE2));

    // read inode at end boundary of first block and validate
    struct inode out;
    int inum = INODES_PER_BLOCK - 1;
    TEST_ASSERT_EQUAL_INT(0, inode_read(&out, inum));
    TEST_ASSERT_EQUAL_MEMORY(&INODE1, &out, sizeof(out));

    // read inode at starting boundary of second block and validate
    TEST_ASSERT_EQUAL_INT(0, inode_read(&out, inum + 1));
    TEST_ASSERT_EQUAL_MEMORY(&INODE2, &out, sizeof(out));
}

void test_inode_alloc_returns_failure_when_inode_table_is_full(void) {
    // allocate to all slots
    for (int i = 0; i < NUM_INODES; i++) {
        TEST_ASSERT_EQUAL_INT(0, inode_alloc(&INODE1));
    }

    // next allocation should fail since table is full
    TEST_ASSERT_EQUAL_INT(-1, inode_alloc(&INODE2));
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
    TEST_ASSERT_EQUAL_INT(0, inode_alloc(&INODE1));

    // free inode should succeed
    TEST_ASSERT_EQUAL_INT(0, inode_free(0));

    // next allocation should claim free slot
    TEST_ASSERT_EQUAL_INT(0, inode_alloc(&INODE2));
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
    TEST_ASSERT_EQUAL_INT(0, inode_alloc(&INODE1));

    // read inode at that slot should succeed
    struct inode out;
    TEST_ASSERT_EQUAL_INT(0, inode_read(&out, 0));

    // check if contents are same
    TEST_ASSERT_EQUAL_MEMORY(&INODE1, &out, sizeof(struct inode));
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
 * === INODE_UPDATE ===
 */

void test_inode_update(void) {
    // allocate inode should succeed -- alloc'ed to slot 0
    TEST_ASSERT_EQUAL_INT(0, inode_alloc(&INODE1));

    // read should match what we expect
    struct inode out;
    TEST_ASSERT_EQUAL_INT(0, inode_read(&out, 0));
    TEST_ASSERT_EQUAL_MEMORY(&INODE1, &out, sizeof(out));

    // update inode should succeed
    TEST_ASSERT_EQUAL_INT(0, inode_update(&INODE2, 0));

    // read should match newly updated
    TEST_ASSERT_EQUAL_INT(0, inode_read(&out, 0));
    TEST_ASSERT_EQUAL_MEMORY(&INODE2, &out, sizeof(out));
}

void test_inode_update_invalid_inputs(void) {
    // null ptr
    TEST_ASSERT_EQUAL_INT(-1, inode_update(NULL, 0));
    // negative slot number
    TEST_ASSERT_EQUAL_INT(-1, inode_update(&INODE1, -1));
    // out of bounds slot number
    TEST_ASSERT_EQUAL_INT(-1, inode_update(&INODE1, NUM_INODES));
}
