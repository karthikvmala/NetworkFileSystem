#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define BUFFER_SIZE 4096
#define MAX_DATA_LENGTH 1024
typedef struct trie_node
{
    char *key;
    int end;
    int ssid;
    struct trie_node *children[256]; 
} trie_node;

struct trie_node *create_trie_node()
{
    // Allocating memory to the node
    struct trie_node *node = (struct trie_node *)malloc(sizeof(struct trie_node));
    if (node == NULL)
    {
        fprintf(stderr, "Error in allocating memory for trie");
        return NULL;
    }
    node->key = NULL; 
    node->end = 0;    
    node->ssid = -1;  
    int i = 0;
    while (i < 256)
    {
        node->children[i] = NULL;
        i++;
    }
    return node;
}

int insert_path(struct trie_node *root, char *String_to_search, int ssid)
{
    struct trie_node *current = root;
    int i = 0;
    while (String_to_search[i] != '\0')
    {
        int index = (int)String_to_search[i];

        if (current->children[index] == NULL)
        {
            current->children[index] = create_trie_node();
            if (current->children[index] == NULL)
            {
                fprintf(stderr, "create_trie_node : could not create node.\n");
                return 0;
            }
        }
        current = current->children[index];
        i++;
    }
    current->key = strdup(String_to_search);
    if (current->key == NULL) {
        fprintf(stderr, "Failed to allocate memory for key.\n");
        return 0;
    }
    current->end = 1;
    current->ssid = ssid;
    return 1;
}


int search_path(struct trie_node *root, char *key)
{
    struct trie_node *current = root;
    int i = 0;
    while (key[i] != '\0')
    {
        int index = (int)key[i];

        if (current->children[index] == NULL)
        {
            return -1;
        }
        current = current->children[index];
        i++;
    }

    if (current->end == 1)
    {
        return current->ssid;
    }
    return -1;
}

int delete_path(struct trie_node *root, char *string_to_delete)
{
    struct trie_node *current = root;
    int i = 0;
    while (string_to_delete[i] != '\0')
    {
        int index = (int)string_to_delete[i];

        if (current->children[index] == NULL)
        {
            return 0;
        }
        current = current->children[index];
        i++;
    }
    
    current->key = NULL;
    current->end = 0;
    current->ssid = -1;
    return 1;
}

void print_paths(struct trie_node *root)
{
    if (root != NULL)
    {
        if (root->end == 1)
        {
            printf("%s\n %d\n", root->key,root->ssid);
        }

        int i = 0;
        while (i < 256)
        {
            if (root->children[i] != NULL)
            {
                print_paths(root->children[i]);
            }
            i++;
        }
    }
}
void add_paths_to_array(char paths[][MAX_DATA_LENGTH], int *cnt, struct trie_node *root) {
    if (root != NULL) {
        if (root->end == 1) {
            strcpy(paths[*cnt], root->key);
            (*cnt)++;
        }

        int i = 0;
        while (i < 256) {
            if (root->children[i] != NULL) {
                add_paths_to_array(paths, cnt, root->children[i]);
            }
            i++;
        }
    }
}
    
