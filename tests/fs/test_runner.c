#include "../../src/fs/disk.h"
#include "block_test.h"
#include "data_test.h"
#include "directory_test.h"
#include "disk_test.h"
#include "file_test.h"
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
    RUN_TEST(test_inode_alloc_can_allocate_all_slots);
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
    RUN_TEST(test_inode_update);
    RUN_TEST(test_inode_update_invalid_inputs);

    // data layer tests
    RUN_TEST(test_data_alloc_allocates_first_free_slot);
    RUN_TEST(test_data_alloc_uses_lowest_available_slot);
    RUN_TEST(test_data_alloc_can_fill_data_region);
    RUN_TEST(test_data_alloc_fails_when_data_region_full);
    RUN_TEST(test_data_free_marks_block_available);
    RUN_TEST(test_data_free_fails_when_block_already_free);
    RUN_TEST(test_data_free_invalid_inputs);
    RUN_TEST(test_data_read_reads_block_at_requested_slot);
    RUN_TEST(test_data_read_invalid_inputs);
    RUN_TEST(test_data_update_updates_only_requested_block);
    RUN_TEST(test_data_update_invalid_inputs);

    // file layer tests
    RUN_TEST(test_file_write_with_and_without_offset);
    RUN_TEST(test_file_write_can_write_to_entire_address_space);
    RUN_TEST(test_file_write_with_zero_byte_write);
    RUN_TEST(test_file_write_invalid_inputs);
    RUN_TEST(test_file_read_round_trip);
    RUN_TEST(test_file_read_can_read_from_entire_address_space);
    RUN_TEST(test_file_read_with_zero_byte_read);
    RUN_TEST(test_file_read_invalid_inputs);

    // directory layer tests
    RUN_TEST(test_dir_lookup_finds_entry);
    RUN_TEST(test_dir_lookup_detects_missing);
    RUN_TEST(test_dir_lookup_skips_invalid_entries);
    RUN_TEST(test_dir_lookup_invalid_inputs);
    RUN_TEST(test_dir_empty_returns_true_for_no_entries);
    RUN_TEST(test_dir_empty_returns_true_for_dot_entries);
    RUN_TEST(test_dir_empty_returns_false_for_dir_with_entry);
    RUN_TEST(test_dir_add_fails_when_full);
    RUN_TEST(test_dir_add_fills_first_hole);
    RUN_TEST(test_dir_add_detects_duplicates);
    RUN_TEST(test_dir_add_invalid_inputs);
    RUN_TEST(test_dir_remove_rejects_missing_entry);
    RUN_TEST(test_dir_remove_ignores_removed_entries);
    RUN_TEST(test_dir_remove_invalid_inputs);

    return UNITY_END();
}
