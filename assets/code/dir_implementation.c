#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_FILENAME 255

// traditional directory entry structure
struct dir_entry {
    unsigned long inode;       // Inode number
    unsigned short rec_len;    // Record length
    unsigned char name_len;    // Name length
    char name[MAX_FILENAME];   // Filename
    struct dir_entry* next;    // Pointer to next entry
};

struct directory {
    struct dir_entry* head;
    unsigned long size;
};

struct directory* init_directory() {
    struct directory* dir = malloc(sizeof(struct directory));
    dir->head = NULL;
    dir->size = 0;
    return dir;
}

void add_file(struct directory* dir, const char* filename, unsigned long inode) {
    struct dir_entry* entry = malloc(sizeof(struct dir_entry));
    entry->inode = inode;
    entry->name_len = strlen(filename);
    entry->rec_len = sizeof(struct dir_entry);
    strncpy(entry->name, filename, MAX_FILENAME);
    entry->next = dir->head;
    dir->head = entry;
    dir->size++;
}

struct dir_entry* find_file(struct directory* dir, const char* filename) {
    struct dir_entry* current = dir->head;
    while (current != NULL) {
        if (strcmp(current->name, filename) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

int main() {
    struct directory* test_dir = init_directory();

    add_file(test_dir, "file1.txt", 1001);
    add_file(test_dir, "file2.txt", 1002);
    add_file(test_dir, "file3.txt", 1003);

    const char* search_name = "file2.txt";
    struct dir_entry* result = find_file(test_dir, search_name);

    if (result != NULL) {
        printf("Found file: %s (inode: %lu)\n", result->name, result->inode);
    } else {
        printf("File not found: %s\n", search_name);
    }

    return 0;
}
