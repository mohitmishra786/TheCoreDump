#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <stdint.h>

#define BLOCK_SIZE 4096
#define MAX_FILENAME 255
#define NUM_FILES 10000
#define FILENAME_LENGTH 20
#define MAX_ENTRIES_PER_BLOCK ((BLOCK_SIZE - sizeof(struct block_header)) / sizeof(struct dir_entry))
#define MAX_INDEX_ENTRIES ((BLOCK_SIZE - sizeof(struct block_header)) / sizeof(struct index_entry))

// Forward declarations
struct htree_directory;
struct htree_block;
struct dir_entry;

// Structures for H-tree implementation
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

// Benchmark results structure
struct benchmark_results {
    double insertion_time;
    double search_time;
    double memory_usage;
};

// Hash function for filenames
uint32_t hash_filename(const char* name) {
    uint32_t hash = 0;
    while (*name) {
        hash = (hash << 5) + hash + *name;
        name++;
    }
    return hash;
}

// Initialize H-tree directory
struct htree_directory* init_htree_directory() {
    struct htree_directory* dir = malloc(sizeof(struct htree_directory));
    if (!dir) return NULL;

    // Initialize root block
    dir->root_block = malloc(sizeof(struct htree_block));
    if (!dir->root_block) {
        free(dir);
        return NULL;
    }

    dir->root_block->header.block_type = 1;
    dir->root_block->header.entry_count = 0;
    dir->root_block->header.free_space = BLOCK_SIZE - sizeof(struct block_header);

    dir->index_blocks = NULL;
    dir->entry_blocks = NULL;
    dir->num_index_blocks = 0;
    dir->num_entry_blocks = 0;

    return dir;
}

// Add a new entry block
struct htree_block* add_entry_block(struct htree_directory* dir) {
    dir->num_entry_blocks++;
    dir->entry_blocks = realloc(dir->entry_blocks,
                               dir->num_entry_blocks * sizeof(struct htree_block*));
    if (!dir->entry_blocks) return NULL;

    struct htree_block* block = malloc(sizeof(struct htree_block));
    if (!block) {
        dir->num_entry_blocks--;
        return NULL;
    }

    block->header.block_type = 3;
    block->header.entry_count = 0;
    block->header.free_space = BLOCK_SIZE - sizeof(struct block_header);

    dir->entry_blocks[dir->num_entry_blocks - 1] = block;
    return block;
}

// Find appropriate entry block for a hash value
struct htree_block* find_entry_block(struct htree_directory* dir, uint32_t hash) {
    for (uint32_t i = 0; i < dir->num_entry_blocks; i++) {
        struct htree_block* block = dir->entry_blocks[i];
        if (block->header.entry_count < MAX_ENTRIES_PER_BLOCK) {
            return block;
        }
    }
    return add_entry_block(dir);
}

// Insert a file into the H-tree directory
void insert_file(struct htree_directory* dir, const char* name, uint32_t inode) {
    uint32_t hash = hash_filename(name);
    struct htree_block* block = find_entry_block(dir, hash);
    if (!block) return;

    struct dir_entry* entry = &block->data.entries[block->header.entry_count];
    entry->inode = inode;
    entry->name_len = strlen(name);
    entry->rec_len = sizeof(struct dir_entry);
    entry->file_type = 1; // Regular file
    strncpy(entry->name, name, MAX_FILENAME);

    block->header.entry_count++;
    block->header.free_space -= sizeof(struct dir_entry);
}

// Search for a file in the H-tree directory
struct dir_entry* find_file(struct htree_directory* dir, const char* name) {
    uint32_t hash = hash_filename(name);

    for (uint32_t i = 0; i < dir->num_entry_blocks; i++) {
        struct htree_block* block = dir->entry_blocks[i];
        for (uint32_t j = 0; j < block->header.entry_count; j++) {
            struct dir_entry* entry = &block->data.entries[j];
            if (strcmp(entry->name, name) == 0) {
                return entry;
            }
        }
    }
    return NULL;
}

// Timing utility function
double get_time() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec * 1e-6;
}

// Generate random filename
void generate_filename(char* buffer) {
    static const char charset[] = "abcdefghijklmnopqrstuvwxyz0123456789";
    for (int i = 0; i < FILENAME_LENGTH - 4; i++) {
        buffer[i] = charset[rand() % (sizeof(charset) - 1)];
    }
    strcpy(buffer + FILENAME_LENGTH - 4, ".txt");
    buffer[FILENAME_LENGTH] = '\0';
}

// Cleanup H-tree directory
void cleanup_htree_directory(struct htree_directory* dir) {
    if (!dir) return;

    if (dir->root_block) {
        free(dir->root_block);
    }

    if (dir->index_blocks) {
        for (uint32_t i = 0; i < dir->num_index_blocks; i++) {
            free(dir->index_blocks[i]);
        }
        free(dir->index_blocks);
    }

    if (dir->entry_blocks) {
        for (uint32_t i = 0; i < dir->num_entry_blocks; i++) {
            free(dir->entry_blocks[i]);
        }
        free(dir->entry_blocks);
    }

    free(dir);
}

// Run benchmark tests
struct benchmark_results run_htree_benchmark() {
    struct benchmark_results results = {0};
    struct htree_directory* dir = init_htree_directory();
    if (!dir) {
        printf("Failed to initialize H-tree directory\n");
        return results;
    }

    char** filenames = malloc(NUM_FILES * sizeof(char*));
    if (!filenames) {
        cleanup_htree_directory(dir);
        printf("Failed to allocate memory for filenames\n");
        return results;
    }

    // Generate filenames
    for (int i = 0; i < NUM_FILES; i++) {
        filenames[i] = malloc(FILENAME_LENGTH + 1);
        if (!filenames[i]) {
            for (int j = 0; j < i; j++) {
                free(filenames[j]);
            }
            free(filenames);
            cleanup_htree_directory(dir);
            printf("Failed to allocate memory for filename %d\n", i);
            return results;
        }
        generate_filename(filenames[i]);
    }

    // Measure insertion time
    double start_time = get_time();
    for (int i = 0; i < NUM_FILES; i++) {
        insert_file(dir, filenames[i], i + 1000);
    }
    results.insertion_time = get_time() - start_time;

    // Measure search time (random access)
    start_time = get_time();
    for (int i = 0; i < 1000; i++) {
        int index = rand() % NUM_FILES;
        find_file(dir, filenames[index]);
    }
    results.search_time = get_time() - start_time;

    // Calculate memory usage
    results.memory_usage = sizeof(struct htree_directory) +
                          sizeof(struct htree_block) * (1 + dir->num_index_blocks + dir->num_entry_blocks);

    // Cleanup
    for (int i = 0; i < NUM_FILES; i++) {
        free(filenames[i]);
    }
    free(filenames);
    cleanup_htree_directory(dir);

    return results;
}

int main() {
    srand(time(NULL));

    printf("Running H-tree performance benchmark...\n");
    printf("Configuration:\n");
    printf("- Number of files: %d\n", NUM_FILES);
    printf("- Block size: %d bytes\n", BLOCK_SIZE);
    printf("- Max entries per block: %lu\n", MAX_ENTRIES_PER_BLOCK);

    struct benchmark_results results = run_htree_benchmark();

    printf("\nBenchmark Results:\n");
    printf("Insertion time for %d files: %.3f seconds\n", NUM_FILES, results.insertion_time);
    printf("Average search time (1000 random lookups): %.6f seconds\n",
           results.search_time / 1000.0);
    printf("Memory usage: %.2f MB\n", results.memory_usage / (1024.0 * 1024.0));
    printf("Average insertion time per file: %.6f ms\n",
           (results.insertion_time * 1000.0) / NUM_FILES);

    return 0;
}
