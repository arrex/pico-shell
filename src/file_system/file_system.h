// file system specs
#define NUM_BLOCKS 64
#define BLOCK_SIZE 4096 // 4kB
#define NUM_INODES 80 // 16 inodes per block w 5 blocks
#define INODE_SIZE 256 // 256B
#define NUM_DIRECT_BLOCKS 5
#define FS_TYPE "my_ext" // our own extent-based fs type -- 7 bytes long

enum file_type {
    UNUSED_T,
    FILE_T,
    DIRECTORY_T
};

// file system components
typedef struct superblock {
    char file_system_type[sizeof(FS_TYPE)];
    int num_inodes;
    // stores which disk block marks the start of the inodes table
    int inodes_table_start;
    int num_data_blocks;
    int block_size; // in bytes
} superblock;

typedef struct inode {
    enum file_type file_type;
    int size;
    int blocks_occupied;
    int direct[NUM_DIRECT_BLOCKS];
    int indirect; // single indirect block

    // can add more metadata in the future (uid, perms, created_at,
    // updated_at, etc.)
} inode;

void fs_init();
