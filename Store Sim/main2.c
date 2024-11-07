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





/*----------------------------------------------------------------------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------------------------------------------------------------------------
ANDREY's TESTS----------------------------------------------------------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------------------------------------------------------------------------------*/


void tests_sum_array_elements() {
	printf("\n   - testing sum_array_elements()..........");

	//T1
	if (sum_array_elements(NULL, 0) != 0) {
		printf("T1 failed\n");
		exit(1);
	}

	//T2
	if (sum_array_elements(NULL, -1) != 0) {
		printf("T2 failed\n");
        exit(1);
	}

	//T3
	if (sum_array_elements(NULL, 100) != 0) {
        printf("T3 failed\n");
        exit(1);
    }

	int arr[6] = {1, 6, -1, 5, 10000012, -10000011};
	//T4
	if (sum_array_elements(arr, -1) != 0) {
        printf("T4 failed\n");
        exit(1);
    }

	//T5
    
	if (sum_array_elements(arr, 0) != 0) {
        printf("T5 failed\n");
        exit(1);
    }

	//T6
    if (sum_array_elements(arr, 5) != 10000023) {
        printf("T6 failed\n");
        exit(1);
    }

	//T7
    if (sum_array_elements(arr, 6) != 12) {
        printf("T7 failed\n");
        exit(1);
    }

	printf("works.\n");
	return;
}


void tests_compute_occurrence_array() {
	printf("\n   - testing compute_occurrenc_array()..........");

    //T1
	compute_occurrence_array(NULL, NULL);
	//T2
	int occurr_arr[ASCII_CHARACTER_SET_SIZE];
	for (int i = 0; i < ASCII_CHARACTER_SET_SIZE; i++) occurr_arr[i] = 0;
	compute_occurrence_array(occurr_arr, "bombom");
	if (count_positive_occurrences(occurr_arr) != 3) {
		printf("T2 failed\n");
		exit(1);
	}
	//T3
	compute_occurrence_array(occurr_arr, "");
    if (count_positive_occurrences(occurr_arr) != 0) {
        printf("T3 failed\n");
        exit(1);
    }

	compute_occurrence_array(occurr_arr, "bomyam");
    //T4
	if (count_positive_occurrences(occurr_arr) != 5) {
        printf("T4 failed\n");
        exit(1);
    }

	//T5
    
	if (occurr_arr[109] != 2) {
		printf("T5 failed\n");
		exit(1);
	}

	compute_occurrence_array(occurr_arr, "1");
	//T6

	if (count_positive_occurrences(occurr_arr) != 1) {
		printf("T6 failed\n");
		exit(1);
	}

	printf("works.\n");
    return;
}


void tests_count_positive_occurrences() {
	printf("\n   - testing count_positive_occurrences()..........");

	//T1
	if (count_positive_occurrences(NULL) != 0) {
		printf("T1 failed\n");
		exit(1);
	}

	int occurr_arr[ASCII_CHARACTER_SET_SIZE];
    for (int i = 0; i < ASCII_CHARACTER_SET_SIZE; i++) occurr_arr[i] = 0;

	//T2
	compute_occurrence_array(occurr_arr, "");
	if (count_positive_occurrences(occurr_arr) != 0) {
        printf("T2 failed\n");
        exit(1);
    }

	//T3
    compute_occurrence_array(occurr_arr, "~");
    if (count_positive_occurrences(occurr_arr) != 1) {
        printf("T3 failed\n");
        exit(1);
    }


	//T4
    compute_occurrence_array(occurr_arr, "bombom");
    if (count_positive_occurrences(occurr_arr) != 3) {
        printf("T4 failed\n");
        exit(1);
    }

	printf("works.\n");
    return;
}

void tests_create_wacky_list() {
	printf("\n   - testing create_wacky_list()..........");

	WackyLinkedNode* test_node = NULL;
	//T1
	test_node = create_wacky_list(NULL);
	if (test_node != NULL) {
		printf("T1 failed\n");
		exit(1);
	}

	WackyTreeNode* tree = NULL;
	int occurr_arr[ASCII_CHARACTER_SET_SIZE];
	compute_occurrence_array(occurr_arr, "Q");
	//T2
	test_node = NULL;
	test_node = create_wacky_list(occurr_arr);
	if (test_node->val->weight != 1 || test_node->val->val != 'Q' || test_node->next != NULL) {
		printf("T2 failed\n");
		tree = merge_wacky_list(test_node);
		free_tree(tree);
		exit(1);
	}
	tree = merge_wacky_list(test_node);
	free_tree(tree);
	tree = NULL; test_node = NULL;


	compute_occurrence_array(occurr_arr, "!!!2222222222222222222222111");
	//T3
	test_node = NULL;
	test_node = create_wacky_list(occurr_arr);
	if (fabs(test_node->next->val->weight - 0.1071) >= R_ERROR || fabs(test_node->next->next->val->weight - 0.7857) >= R_ERROR
		|| test_node->val->weight != test_node->next->val->weight || test_node->val->val != '!') {
		printf("T3 failed\n");
		tree = merge_wacky_list(test_node);
	    free_tree(tree);
		exit(1);
	}
	tree = merge_wacky_list(test_node);
	free_tree(tree);
	tree = NULL; test_node = NULL;

	//this case is a difference in organization of the list due to tie b\t s & 1 leading to ascii comparison
	compute_occurrence_array(occurr_arr, "sss2222222222222222222222111");
    //T4
    test_node = NULL;
    test_node = create_wacky_list(occurr_arr);
    if (fabs(test_node->next->val->weight - 0.1071) >= R_ERROR || fabs(test_node->next->next->val->weight - 0.7857) >= R_ERROR
        || test_node->val->weight != test_node->next->val->weight || test_node->val->val != '1') {
        printf("T4 failed\n");
        tree = merge_wacky_list(test_node);
        free_tree(tree);
        exit(1);
    }
    tree = merge_wacky_list(test_node);
    free_tree(tree);
    tree = NULL; test_node = NULL;

	printf("works.\n");
    return;
}

void tests_merge_wacky_list() {
	printf("\n   - testing merge_wacky_list()..........");

	int occurr_arr[ASCII_CHARACTER_SET_SIZE];
	compute_occurrence_array(occurr_arr, "");
	WackyLinkedNode* ll = create_wacky_list(occurr_arr);
	WackyTreeNode* tree = merge_wacky_list(ll);
	//T1
	if (tree != NULL) {
		printf("T1 failed\n");
        free_tree(tree);
		exit(1);
	}
    free_tree(tree);
	tree = NULL;

	//T2
	compute_occurrence_array(occurr_arr, " ");
	ll = create_wacky_list(occurr_arr);
	tree = merge_wacky_list(ll);
	if (tree->val != ' ' || tree->weight != 1 || tree->left != NULL || tree->right != NULL) {
		printf("T2 failed\n");
		free_tree(tree);
		exit(1);
	}
	free_tree(tree);
	tree = NULL;

	//T3
	//note the occurrences of various chars in the string if analyzing the usefulness of this test case
	compute_occurrence_array(occurr_arr, "poop!!!!!! nnToronTo");
	ll = create_wacky_list(occurr_arr);
	tree = merge_wacky_list(ll);
	if (tree->weight != 1 || tree->val != '\0') {
		printf("T3.1 failed\n");
		free_tree(tree);
		exit(1);
	}
	bool bool_arr[999];
	int arr_len = 0;
	get_wacky_code(tree, '\0', bool_arr, &arr_len);
	if (get_character(tree, bool_arr, arr_len) != '\0') {
		printf("T3.2 failed\n");
        free_tree(tree);
        exit(1);
    }
	get_wacky_code(tree, ' ', bool_arr, &arr_len);
	if (get_character(tree, bool_arr, arr_len) != ' ') {
        printf("T3.3 failed\n");
        free_tree(tree);
        exit(1);
    }
	get_wacky_code(tree, 'r', bool_arr, &arr_len);
    if (get_character(tree, bool_arr, arr_len) != 'r') {
        printf("T3.4 failed\n");
        free_tree(tree);
        exit(1);
    }
	get_wacky_code(tree, 'p', bool_arr, &arr_len);
    if (get_character(tree, bool_arr, arr_len) != 'p') {
        printf("T3.5 failed\n");
        free_tree(tree);
        exit(1);
    }
	get_wacky_code(tree, 'T', bool_arr, &arr_len);
    if (get_character(tree, bool_arr, arr_len) != 'T') {
        printf("T3.6 failed\n");
        free_tree(tree);
        exit(1);
    }
	get_wacky_code(tree, 'n', bool_arr, &arr_len);
    if (get_character(tree, bool_arr, arr_len) != 'n') {
        printf("T3.7 failed\n");
        free_tree(tree);
        exit(1);
    }
	get_wacky_code(tree, 'o', bool_arr, &arr_len);
    if (get_character(tree, bool_arr, arr_len) != 'o') {
        printf("T3.8 failed\n");
        free_tree(tree);
        exit(1);
    }
	get_wacky_code(tree, '!', bool_arr, &arr_len);
    if (get_character(tree, bool_arr, arr_len) != '!') {
        printf("T3.9 failed\n");
        free_tree(tree);
        exit(1);
    }
	free_tree(tree);

	printf("works.\n");
    return;
}


void tests_get_height() {
	//note that the heights of our trees for this assignment are directly proportional to # of positive occurrences
	printf("\n   - testing get_height()..........");

	//T1
	if (get_height(NULL) != 0) {
		printf("T1 failed\n");
		exit(1);
	}

	//T2
	WackyTreeNode* first = new_leaf_node(0.012, 'p');
	if (get_height(first) != 1) {
		printf("T2 failed\n");
		free(first);
		exit(1);
	}
	free(first);
		first = NULL;

	//T3
	first = new_leaf_node(0.2, '2');
	WackyTreeNode* second = new_leaf_node(0.8, '8');
	WackyTreeNode* third = new_branch_node(first, second);
	if (get_height(third) != 2) {
		printf("T3 failed\n");
		free_tree(third);
		exit(1);
	}
	free_tree(third);
		first = NULL, second = NULL, third = NULL;

	//T4
	int occurr_arr[ASCII_CHARACTER_SET_SIZE];
	compute_occurrence_array(occurr_arr, "poop!!!!!! nnToronTo");
    WackyLinkedNode* ll = create_wacky_list(occurr_arr);
    third = merge_wacky_list(ll);
	if (get_height(third) != 5) {
		printf("T4 failed\n");
		free_tree(third);
		exit(1);
	}
	free_tree(third);

	printf("works.\n");
	return;
}


void tests_get_wacky_code() {
	printf("\n   - testing get_wacky_code()..........");

	//T1 - exception case
	get_wacky_code(NULL, '\0', NULL, NULL);
	bool bool_arr[999];
	bool_arr[0] = true;
	int arr_len = 0;
	int occurr_arr[ASCII_CHARACTER_SET_SIZE];
	compute_occurrence_array(occurr_arr, "poop!!!!!! nnToronTo");
    WackyLinkedNode* ll = create_wacky_list(occurr_arr);
    WackyTreeNode* tree = merge_wacky_list(ll);

	//T2 - exception case
	get_wacky_code(tree, 'p', NULL, &arr_len);
	if (bool_arr[0] == false || arr_len != -1) {
		printf("T2 failed\n");
		free_tree(tree);
		exit(1);
	}

	//T3 - large sample size of chars in tree
	get_wacky_code(tree, 'p', bool_arr, &arr_len);
	if (get_character(tree, bool_arr, arr_len) != 'p' || arr_len != 3) {
		printf("T3 failed\n");
		free_tree(tree);
		exit(1);
	}

	//T4 - if char not in tree
	bool* temp = bool_arr;
	get_wacky_code(tree, 'Q', bool_arr, &arr_len);
	if (bool_arr[0] != temp[0] || bool_arr[2] != temp[2] || arr_len != -1) {
		printf("T4 failed\n");
		free_tree(tree);
		exit(1);
	}

	//T5 - if char to find is root node
	temp = bool_arr;
	get_wacky_code(tree, '\0', bool_arr, &arr_len);
	if (bool_arr[0] != temp[0] || arr_len != 0) {
		printf("T5 failed\n");
		free_tree(tree);
		exit(1);
	}

	free_tree(tree);

	printf("works.\n");
    return;
}

void tests_get_character() {
	printf("\n   - testing get_character()..........");

	//T1
	if (get_character(NULL, NULL, 6) != '\0') {
		printf("T1 failed\n");
		exit(1);
	}

	bool bool_arr[999];
        bool_arr[0] = true;
    int arr_len = 0;
    int occurr_arr[ASCII_CHARACTER_SET_SIZE];
    compute_occurrence_array(occurr_arr, "poop!!!!!! nnToronTo");
    WackyLinkedNode* ll = create_wacky_list(occurr_arr);
    WackyTreeNode* tree = merge_wacky_list(ll);

	//T2 - char not in tree
	get_wacky_code(tree, 'Q', bool_arr, &arr_len);
	if (get_character(tree, bool_arr, arr_len) != '\0') {
		printf("T2 failed\n");
		free_tree(tree);
		exit(1);
	}

	//T3
	get_wacky_code(tree, 'T', bool_arr, &arr_len);
	if (get_character(tree, bool_arr, arr_len) != 'T') {
		printf("T3 failed\n");
		free_tree(tree);
		exit(1);
	}

	//T4 - redundant but whatever
	get_wacky_code(tree, ' ', bool_arr, &arr_len);
	if (get_character(tree, bool_arr, arr_len) != ' ') {
		printf("T4 failed\n");
		free_tree(tree);
		exit(1);
	}

	//T5 - redundant but whatever
    get_wacky_code(tree, '\0', bool_arr, &arr_len);
    if (get_character(tree, bool_arr, arr_len) != '\0') {
        printf("T5 failed\n");
        free_tree(tree);
        exit(1);
    }

	free_tree(tree);

	printf("works.\n");
    return;
}

void tests_free_tree() {
	printf("\n   - testing free_tree()..........");

	//T1
	free_tree(NULL);

    int occurr_arr[ASCII_CHARACTER_SET_SIZE];
	//T2
	compute_occurrence_array(occurr_arr, "1");
	WackyLinkedNode* ll = create_wacky_list(occurr_arr);
	WackyTreeNode* tree = merge_wacky_list(ll);
	free_tree(tree);

	//T3
	compute_occurrence_array(occurr_arr, "bolshaia golova c testom i c bytilkai !!!! blin");
	ll = create_wacky_list(occurr_arr);
	tree = merge_wacky_list(ll);
	free_tree(tree);

	printf("Now test for memory leaks. If none; works.\n");
    return;
}


/*----------------------------------------------------------------------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------------------------------------------------------------------------
END OF ANDREY's TESTS---------------------------------------------------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------------------------------------------------------------------------------*/





int main() {
	/*Extra test case, used to solve an error with merge_wacky_list() (sol'n ended up being a
	  ...missing line of code in insert_to_list_sorted() helper func)
	char quercus_ex[4096] = "Hello";
	printf("%s\n", quercus_ex);
	int occurr_arr[ASCII_CHARACTER_SET_SIZE];
	compute_occurrence_array(occurr_arr, quercus_ex);
	for (int i = 0; i < ASCII_CHARACTER_SET_SIZE; i++) printf("%d", occurr_arr[i]);
		printf("\n");
	WackyLinkedNode* ll = create_wacky_list(occurr_arr);
	printf("Length of LL is %d:\n", get_length_list(ll));
	WackyLinkedNode* temporary = ll;
	while (temporary != NULL) {
		printf("%c____", temporary->val->val);
		temporary = temporary->next;
	}
	printf("\n");
	WackyTreeNode* bi_tree = merge_wacky_list(ll);
	print_wacky_tree(bi_tree);
	assert_tree_node(bi_tree, '\0', 1.0);
	printf("-------------------------------\n");
	*/

    // plain_text contains a story about Jack and the Beanstalk
    char plain_text[4096];
    strcpy(plain_text, JACK_AND_THE_BEANSTALK);

    /**
     * Testing compute_occurrence_array ...
     * Compute the number of occurrences of each ASCII character inside
     * plain_text and store it inside an array.
     */

    int occurrence_array[ASCII_CHARACTER_SET_SIZE];
    compute_occurrence_array(occurrence_array, plain_text);

    if (occurrence_array['a'] != 231) {
        printf("The letter 'a' appears 231 times. Got %d instead.\n",
               occurrence_array['a']);
        exit(1);
    }

    if (occurrence_array['f'] != 60) {
        printf("The letter 'f' appears 60 times. Got %d instead.\n",
               occurrence_array['f']);
        exit(1);
    }

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

    /**
     * Testing create_wacky_list ...
     */

    WackyLinkedNode* linked_list = create_wacky_list(occurrence_array);

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

    /**
     * Testing merge_wacky_list ...
     */

    WackyTreeNode* binary_tree = merge_wacky_list(linked_list);

    // Perform a reverse in-order traversal of the binary_tree to print the
    // layout of the tree.
    print_wacky_tree(binary_tree);
    printf("\n");

    // The entire binary tree should weight 1.00 (100%).
    assert_tree_node(binary_tree, '\0', 1.00);

    // Some additional tests to help validate your binary tree's structure.
    assert_tree_node(binary_tree->left->left, ' ', 0.19109);
    assert_tree_node(binary_tree->right->left->right->right, 'a', 0.07296);

    /**
     * Testing get_height ...
     */
    int height = get_height(binary_tree);
    if (height != 13) {
        printf("The tree has height 13. Got %d instead.\n", height);
        exit(1);
    }

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

    {
        // Characters that cannot be found should return -1 instead.
        get_wacky_code(binary_tree, '#', boolean_array, &array_size);
        if (array_size != -1) {
            printf("Expected array_size = -1. Got %d instead.\n", array_size);
            exit(1);
        }
    }

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

    // If everything works properly, you should now be able to decode the
    // following message!
    int encoded[8] = {45,          -79266821,  1814895092, 1834766313,
                      -2003211311, -229391379, -478575313, 235};
    char* string = decode_ints(binary_tree, encoded);
    printf("decode string = %s\n", string);

    free(string);
    free_tree(binary_tree);





//Running Andrey's tests
	printf("\nRunning Andrey's tests:\n");
	tests_sum_array_elements();
	tests_compute_occurrence_array();
	tests_count_positive_occurrences();
	tests_create_wacky_list();
	tests_get_height();
	tests_merge_wacky_list();
	tests_get_wacky_code();
	tests_get_character();
	tests_free_tree();

	printf("\nAll of Andrey's tests passed.\n");
//end of Andrey's tests
    fscanf(stdin, "c");

    return 0;
}
