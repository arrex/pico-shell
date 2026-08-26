#include "../../src/file_system/disk.h"
#include "block_layer_test.h"
#include "disk_test.h"
#include "inode_test.h"
#include "unity_internals.h"

void setUp(void) { disk_reset(); }

void tearDown(void) {}

int main(void) {
    UNITY_BEGIN();

    // disk layer tests
    RUN_TEST(test_disk_starts_at_all_zeroes);
    RUN_TEST(test_read_write_round_trip);
    RUN_TEST(test_overwrite_existing_data);
    RUN_TEST(test_invalid_read_inputs_leave_disk_unchanged);
    RUN_TEST(test_invalid_write_inputs_leave_disk_unchanged);
    RUN_TEST(test_read_write_at_final_disk_byte);

    // block layer tests
    RUN_TEST(test_block_starts_as_zeroes);
    RUN_TEST(test_block_write_read_round_trip);
    RUN_TEST(test_block_write_only_changes_target_block);
    RUN_TEST(test_block_writes_are_independent);
    RUN_TEST(test_invalid_read_inputs_preserve_state);
    RUN_TEST(test_invalid_write_inputs_preserve_state);

    // inode layer tests
    RUN_TEST(test_inode_alloc_allocates_first_free_slot);
    RUN_TEST(test_inode_alloc_uses_lowest_available_slot);
    RUN_TEST(test_inode_alloc_preserves_existing_inode_entries);
    RUN_TEST(test_inode_alloc_crosses_inode_table_block_boundary);
    RUN_TEST(test_inode_alloc_returns_failure_when_inode_table_is_full);
    RUN_TEST(test_inode_alloc_invalid_inputs);
    RUN_TEST(test_inode_free_marks_inode_available);
    RUN_TEST(test_inode_free_fails_when_slot_already_free);
    RUN_TEST(test_inode_free_invalid_inputs);
    RUN_TEST(test_inode_read_reads_inode_at_requested_slot);
    RUN_TEST(test_inode_read_invalid_inputs);
    RUN_TEST(test_inode_write_updates_only_requested_inode);
    RUN_TEST(test_inode_write_invalid_inputs);

    return UNITY_END();
}
