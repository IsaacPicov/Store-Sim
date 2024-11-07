#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include "wackman.c"

#define BITS_PER_INT (sizeof(int) * CHAR_BIT)
#define PRINT_TREE_SPACING 10
#define R_ERROR 0.0001

void print_list(WackyLinkedNode* head) {
    WackyLinkedNode* current = head;

    while (current != NULL) {
        printf("(%c) %f\n", current->val->val, current->val->weight);
        current = current->next;
    }
}

void print_wacky_tree_helper(WackyTreeNode* node, int space) {
    //printf("%f\n", node->weight);
    if (node == NULL) {
        return;
    }

    print_wacky_tree_helper(node->right, space + TREE_SPACING);

    for (int i = 0; i < space; i++) {
        printf(" ");
    }

    if (node->val != '\0') {
        printf("%.2f (%c)\n", node->weight, node->val);
    } else {
        printf("%.2f\n", node->weight);
    }

    print_wacky_tree_helper(node->left, space + TREE_SPACING);
}

/**
 * Given the root of a WackyTree, this function prints the tree in a horizontal
 * 2D format to the standard output (stdout).
 *
 * @param node Pointer to the root of the WackyTree.
 */
void print_wacky_tree(WackyTreeNode* root) { print_wacky_tree_helper(root, 0); }

int main() {
    char string[4096] = "thomas kielstra taking W's on assigments as always";

    printf("Testing compute_occurrence_array\n");
    int occurrence_array[ASCII_CHARACTER_SET_SIZE];
    compute_occurrence_array(occurrence_array, string);

    assert(occurrence_array[32] == 7);
    assert(occurrence_array[39] == 1);
    assert(occurrence_array[87] == 1);
    assert(occurrence_array[97] == 7);
    assert(occurrence_array[101] == 2);
    assert(occurrence_array[103] == 2);
    assert(occurrence_array[104] == 1);
    assert(occurrence_array[105] == 3);
    assert(occurrence_array[107] == 2);
    assert(occurrence_array[108] == 2);
    assert(occurrence_array[109] == 2);
    assert(occurrence_array[110] == 3);
    assert(occurrence_array[111] == 2);
    assert(occurrence_array[114] == 1);
    assert(occurrence_array[115] == 8);
    assert(occurrence_array[116] == 4);
    assert(occurrence_array[119] == 1);
    assert(occurrence_array[121] == 1);

    printf("Testing sum_array_elements\n");
    int sum_of_array = sum_array_elements(occurrence_array, ASCII_CHARACTER_SET_SIZE);
    assert(sum_of_array == 50);

    printf("Testing count_positive_occurences\n");
    int num_letters = count_positive_occurrences(occurrence_array);
    assert(num_letters == 18);

    printf("Testing create_wacky_list\n");
    WackyLinkedNode* listHead = create_wacky_list(occurrence_array);

    assert(listHead->val->val == '\'');
    assert(fabs(listHead->val->weight - 0.02) < R_ERROR);
    assert(listHead->next->next->val->val == 'h');
    assert(fabs(listHead->next->next->val->weight - 0.02) < R_ERROR);

    WackyLinkedNode* secondLast = listHead;

    while (secondLast->next->next != NULL) {
        secondLast = secondLast->next;
    }

    assert(secondLast->val->val == 'a');
    assert(secondLast->val->weight == 0.14);
    assert(secondLast->next->val->val == 's');
    assert(secondLast->next->val->weight == 0.16);
    assert(secondLast->next->next == NULL);

    // print_list(listHead);

    printf("Testing merge_wacky_list\n");
    // Valid linked list is given
    WackyTreeNode* tree_root = merge_wacky_list(listHead);

    assert(fabs(tree_root->left->weight - 0.4) < R_ERROR);
    assert(fabs(tree_root->left->right->weight - 0.24) < R_ERROR);
    assert(fabs(tree_root->right->weight - 0.6) < R_ERROR);
    assert(fabs(tree_root->right->right->weight - 0.32) < R_ERROR);
    assert(fabs(tree_root->right->right->left->weight - 0.16) < R_ERROR);
    assert(fabs(tree_root->right->right->right->weight - 0.16) < R_ERROR);

    assert(tree_root->left->left->val == 's');
    assert(tree_root->left->right->left->left->val == 'o');
    assert(tree_root->left->right->left->right->val == 'i');
    assert(tree_root->left->right->right->left->val == 'n');
    assert(tree_root->left->right->right->right->left->val == 'l');
    assert(tree_root->left->right->right->right->right->val == 'm');
    assert(tree_root->right->left->left->val == ' ');
    assert(tree_root->right->left->right->val == 'a');
    assert(tree_root->right->right->left->right->val == 't');
    assert(tree_root->right->right->left->left->left->left->val == 'w');
    assert(tree_root->right->right->left->left->left->right->val == 'y');
    assert(tree_root->right->right->left->left->right->left->val == 'h');
    assert(tree_root->right->right->left->left->right->right->val == 'r');
    assert(tree_root->right->right->right->left->left->val == 'g');
    assert(tree_root->right->right->right->left->right->val == 'k');
    assert(tree_root->right->right->right->right->left->left->val == '\'');
    assert(tree_root->right->right->right->right->left->right->val == 'W');
    assert(tree_root->right->right->right->right->right->val == 'e');

    print_wacky_tree(tree_root);
    // When linked list is NULL
    WackyTreeNode* tree_test_null = merge_wacky_list(NULL);
    assert(tree_test_null == NULL);

    printf("Testing get_height\n");
    // Valid tree is given
    int height = get_height(tree_root);
    assert(height == 7);

    // When tree is NULL
    int height_null = get_height(NULL);
    assert(height_null == 0);

    printf("Testing get_wacky_code\n");
    // Finding something in the tree
    int array_size;
    bool path[height];
    get_wacky_code(tree_root, 'r', path, &array_size);

    assert(array_size == 6);
    assert(path[0] == true);
    assert(path[1] == true);
    assert(path[2] == false);
    assert(path[3] == false);
    assert(path[4] == true);
    assert(path[5] == true);

    // Finding something not in the tree
    int array_size2;
    bool path2[height];
    get_wacky_code(tree_root, '!', path2, &array_size2);

    assert(array_size2 == -1);

    // When tree is NULL
    int array_size3;
    get_wacky_code(NULL, 'e', path2, &array_size3);

    assert(array_size3 == -1);

    printf("Testing get_character\n");
    // Testing valid path is given
    bool path3[height];
    path3[0] = false;
    path3[1] = true;
    path3[2] = true;
    path3[3] = false;
    char char1 = get_character(tree_root, path3, 4);

    assert(char1 == 'n');

    bool path4[height];
    path4[0] = true;
    path4[1] = true;
    path4[2] = false;
    path4[3] = false;
    path4[4] = false;
    path4[5] = true;
    char char2 = get_character(tree_root, path4, 6);

    assert(char2 == 'y');

    bool path5[height];
    path5[0] = true;
    path5[1] = true;
    char char3 = get_character(tree_root, path5, 2);

    assert(char3 == '\0');

    // Testing when invalid path is given
    bool path6[height];
    path6[0] = false;
    path6[1] = false;
    path6[2] = false;
    path6[3] = true;
    char char4 = get_character(tree_root, path6, 4);

    assert(char4 == '\0');
    free_tree(tree_root);
    printf("All good!");

    return 0;
}