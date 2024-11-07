#include <limits.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TREE_SPACING 10
#define ASCII_CHARACTER_SET_SIZE 128

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

int setBit(int n, int k) { return (n | (1 << k)); }

bool findBit(int n, int k) { return ((n >> k) & 1); }

typedef struct WackyTreeNode WackyTreeNode;
struct WackyTreeNode {
    double weight;
    char val;

    WackyTreeNode* left;
    WackyTreeNode* right;
};

typedef struct WackyLinkedNode WackyLinkedNode;
struct WackyLinkedNode {
    WackyTreeNode* val;
    WackyLinkedNode* next;
};

WackyTreeNode* new_leaf_node(double weight, char val) {
    WackyTreeNode* node = (WackyTreeNode*)malloc(sizeof(WackyTreeNode));
    node->weight = weight;
    node->val = val;
    node->left = NULL;
    node->right = NULL;
    return node;
}

WackyTreeNode* new_branch_node(WackyTreeNode* left, WackyTreeNode* right) {
    WackyTreeNode* node = (WackyTreeNode*)malloc(sizeof(WackyTreeNode));
    node->weight = left->weight + right->weight;
    node->val = '\0';
    node->left = left;
    node->right = right;
    return node;
}

WackyLinkedNode* new_linked_node(WackyTreeNode* val) {
    WackyLinkedNode* node = (WackyLinkedNode*)malloc(sizeof(WackyLinkedNode));
    node->val = val;
    node->next = NULL;
    return node;
}
