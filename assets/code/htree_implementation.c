#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define BLOCK_SIZE 4096
#define MAX_FILENAME 255
#define MAX_ENTRIES_PER_BLOCK ((BLOCK_SIZE - sizeof(struct block_header)) / sizeof(struct dir_entry))
#define MAX_INDEX_ENTRIES ((BLOCK_SIZE - sizeof(struct block_header)) / sizeof(struct index_entry))

uint32_t hash_filename(const char* name) {
    uint32_t hash = 0;
    while (*name) {
        hash = (hash << 5) + hash + *name;
        name++;
    }
    return hash;
}

struct block_header {
    uint32_t block_type;  // ROOT = 1, INDEX = 2, ENTRY = 3
    uint32_t entry_count;
    uint32_t free_space;
};

struct dir_entry {
    uint32_t inode;
    uint16_t rec_len;
    uint8_t name_len;
    uint8_t file_type;
    char name[MAX_FILENAME];
};

struct index_entry {
    uint32_t hash;
    uint32_t block_number;
};

struct htree_block {
    struct block_header header;
    union {
        struct dir_entry entries[MAX_ENTRIES_PER_BLOCK];
        struct index_entry indices[MAX_INDEX_ENTRIES];
    } data;
};

struct htree_directory {
    struct htree_block* root_block;
    struct htree_block** index_blocks;
    struct htree_block** entry_blocks;
    uint32_t num_index_blocks;
    uint32_t num_entry_blocks;
};

struct htree_directory* init_htree_directory() {
    struct htree_directory* dir = malloc(sizeof(struct htree_directory));

    dir->root_block = malloc(sizeof(struct htree_block));
    dir->root_block->header.block_type = 1;
    dir->root_block->header.entry_count = 0;
    dir->root_block->header.free_space = BLOCK_SIZE - sizeof(struct block_header);

    dir->index_blocks = NULL;
    dir->entry_blocks = NULL;
    dir->num_index_blocks = 0;
    dir->num_entry_blocks = 0;

    return dir;
}

struct htree_block* add_entry_block(struct htree_directory* dir) {
    dir->num_entry_blocks++;
    dir->entry_blocks = realloc(dir->entry_blocks,
                               dir->num_entry_blocks * sizeof(struct htree_block*));

    struct htree_block* block = malloc(sizeof(struct htree_block));
    block->header.block_type = 3;
    block->header.entry_count = 0;
    block->header.free_space = BLOCK_SIZE - sizeof(struct block_header);

    dir->entry_blocks[dir->num_entry_blocks - 1] = block;
    return block;
}

struct htree_block* find_entry_block(struct htree_directory* dir, uint32_t hash) {
    for (uint32_t i = 0; i < dir->num_entry_blocks; i++) {
        struct htree_block* block = dir->entry_blocks[i];
        if (block->header.entry_count < MAX_ENTRIES_PER_BLOCK) {
            return block;
        }
    }
    return add_entry_block(dir);
}

void insert_file(struct htree_directory* dir, const char* name, uint32_t inode) {
    uint32_t hash = hash_filename(name);
    struct htree_block* block = find_entry_block(dir, hash);

    struct dir_entry* entry = &block->data.entries[block->header.entry_count];
    entry->inode = inode;
    entry->name_len = strlen(name);
    entry->rec_len = sizeof(struct dir_entry);
    entry->file_type = 1; // Regular file
    strncpy(entry->name, name, MAX_FILENAME);

    block->header.entry_count++;
    block->header.free_space -= sizeof(struct dir_entry);
}

int main() {
    struct htree_directory* dir = init_htree_directory();

    insert_file(dir, "file1.txt", 1001);
    insert_file(dir, "file2.txt", 1002);
    insert_file(dir, "file3.txt", 1003);

    printf("Directory statistics:\n");
    printf("Number of entry blocks: %u\n", dir->num_entry_blocks);
    printf("First block entries: %u\n",
           dir->entry_blocks[0]->header.entry_count);

    return 0;
}
