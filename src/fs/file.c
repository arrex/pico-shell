#include "file.h"

#include <string.h>

#include "block.h"
#include "data.h"
#include "fs.h"
#include "inode.h"
#include "stdio.h"

#define min(a, b) ((a < b) ? (a) : (b))

int file_read(struct inode* inode, char* buf, int offset, int byte_count) {
    if (inode == NULL) {
        fprintf(stderr, "[file] error: inode pointer cannot be NULL\n");
        return -1;
    }

    if (buf == NULL) {
        fprintf(stderr, "[file] error: input buffer cannot be NULL\n");
        return -1;
    }

    if (offset > inode->size) {
        fprintf(stderr, "[file] error: invalid offset value %d\n", offset);
        return -1;
    }

    if (offset + byte_count < offset) {
        fprintf(stderr,
                "[file] error: offset and byte count will cause overflow\n");
        return -1;
    }

    if (offset + byte_count > MAX_FILE_SIZE) {
        fprintf(stderr,
                "[file] error: cannot read bytse chunk out of bounds\n");
        return -1;
    }

    uint chunk_size;
    for (int read = 0; read < byte_count;
         read += chunk_size, buf += chunk_size, offset += chunk_size) {
        // calculate which addr to access
        uint addr = offset / BLOCK_SIZE;
        uint dnum = inode_datamap(inode, addr);
        block block;
        if (data_read(&block, dnum) != 0) {
            return -1;
        }

        // calculate chunk size and read into buf
        chunk_size = min(byte_count - read, BLOCK_SIZE - offset % BLOCK_SIZE);
        memcpy(buf, block + offset % BLOCK_SIZE, chunk_size);
    }

    return byte_count;
}

/*
 * write data in provided buffer to inode. specify destination and dimension of
 * data we want to write via offset and byte_count respectively.
 *
 * returns number of bytes written in case of success, else -1.
 */
int file_write(struct inode* inode, char* buf, uint offset, uint byte_count) {
    if (inode == NULL) {
        fprintf(stderr, "[file] error: inode pointer cannot be NULL\n");
        return -1;
    }

    if (buf == NULL) {
        fprintf(stderr, "[file] error: input buffer cannot be NULL\n");
        return -1;
    }

    if (offset > inode->size) {
        fprintf(stderr, "[file] error: invalid offset value %d\n", offset);
        return -1;
    }

    if (offset + byte_count < offset) {
        fprintf(stderr,
                "[file] error: offset and byte count will cause overflow\n");
        return -1;
    }

    if (offset + byte_count > MAX_FILE_SIZE) {
        fprintf(stderr,
                "[file] error: byte chunk will be written out of bounds\n");
        return -1;
    }

    uint chunk_size;
    for (uint written = 0; written < byte_count;
         written += chunk_size, buf += chunk_size, offset += chunk_size) {
        // calculate which addr to access
        uint addr = offset / BLOCK_SIZE;
        uint dnum = inode_datamap(inode, addr);
        block block;
        if (data_read(&block, dnum) != 0) {
            return -1;
        }

        // calculate size of chunk we can right to curr block
        chunk_size =
            min(byte_count - written, BLOCK_SIZE - offset % BLOCK_SIZE);
        memcpy(&block + offset % BLOCK_SIZE, buf, chunk_size);

        // write block back
        if (data_update(&block, dnum) != 0) {
            return -1;
        }
    }

    // offset at this point represents the original offset + all the bytes we
    // wrote
    if (offset > inode->size) {
        inode->size = offset;
        if (inode_update(inode, inode->inum) != 0) {
            return -1;
        }
    }

    return byte_count;
}
