//
// Created by Samarth Mahendra on 11/4/25.
//

#include "btree.h"
#include <stdlib.h>
#include <string.h>

static void write_node(BTree *tree, long offset, BTreeNode *node) {
    fseek(tree->fp, offset, SEEK_SET);
    fwrite(node, sizeof(BTreeNode), 1, tree->fp);
    fflush(tree->fp);
}

static void read_node(BTree *tree, long offset, BTreeNode *node) {
    fseek(tree->fp, offset, SEEK_SET);
    fread(node, sizeof(BTreeNode), 1, tree->fp);
}

static long allocate_page(BTree *tree) {
    fseek(tree->fp, 0, SEEK_END);
    long offset = ftell(tree->fp);
    BTreeNode blank = {0};
    fwrite(&blank, sizeof(BTreeNode), 1, tree->fp);
    fflush(tree->fp);
    return offset;
}

BTree *btree_open(const char *filename) {
    BTree *tree = malloc(sizeof(BTree));
    tree->fp = fopen(filename, "r+b");
    if (!tree->fp) tree->fp = fopen(filename, "w+b");

    fseek(tree->fp, 0, SEEK_END);
    if (ftell(tree->fp) == 0) {
        // new tree → create root
        BTreeNode root = {0};
        root.is_leaf = 1;
        tree->root_offset = allocate_page(tree);
        write_node(tree, tree->root_offset, &root);
    } else {
        // assume root at start
        tree->root_offset = 0;
    }
    return tree;
}

void btree_close(BTree *tree) {
    fclose(tree->fp);
    free(tree);
}

/* -------- Search -------- */
int btree_search(BTree *tree, long offset, const char *key, char *out_value) {
    BTreeNode node;
    read_node(tree, offset, &node);

    int i = 0;
    while (i < node.num_keys && strcmp(key, node.keys[i]) > 0)
        i++;

    if (node.is_leaf) {
        if (i < node.num_keys && strcmp(key, node.keys[i]) == 0) {
            strcpy(out_value, node.values[i]);
            return 1;
        }
        return 0;
    } else {
        long child = (i < node.num_keys && strcmp(key, node.keys[i]) == 0)
                         ? node.children[i + 1]
                         : node.children[i];
        return btree_search(tree, child, key, out_value);
    }
}

/* ---- Helper: split child ---- */
static void split_child(BTree *tree, BTreeNode *parent, int idx,
                        long child_offset) {
    BTreeNode child, new_child;
    read_node(tree, child_offset, &child);

    new_child.is_leaf = child.is_leaf;
    new_child.num_keys = ORDER / 2 - 1;

    // copy right half
    for (int j = 0; j < new_child.num_keys; j++) {
        strcpy(new_child.keys[j], child.keys[j + ORDER / 2]);
        strcpy(new_child.values[j], child.values[j + ORDER / 2]);
    }

    if (!child.is_leaf) {
        for (int j = 0; j < ORDER / 2; j++)
            new_child.children[j] = child.children[j + ORDER / 2];
    }

    child.num_keys = ORDER / 2 - 1;

    long new_offset = allocate_page(tree);
    write_node(tree, new_offset, &new_child);

    // shift parent keys/children
    for (int j = parent->num_keys; j > idx; j--) {
        strcpy(parent->keys[j], parent->keys[j - 1]);
        parent->children[j + 1] = parent->children[j];
    }

    // promote middle key
    strcpy(parent->keys[idx], child.keys[ORDER / 2 - 1]);
    parent->children[idx + 1] = new_offset;
    parent->num_keys++;

    write_node(tree, child_offset, &child);
}

/* ---- Helper: insert non-full ---- */
static void insert_nonfull(BTree *tree, long offset, const char *key,
                           const char *value) {
    BTreeNode node;
    read_node(tree, offset, &node);

    int i = node.num_keys - 1;

    if (node.is_leaf) {
        // shift larger keys
        while (i >= 0 && strcmp(key, node.keys[i]) < 0) {
            strcpy(node.keys[i + 1], node.keys[i]);
            strcpy(node.values[i + 1], node.values[i]);
            i--;
        }
        strcpy(node.keys[i + 1], key);
        strcpy(node.values[i + 1], value);
        node.num_keys++;
        write_node(tree, offset, &node);
    } else {
        // find child
        while (i >= 0 && strcmp(key, node.keys[i]) < 0)
            i--;
        i++;
        BTreeNode child;
        read_node(tree, node.children[i], &child);

        if (child.num_keys == ORDER - 1) {
            split_child(tree, &node, i, node.children[i]);
            write_node(tree, offset, &node);
            if (strcmp(key, node.keys[i]) > 0)
                i++;
        }
        insert_nonfull(tree, node.children[i], key, value);
    }
}

/* -------- Insert -------- */
int btree_insert(BTree *tree, const char *key, const char *value) {
    BTreeNode root;
    read_node(tree, tree->root_offset, &root);

    if (root.num_keys == ORDER - 1) {
        BTreeNode new_root = {0};
        new_root.is_leaf = 0;
        new_root.children[0] = tree->root_offset;

        long new_root_offset = allocate_page(tree);
        split_child(tree, &new_root, 0, tree->root_offset);

        tree->root_offset = new_root_offset;
        write_node(tree, new_root_offset, &new_root);
        insert_nonfull(tree, new_root_offset, key, value);
    } else {
        insert_nonfull(tree, tree->root_offset, key, value);
    }
    return 0;
}

