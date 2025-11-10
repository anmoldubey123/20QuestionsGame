#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lab5.h"

extern Node *g_root;

/* TODO 29: Implement check_integrity
 * Use BFS to verify tree structure:
 * - Question nodes must have both yes and no children (not NULL)
 * - Leaf nodes (isQuestion == 0) must have NULL children
 * 
 * Return 1 if valid, 0 if invalid
 * 
 * Steps:
 * 1. Return 1 if g_root is NULL (empty tree is valid)
 * 2. Initialize queue and enqueue root with id=0
 * 3. Set valid = 1
 * 4. While queue not empty:
 *    - Dequeue node
 *    - If node->isQuestion:
 *      - Check if yes == NULL or no == NULL
 *      - If so, set valid = 0 and break
 *      - Otherwise, enqueue both children
 *    - Else (leaf node):
 *      - Check if yes != NULL or no != NULL
 *      - If so, set valid = 0 and break
 * 5. Free queue and return valid
 */
int check_integrity() {
    // Step 1: Return 1 if g_root is NULL (empty tree is valid)
    if (g_root == NULL) {
        return 1;  // An empty tree is considered valid
    }
    
    // Step 2: Initialize queue and enqueue root with id=0
    Queue q;
    q_init(&q);
    q_enqueue(&q, g_root, 0);  // Start BFS from root with ID 0
    
    // Step 3: Set valid = 1 (assume valid until proven otherwise)
    int valid = 1;
    
    // Step 4: While queue not empty - BFS traversal of tree
    while (!q_empty(&q)) {
        Node *node;
        int id;
        
        // Dequeue next node to check
        q_dequeue(&q, &node, &id);
        
        // Check if node follows the rules based on its type
        if (node->isQuestion) {
            // Question nodes MUST have both yes and no children
            if (node->yes == NULL || node->no == NULL) {
                // Invalid: Question node missing a child
                valid = 0;
                break;  // Stop checking once we find an error
            }
            // Valid question node - enqueue both children for checking
            q_enqueue(&q, node->yes, id * 2 + 1);  // Left child gets odd ID
            q_enqueue(&q, node->no, id * 2 + 2);   // Right child gets even ID
        }
        else {
            // Leaf nodes (animals) must NOT have any children
            if (node->yes != NULL || node->no != NULL) {
                // Invalid: Leaf node has children
                valid = 0;
                break;  // Stop checking once we find an error
            }
            // Valid leaf node - no children to enqueue
        }
    }
    
    // Step 5: Free queue and return validity result
    q_free(&q);  // Clean up queue memory
    return valid;  // Return 1 if valid, 0 if invalid
}

typedef struct PathNode {
    Node *treeNode;
    struct PathNode *parent;
    int viaYes;  /* 1 if reached via yes, 0 if via no, -1 for root */
} PathNode;

/* TODO 30: Implement find_shortest_path (OPTIONAL CHALLENGE)
 * Find the shortest distinguishing path between two animals
 * 
 * This is an optional challenge problem. If you want to attempt it:
 * 
 * Steps:
 * 1. Use BFS to find paths to both animals
 * 2. Build PathNode structures that track parent and direction
 * 3. Build arrays of PathNodes from each animal to root
 * 4. Find the Lowest Common Ancestor (LCA)
 * 5. Print the questions that distinguish them
 * 
 * Algorithm:
 * - Use BFS with a PathNode array to track the path taken
 * - For each node visited, record its parent and which branch (yes/no)
 * - Once both animals are found, trace back to root for each
 * - Find where the paths diverge (LCA)
 * - Print the distinguishing questions from LCA to each animal
 */
void find_shortest_path(const char *animal1, const char *animal2) {
    if (g_root == NULL) return;
    
    // TODO: Implement this function (OPTIONAL CHALLENGE)
    // This is complex and requires careful path tracking
    
    printf("find_shortest_path not yet implemented\n");
}