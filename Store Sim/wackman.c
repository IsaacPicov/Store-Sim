
#include "wackman.h"


int sum_array_elements(int int_array[], int array_size) {
    int sum =0;
    if(int_array == NULL){
        return 0;
    }
    for(int i =0; i<array_size; i++){
        sum += int_array[i]; 
    }
    return sum;
}


void compute_occurrence_array(int occurrence_array[ASCII_CHARACTER_SET_SIZE], char* string) {
    char* p;
    p = string;
    if(occurrence_array == NULL){
        return;
    }
    for (int i =0; i < ASCII_CHARACTER_SET_SIZE; i++){
        occurrence_array[i] = 0; 
    } 
    if(string == NULL){
        return;
    }
    while(*p != '\0'){
        occurrence_array[*p] +=1; 
        p++; 
    }
}


int count_positive_occurrences(int occurrence_array[ASCII_CHARACTER_SET_SIZE]) {
    int sum =0;
    if(occurrence_array == NULL){
        return 0;
    }
    for (int i =0; i < ASCII_CHARACTER_SET_SIZE; i++)
        if(occurrence_array[i] > 0){
            sum++; 
        }
    return sum;
}

WackyLinkedNode* create_wacky_list(int occurrence_array[ASCII_CHARACTER_SET_SIZE]) {
    WackyLinkedNode* head = NULL;
    WackyTreeNode* val = NULL;
    int arr_sum = sum_array_elements(occurrence_array, ASCII_CHARACTER_SET_SIZE);
    if(occurrence_array == NULL){
        return NULL; 
    }
    for(int i =0; i < ASCII_CHARACTER_SET_SIZE; i++){
        if(occurrence_array[i] > 0){
            double weight = (double)occurrence_array[i] / arr_sum;

            char curr_val = occurrence_array[i];
            val = new_leaf_node(weight, i);

            WackyLinkedNode* linked_node = new_linked_node(val);

            if(head == NULL || weight < head -> val -> weight || (weight == head -> val-> weight && i < head -> val -> val)){
                linked_node-> next = head;
                head = linked_node; 
            }
            else{
                 WackyLinkedNode* temp = NULL;
                 temp = head; 
                while(temp -> next != NULL && ((weight == temp ->next -> val-> weight && i > temp -> next->val -> val) || temp -> next -> val -> weight < weight)){
                    temp = temp -> next; 
                }
                linked_node -> next = temp -> next;
                temp -> next = linked_node;
            }

        }
    }
    return head;
}


WackyTreeNode* merge_wacky_list(WackyLinkedNode* linked_list) {
    WackyLinkedNode* head = linked_list;
    WackyTreeNode* boobs = NULL; 
    if (head == NULL){
        return NULL;
    }
    if (head -> next == NULL){
        boobs = head->val; 
        free(head);
        return boobs; 
    }
    WackyLinkedNode *first = NULL, *second = NULL, *new_node = NULL; 
    WackyTreeNode* new_branch = NULL;
    while(head -> next != NULL){
        first = head;
        second = head->next; 
        head = head->next->next; 
        new_branch = new_branch_node(first->val, second->val); 
        new_node = new_linked_node(new_branch); 
        free(first);
        free(second);
        if(head == NULL || new_node->val->weight < head ->val->weight|| new_node->val->weight == head -> val ->weight){
            new_node -> next = head;
            head = new_node; 
        }
        else{
            WackyLinkedNode* temp2 = NULL;
            temp2 = head;
            while(temp2 ->next != NULL && temp2 -> next -> val -> weight < new_node->val->weight){
                temp2 = temp2 ->next;
            }
            new_node -> next = temp2 -> next;
            temp2 -> next = new_node; 
        }
    }
    boobs = head->val;
    free(head);
    return boobs; 
}


int get_height(WackyTreeNode* tree) {
    if (tree == NULL)
        return 0; 
    return MAX(get_height(tree->left), get_height(tree->right))+1;
}

int wacky_helper(WackyTreeNode* tree, char character, bool* bool_arr,int depth){
   if(tree == NULL || bool_arr == NULL){
    return -1; 
   }
   if(tree->val == character){
    return depth; 
    }
        bool_arr[depth] = false; 
        int left = wacky_helper(tree->left, character, bool_arr, depth+1);
        if(left != -1){
            return left;
        }

        bool_arr[depth] = true; 
        int right =wacky_helper(tree->right,character, bool_arr, depth+1);

         if(right != -1){
        return right;
        }
    return -1; 
}

void get_wacky_code(WackyTreeNode* tree, char character, bool boolean_array[], int* array_size) {
    if(array_size == NULL){
        return;
    }
    *array_size = wacky_helper(tree, character, boolean_array, 0);
}

char get_character(WackyTreeNode* tree, bool boolean_array[], int array_size) {
    char value = '\0';
    if (tree == NULL || boolean_array == NULL){
        return '\0';
    }
    for (int i =0; i < array_size; i++){
        if (boolean_array[i] == true){
            if(tree -> right == NULL){
                return '\0';
            }
            tree = tree -> right;
            
        }
        else{
            if(tree -> left == NULL){
                return '\0';
            }
            tree = tree->left; 
        }
    }
    return tree-> val;
}

void free_tree(WackyTreeNode* tree) {
    if (tree == NULL)
        return;
    free_tree(tree->left);
    free_tree(tree->right);
    free(tree); 
}
 