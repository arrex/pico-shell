#include "../../src/fs/directory.h"

#include "../../src/fs/file.h"
#include "../../src/fs/inode.h"
#include "unity.h"

static struct dirent DIRENT = {
    .valid = true,
    .name = "test_entry",
    .inum = 1,
};

/* TODO: hook inode allocation and other setup into scaffold
 * might break other unit tests, so wait until fs init logic is
 * stable before
 *
 * the inode written will be allocated to slot 0 in the inode
 * table
 */
static void setup(void) {
    struct inode fixture = {
        .type = DIRECTORY_T,
        .addrs = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}};

    TEST_ASSERT_EQUAL_INT(0, inode_alloc(&fixture));
}

/*
 * === DIR_LOOKUP ===
 */

void test_dir_lookup_finds_entry(void) {
    setup();

    // adding entry should succeed
    TEST_ASSERT_EQUAL_INT(0, dir_add(0, &DIRENT));

    // lookup should succeed
    TEST_ASSERT_EQUAL_INT(DIRENT.inum, dir_lookup(0, DIRENT.name));
}

void test_dir_lookup_detects_missing(void) {
    setup();

    // lookup should fail
    TEST_ASSERT_EQUAL_INT(-1, dir_lookup(0, "does_not_exist"));
}

void test_dir_lookup_skips_invalid_entries(void) {
    setup();

    // adding entry should succeed
    TEST_ASSERT_EQUAL_INT(0, dir_add(0, &DIRENT));

    // lookup should succeed
    TEST_ASSERT_EQUAL_INT(DIRENT.inum, dir_lookup(0, DIRENT.name));

    // remove entry now
    TEST_ASSERT_EQUAL_INT(0, dir_remove(0, DIRENT.name));

    // lookup should fail
    TEST_ASSERT_EQUAL_INT(-1, dir_lookup(0, DIRENT.name));
}

void test_dir_lookup_invalid_inputs(void) {
    // write inode -- will get alloc'ed to slot 0
    struct inode file_inode = {
        .type = FILE_T,
    };
    TEST_ASSERT_EQUAL_INT(0, inode_alloc(&file_inode));

    // null ptr
    TEST_ASSERT_EQUAL_INT(-1, dir_lookup(0, NULL));
    // inum out of bounds
    TEST_ASSERT_EQUAL_INT(-1, dir_lookup(NUM_INODES, DIRENT.name));
    // not a directory
    TEST_ASSERT_EQUAL_INT(-1, dir_lookup(0, DIRENT.name));
}

/*
 * === DIR_EMPTY ===
 */

void test_dir_empty_returns_true_for_no_entries(void) {
    setup();

    // dir should be empty
    TEST_ASSERT_TRUE(dir_empty(0));
}

void test_dir_empty_returns_true_for_dot_entries(void) {
    setup();

    // adding entry should succeed
    struct dirent dot = {
        .valid = true,
        .name = ".",
        .inum = 0,
    };
    TEST_ASSERT_EQUAL_INT(0, dir_add(0, &dot));

    // dir should be empty
    TEST_ASSERT_TRUE(dir_empty(0));
}

void test_dir_empty_returns_false_for_dir_with_entry(void) {
    setup();

    // adding entry should succeed
    TEST_ASSERT_EQUAL_INT(0, dir_add(0, &DIRENT));

    // dir should not be empty
    TEST_ASSERT_FALSE(dir_empty(0));

    // remove entry
    TEST_ASSERT_EQUAL_INT(0, dir_remove(0, DIRENT.name));

    // dir should now be emtpy
    TEST_ASSERT_TRUE(dir_empty(0));
}

void test_dir_empty_invalid_inputs(void) {
    // write inode -- will get alloc'ed to slot 0
    struct inode file_inode = {
        .type = FILE_T,
    };
    TEST_ASSERT_EQUAL_INT(0, inode_alloc(&file_inode));

    // inum out of bounds
    TEST_ASSERT_EQUAL_INT(-1, dir_empty(NUM_INODES));
    // not a directory
    TEST_ASSERT_EQUAL_INT(-1, dir_empty(0));
}

/*
 * === DIR_ADD ===
 */

void test_dir_add_fails_when_full(void) {
    setup();

    for (int i = 0; i < MAX_FILE_SIZE / sizeof(dirent); i++) {
        struct dirent dirent = {
            .valid = true,
            .inum = 1,  // can be arbitrary, dir layer does enforce valid inum
                        // in dirent
        };
        snprintf(dirent.name, sizeof(dirent.name), "%d",
                 i);  // unique dirent names
        TEST_ASSERT_EQUAL_INT(0, dir_add(0, &dirent));
    }

    // directory data region full, adding another dirent should fail
    struct dirent extra = {
        .valid = 1,
        .name = "should_fail",
        .inum = 1,
    };
    TEST_ASSERT_EQUAL_INT(-1, dir_add(0, &extra));
}

void test_dir_add_fills_first_hole(void) {
    setup();

    // adding 3 entries should succeed
    for (int i = 0; i < 3; i++) {
        struct dirent dirent = {
            .valid = true,
            .inum = 1,
        };
        snprintf(dirent.name, sizeof(dirent.name), "%d", i);
        TEST_ASSERT_EQUAL_INT(0, dir_add(0, &dirent));
    }

    // remove dirent in the middle
    TEST_ASSERT_EQUAL_INT(0, dir_remove(0, "2"));

    // allocate new dirent, should fill hole in the middle
    TEST_ASSERT_EQUAL_INT(0, dir_add(0, &DIRENT));

    // inode size should be the same
    struct inode inode;
    TEST_ASSERT_EQUAL_INT(0, inode_read(&inode, 0));
    TEST_ASSERT_EQUAL_INT(sizeof(DIRENT) * 3, inode.size);

    // lookup on entry inserted in middle should succeed
    TEST_ASSERT_TRUE(dir_lookup(0, DIRENT.name));
}

void test_dir_add_detects_duplicates(void) {
    setup();

    TEST_ASSERT_EQUAL_INT(0, dir_add(0, &DIRENT));

    // adding duplicate dirent should fail
    TEST_ASSERT_EQUAL_INT(-1, dir_add(0, &DIRENT));
}

void test_dir_add_invalid_inputs(void) {
    // write inode -- will get alloc'ed to slot 0
    struct inode file_inode = {
        .type = FILE_T,
    };
    TEST_ASSERT_EQUAL_INT(0, inode_alloc(&file_inode));

    // null ptr
    TEST_ASSERT_EQUAL_INT(-1, dir_add(0, NULL));
    // inum out of bounds
    TEST_ASSERT_EQUAL_INT(-1, dir_add(NUM_INODES, &DIRENT));
    // not a directory
    TEST_ASSERT_EQUAL_INT(-1, dir_add(0, &DIRENT));
}

/*
 * === DIR_REMOVE ===
 */

void test_dir_remove_rejects_missing_entry(void) {
    setup();

    TEST_ASSERT_EQUAL_INT(-1, dir_remove(0, "does_not_exist"));
}

void test_dir_remove_ignores_removed_entries(void) {
    setup();

    // adding entry should succeed
    TEST_ASSERT_EQUAL_INT(0, dir_add(0, &DIRENT));

    // removing entry should succeed
    TEST_ASSERT_EQUAL_INT(0, dir_remove(0, DIRENT.name));

    // removing again should fail since dirent now invalid
    TEST_ASSERT_EQUAL_INT(-1, dir_remove(0, DIRENT.name));
}

void test_dir_remove_invalid_inputs(void) {
    // write inode -- will get alloc'ed to slot 0
    struct inode file_inode = {
        .type = FILE_T,
    };
    TEST_ASSERT_EQUAL_INT(0, inode_alloc(&file_inode));

    // null ptr
    TEST_ASSERT_EQUAL_INT(-1, dir_add(0, NULL));
    // inum out of bounds
    TEST_ASSERT_EQUAL_INT(-1, dir_add(NUM_INODES, &DIRENT));
    // not a directory
    TEST_ASSERT_EQUAL_INT(-1, dir_add(0, &DIRENT));
    // dot entries
    TEST_ASSERT_EQUAL_INT(-1, dir_remove(0, "."));
    TEST_ASSERT_EQUAL_INT(-1, dir_remove(0, ".."));
}
