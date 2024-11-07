/**
 * Assignment 1 - Wackman Compression
 *
 * Please read the comments below carefully, they describe your task in detail.
 *
 * There are also additional notes and clarifications on Quercus.
 *
 * Any clarifications and corrections will be posted to Piazza so please keep an
 * eye on the forum!
 *
 * Unauthorized distribution or posting is strictly prohibited. You must seek
 * approval from course staff before uploading and sharing with others.
 */

/**
 * You may modify this file however you wish as it will not be submitted on
 * Quercus. Please ensure that your code still works and does not depend on this
 * file as the automarker will be using a different main().
 */

#include <assert.h>

#include "beanstalk.c"
#include "wackman.c"

#define BITS_PER_INT (sizeof(int) * CHAR_BIT)
#define PRINT_TREE_SPACING 10
#define R_ERROR 0.0001

/**
 * This function is a helper for print_wacky_tree() and is responsible for
 * printing a WackyTree at a given node with a specified amount of space
 * padding.
 *
 * @param node Pointer to the root of the WackyTree or a subtree.
 * @param space The amount of space padding to be applied before printing the
 * node.
 */
void print_wacky_tree_helper(WackyTreeNode* node, int space) {
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

/**
 * Given the root of a WackyTree and a string, this function returns the integer
 * array encoding of the string derived from the given WackyTree. If any
 * characters cannot be encoded, it returns NULL instead.
 *
 * @param tree Pointer to the root of the WackyTree.
 * @param string The input string to be encoded.
 *
 * @return A dynamically allocated integer array representing the encoding of
 * the input string, or NULL if any characters cannot be encoded.
 */
int* encode_string(WackyTreeNode* tree, char* string) {
    if (tree == NULL || string == NULL || string[0] == '\0') {
        return NULL;
    }

    // These variables store information regarding the write buffer.
    int int_buffer_size = 1;

    // We need a padding at the start, to store length.
    int* return_int_buffer = calloc(1 + int_buffer_size, sizeof(int));
    int* write_int_buffer = &return_int_buffer[1];

    // Compute the max padding required from tree's height.
    int height_of_tree = get_height(tree);
    bool* boolean_array = malloc(height_of_tree * sizeof(bool));
    int boolean_array_length = 0;

    // Read from the string, until end of line delimiter.
    int string_index = 0;
    int bit_index = 0;

    while (string[string_index] != '\0') {
        char character = string[string_index];

        // Get the coding for this specific character.
        get_wacky_code(tree, character, boolean_array, &boolean_array_length);
        if (boolean_array_length < 0) {
            printf("Could not find coding for '%c', aborting...\n", character);
            free(boolean_array);
            free(return_int_buffer);
            return NULL;
        }

        // Write the coding to the int buffer.
        for (int i = 0; i < boolean_array_length; i++) {
            int int_buffer_idx = bit_index / BITS_PER_INT;
            if (int_buffer_idx >= int_buffer_size) {
                // Current buffer not big enough. Expand by 2x.
                int old_buffer_size = int_buffer_size;
                int_buffer_size = old_buffer_size * 2;
                return_int_buffer = realloc(
                    return_int_buffer, (1 + int_buffer_size) * sizeof(int));
                write_int_buffer = &return_int_buffer[1];
                memset(&write_int_buffer[old_buffer_size], 0,
                       old_buffer_size * sizeof(int));
            }

            // Originally, all bits are FALSE.
            // If TRUE, set the i_th bit to TRUE.
            if (boolean_array[i]) {
                int next_val = write_int_buffer[int_buffer_idx];
                int int_bit_idx = (bit_index % BITS_PER_INT);
                next_val = setBit(next_val, int_bit_idx);
                write_int_buffer[int_buffer_idx] = next_val;
            }

            bit_index++;
        }

        string_index++;
    }

    // No need the allocated boolean_array anymore. Free it.
    free(boolean_array);
    printf("\n");

    // Recall that return_int_buffer has a padding. Set the string length.
    return_int_buffer[0] = string_index;
    return return_int_buffer;
}

/**
 * Given the root of a WackyTree and an array of encoded integers, this function
 * returns the string derived from the given WackyTree. If any characters cannot
 * be decoded, it returns NULL instead.
 *
 * @param tree Pointer to the root of the WackyTree.
 * @param ints The input integer array to be decoded.
 *
 * @return A dynamically allocated string representing the decoded string,
 *         or NULL if any characters cannot be decoded.
 */
char* decode_ints(WackyTreeNode* tree, int* ints) {
    if (tree == NULL || ints == NULL) {
        return NULL;
    }

    // The length of the string is at ints[0].
    // The actual read buffer starts at ints[1];
    int string_length = ints[0];
    int* read_buffer = &ints[1];

    // Allocate enough memory to store the length of the string and the
    // end of line DELIMITER.
    char* output = malloc((string_length + 1) * sizeof(char));

    WackyTreeNode* current = tree;
    int current_string_length = 0;

    // For a tree with a single leaf node, just stay in place.
    while (current->left == NULL && current->right == NULL &&
           current_string_length < string_length) {
        output[current_string_length] = current->val;
        current_string_length++;
    }

    // Traverse the tree, left and right, until string is saturated.
    for (int read_buffer_idx = 0; current_string_length < string_length;
         read_buffer_idx++) {
        int value = read_buffer[read_buffer_idx];
        for (int bit_idx = 0; (bit_idx < BITS_PER_INT) &&
                              (current_string_length < string_length);
             bit_idx++) {
            bool toggle = findBit(value, bit_idx);
            if (toggle) {
                current = current->right;
            } else {
                current = current->left;
            }

            if (current->left == NULL && current->right == NULL) {
                output[current_string_length] = current->val;
                current_string_length++;
                current = tree;
            }
        }
    }

    // Add the end of line DELIMITER.
    output[string_length] = '\0';
    return output;
}

/**
 * Given a WackyTreeNode, this function asserts that the actual value and weight
 * of the node match the expected values. If not, it may raise an assertion
 * failure.
 *
 * @param node Pointer to the WackyTreeNode to be asserted.
 * @param expected_val The expected value.
 * @param expected_weight The expected weight.
 */
void assert_tree_node(WackyTreeNode* node, char expected_val,
                      double expected_weight) {
    assert(expected_val == node->val);
    assert(fabs(expected_weight - node->weight) <= R_ERROR);
}

int main() {
    printf("of 12\n");
    int occurrence_array[ASCII_CHARACTER_SET_SIZE];

    //empty
    compute_occurrence_array(occurrence_array, "");
    sum_array_elements(occurrence_array, ASCII_CHARACTER_SET_SIZE);
    WackyLinkedNode* linked_list = create_wacky_list(occurrence_array);
    WackyTreeNode* binary_tree = merge_wacky_list(linked_list);
    print_wacky_tree(binary_tree);
    if(get_height(binary_tree) != 0){
        printf("get height fail empty\n");
        exit(1);
    }
    int size;
    bool arr[128];
    get_wacky_code(binary_tree, '\0', arr, &size);
    if(size != -1){
        printf("get wackey code fail empty\n");
        exit(1);
    }
    if(get_character(binary_tree, arr, size) != '\0'){
        printf("get char fail empty\n");
        exit(1);
    }
    free_tree(binary_tree);
    //size 1///////////////////////////
    compute_occurrence_array(occurrence_array, "b");
    if(sum_array_elements(occurrence_array, ASCII_CHARACTER_SET_SIZE) != 1){
        printf("sum one fail one\n");
        exit(1);
    }
    linked_list = create_wacky_list(occurrence_array);
    binary_tree = merge_wacky_list(linked_list);
    print_wacky_tree(binary_tree);
    if(get_height(binary_tree) != 1){
        printf("get height fail one\n");
        exit(1);
    }
    get_wacky_code(binary_tree, 'b', arr, &size);
    if(size != 0){
        printf("get wackey code fail one\n");
        exit(1);
    }
    if(get_character(binary_tree, arr, size) != 'b'){
        printf("get char fail one\n");
        exit(1);
    }
    free_tree(binary_tree);
    ////////////Hello
    compute_occurrence_array(occurrence_array, "Hello");
    linked_list = create_wacky_list(occurrence_array);
    binary_tree = merge_wacky_list(linked_list);
    print_wacky_tree(binary_tree);
    assert(get_height(binary_tree) == 4);
    free_tree(binary_tree);
    /////all 128 characters but last position has delim
    char allCHAR[128];
    int i = 0;
    for(; i< 127; i++){
        allCHAR[i] = (char)i+1;
    }
    allCHAR[i] = (char)0;
    compute_occurrence_array(occurrence_array, allCHAR);
    linked_list = create_wacky_list(occurrence_array);
    binary_tree = merge_wacky_list(linked_list);
    print_wacky_tree(binary_tree);
    free_tree(binary_tree);
////////////////////////////
    printf("pass EDGE 0\n");
    // plain_text contains a story about Jack and the Beanstalk
    char plain_text[4096];
    strcpy(plain_text, JACK_AND_THE_BEANSTALK);
    /**
     * Testing compute_occurrence_array ...
     * Compute the number of occurrences of each ASCII character inside
     * plain_text and store it inside an array.
     */

    compute_occurrence_array(occurrence_array, plain_text);

    if (occurrence_array['a'] != 231) {
        printf("The letter 'a' appears 231 times. Got %d instead.\n",
               occurrence_array['a']);
        exit(1);
    }
    printf("pass 1\n");

    if (occurrence_array['f'] != 60) {
        printf("The letter 'f' appears 60 times. Got %d instead.\n",
               occurrence_array['f']);
        exit(1);
    }

    printf("pass 2\n");

    /**
     * Testing sum_array_elements ...
     * You can validate this by pasting plain_text into a web counter like
     * https://www.charactercountonline.com/ and getting the resulting
     * characters count.
     */

    int sum_of_array =
        sum_array_elements(occurrence_array, ASCII_CHARACTER_SET_SIZE);
    if (sum_of_array != 3166) {
        printf(
            "The sum of all integers in the array is 3166. Got %d instead.\n",
            sum_of_array);
        exit(1);
    }
    printf("pass 3\n");

    /**
     * Testing count_positive_occurrences ...
     * This is the total number of ASCII characters being used.
     */

    int positive_occurrences = count_positive_occurrences(occurrence_array);
    if (positive_occurrences != 44) {
        printf(
            "There are a total of 44 ASCII characters being used. Got %d "
            "instead.\n",
            positive_occurrences);
        exit(1);
    }
    printf("pass 4\n");

    /**
     * Testing create_wacky_list ...
     */

    linked_list = create_wacky_list(occurrence_array);
/////print order
    WackyLinkedNode* temp = linked_list;
    char allCharacters[ASCII_CHARACTER_SET_SIZE];
    int j = 0;
    while (temp != NULL) {
        printf("%f, %c\n",temp->val->weight ,temp->val->val);
        allCharacters[j] = temp->val->val;
        temp = temp->next;
        j++;
    }
    allCharacters[j] = '\0';
////////////////////////////////////
    {  // The first element should be '?' with weight '~0.00032'.
        WackyTreeNode* tree_node = linked_list->val;
        if (tree_node->val != '?' ||
            fabs(tree_node->weight - 0.00032) >= R_ERROR) {
            printf(
                "The first character in the linked list should be '?' with "
                "probably ~0.00032. Got '%c' %.5f instead.\n",
                tree_node->val, tree_node->weight);
            exit(1);
        }
    }
    printf("pass 5\n");

    {  // The last element should be ' ' with weight '~0.19109'.
        WackyLinkedNode* last_node = linked_list;
        while (last_node->next != NULL) {
            last_node = last_node->next;
        }

        WackyTreeNode* tree_node = last_node->val;
        if (tree_node->val != ' ' ||
            fabs(tree_node->weight - 0.19109) >= R_ERROR) {
            printf(
                "The last character in the linked list should be ' ' with "
                "probably ~0.19109. Got '%c' %.5f instead.\n",
                tree_node->val, tree_node->weight);
            exit(1);
        }
    }
    printf("pass 6\n");

    /**
     * Testing merge_wacky_list ...
     */

    binary_tree = merge_wacky_list(linked_list);

    // Perform a reverse in-order traversal of the binary_tree to print the
    // layout of the tree.
    print_wacky_tree(binary_tree);
    printf("\n");
    printf("pass 6.1\n");
    // The entire binary tree should weight 1.00 (100%).
    assert_tree_node(binary_tree, '\0', 1.00);
    printf("pass 6.2\n");
    // Some additional tests to help validate your binary tree's structure.
    assert_tree_node(binary_tree->left->left, ' ', 0.19109);
    printf("pass 6.3\n");
    assert_tree_node(binary_tree->right->left->right->right, 'a', 0.07296);

    printf("pass 7\n");
    /**
     * Testing get_height ...
     */
    int height = get_height(binary_tree);
    if (height != 13) {
        printf("The tree has height 13. Got %d instead.\n", height);
        exit(1);
    }
    printf("pass 8\n");
    /**
     * Testing get_wacky_code ...
     */

    bool boolean_array[128];
    int array_size;

    {
        // As shown above, 'a' is found at right->left->right->right
        // This translates to [TRUE, FALSE, TRUE, TRUE] of size 4.
        get_wacky_code(binary_tree, 'a', boolean_array, &array_size);
        if (array_size != 4) {
            printf("Expected array_size = 4. Got %d instead.\n", array_size);
            exit(1);
        }

        if (!boolean_array[0] || boolean_array[1] || !boolean_array[2] ||
            !boolean_array[3]) {
            printf("Expected true false true true. Got ");
            for (int i = 0; i < array_size; i++) {
                if (boolean_array[i]) {
                    printf("true ");
                } else {
                    printf("false ");
                }
            }
            printf("instead.\n");
            exit(1);
        }
    }
    printf("pass 9\n");
    {
        // Characters that cannot be found should return -1 instead.
        get_wacky_code(binary_tree, '#', boolean_array, &array_size);
        if (array_size != -1) {
            printf("Expected array_size = -1. Got %d instead.\n", array_size);
            exit(1);
        }
    }
    printf("pass 10\n");
    {
        // Similarily, feeding [TRUE, FALSE, TRUE, TRUE] should return 'a'.
        boolean_array[0] = true;
        boolean_array[1] = false;
        boolean_array[2] = true;
        boolean_array[3] = true;
        char character = get_character(binary_tree, boolean_array, 4);
        if (character != 'a') {
            printf("Expected character 'a'. Got %c instead.\n", character);
            exit(1);
        }
    }
    printf("pass 11\n");
    // The message below can only be decompiled on a 32-bit runnable.
    // if (sizeof(int) != 4 || CHAR_BIT != 8 || sizeof(int*) != 4) {
    //     printf("You must compile the main() program in 32-bit to continue ...\n");
    //     exit(1);
    // }
    //////////monkey wrench :)/////////////////////////////////////////////
    printf("pass bad inputs\n ");

    bool code[] = {true,true, true, true, true, true,true, true, true, true, true, true, true, true};
    

    char character = get_character(binary_tree, code, 14);
    printf("1\n");
    assert(character == '\0');
    
    character = get_character(binary_tree, code, 100000000);
    printf("2\n");
    assert(character == '\0');
    
    character = get_character(binary_tree, code, 0);
    printf("3\n");
    assert(character == '\0');
    height = get_height(NULL);
    printf("4\n");
    assert(height == 0);
    free_tree(NULL);
    printf("done\n");
    

   
    bool newCode[128];
    get_wacky_code(NULL, 'h', newCode, &size);
    assert(size == -1);
    printf("good\n");
    get_wacky_code(NULL, '\0', newCode, &size);
    assert(size == -1);
    printf("good\n");
    get_wacky_code(binary_tree, '\0', newCode, &size);
    assert(size == 0);
    printf("good\n");
    

    for(int i = 0; i<ASCII_CHARACTER_SET_SIZE; i++){
        if(allCharacters[i] == '\0'){
            break;
        }
        get_wacky_code(binary_tree, allCharacters[i], newCode, &size);
        assert(get_character(binary_tree, newCode, size) == allCharacters[i]);
    }
    printf("godd\n");
    get_wacky_code(binary_tree, (char)134, newCode, &size);
    assert(size == -1);
    int newPostive_occurrence_array[ASCII_CHARACTER_SET_SIZE];
    printf("spend\n");
    compute_occurrence_array(newPostive_occurrence_array, NULL);
    printf("shawn\n");
    for(int i = 0; i<ASCII_CHARACTER_SET_SIZE; i++){
        assert(newPostive_occurrence_array[i] == 0);
    }

    

    /////////////////////////////////////////////////////////////
    // If everything works properly, you should now be able to decode the
    // following message!
    printf("pass 12\n");
    int encoded[8] = {45,          -79266821,  1814895092, 1834766313,
                      -2003211311, -229391379, -478575313, 235};
    char* string = decode_ints(binary_tree, encoded);
    printf("decode string = %s\n", string);

    free(string);
    free_tree(binary_tree);
    printf("end of testing\n");
    return 0;
}
