#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_FILENAME 255

struct btree_node {
    unsigned long inode;
    char name[MAX_FILENAME];
    struct btree_node* left;
    struct btree_node* right;
};

struct btree_directory {
    struct btree_node* root;
    unsigned long size;
};

struct btree_directory* init_btree_directory() {
    struct btree_directory* dir = malloc(sizeof(struct btree_directory));
    dir->root = NULL;
    dir->size = 0;
    return dir;
}

struct btree_node* create_node(const char* name, unsigned long inode) {
    struct btree_node* node = malloc(sizeof(struct btree_node));
    strncpy(node->name, name, MAX_FILENAME);
    node->inode = inode;
    node->left = NULL;
    node->right = NULL;
    return node;
}

struct btree_node* insert_file_recursive(struct btree_node* node, const char* name,
                                       unsigned long inode) {
    if (node == NULL) {
        return create_node(name, inode);
    }

    int cmp = strcmp(name, node->name);
    if (cmp < 0) {
        node->left = insert_file_recursive(node->left, name, inode);
    } else if (cmp > 0) {
        node->right = insert_file_recursive(node->right, name, inode);
    }

    return node;
}

void insert_file(struct btree_directory* dir, const char* name, unsigned long inode) {
    dir->root = insert_file_recursive(dir->root, name, inode);
    dir->size++;
}

struct btree_node* find_file(struct btree_directory* dir, const char* name) {
    struct btree_node* current = dir->root;

    while (current != NULL) {
        int cmp = strcmp(name, current->name);
        if (cmp == 0) {
            return current;
        } else if (cmp < 0) {
            current = current->left;
        } else {
            current = current->right;
        }
    }

    return NULL;
}

int main() {
    struct btree_directory* dir = init_btree_directory();

    insert_file(dir, "file1.txt", 1001);
    insert_file(dir, "file2.txt", 1002);
    insert_file(dir, "file3.txt", 1003);

    const char* search_name = "file2.txt";
    struct btree_node* result = find_file(dir, search_name);

    if (result != NULL) {
        printf("Found file: %s (inode: %lu)\n", result->name, result->inode);
    } else {
        printf("File not found: %s\n", search_name);
    }

    return 0;
}
