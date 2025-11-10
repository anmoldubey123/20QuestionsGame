#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "lab5.h"

extern Node *g_root;

#define MAGIC 0x41544C35  /* "ATL5" */
#define VERSION 1

typedef struct {
    Node *node;
    int id;
} NodeMapping;

/* TODO 27: Implement save_tree
 * Save the tree to a binary file using BFS traversal
 * 
 * Binary format:
 * - Header: magic (4 bytes), version (4 bytes), nodeCount (4 bytes)
 * - For each node in BFS order:
 *   - isQuestion (1 byte)
 *   - textLen (4 bytes)
 *   - text (textLen bytes, no null terminator)
 *   - yesId (4 bytes, -1 if NULL)
 *   - noId (4 bytes, -1 if NULL)
 * 
 * Steps:
 * 1. Return 0 if g_root is NULL
 * 2. Open file for writing binary ("wb")
 * 3. Initialize queue and NodeMapping array
 * 4. Use BFS to assign IDs to all nodes:
 *    - Enqueue root with id=0
 *    - Store mapping[0] = {g_root, 0}
 *    - While queue not empty:
 *      - Dequeue node and id
 *      - If node has yes child: add to mappings, enqueue with new id
 *      - If node has no child: add to mappings, enqueue with new id
 * 5. Write header (magic, version, nodeCount)
 * 6. For each node in mapping order:
 *    - Write isQuestion, textLen, text bytes
 *    - Find yes child's id in mappings (or -1)
 *    - Find no child's id in mappings (or -1)
 *    - Write yesId, noId
 * 7. Clean up and return 1 on success
 */
int save_tree(const char *filename) 
{
    // Step 1: Return 0 if g_root is NULL
    if (g_root == NULL) {
        return 0;  // Nothing to save if tree is empty
    }
    
    // Step 2: Open file for writing binary ("wb")
    FILE *fp = fopen(filename, "wb");
    if (fp == NULL) {
        return 0;  // Failed to open file
    }
    
    // Step 3: Initialize queue and NodeMapping array
    Queue q;
    q_init(&q);
    
    // Count nodes to allocate mapping array
    int nodeCount = count_nodes(g_root);
    NodeMapping *mappings = malloc(nodeCount * sizeof(NodeMapping));
    if (mappings == NULL) {
        fclose(fp);
        q_free(&q);
        return 0;  // Failed to allocate memory
    }
    
    // Step 4: Use BFS to assign IDs to all nodes
    int nextId = 0;
    
    // Enqueue root with id=0
    q_enqueue(&q, g_root, 0);
    // Store mapping[0] = {g_root, 0}
    mappings[0].node = g_root;
    mappings[0].id = 0;
    nextId = 1;
    
    // BFS traversal to assign IDs to all nodes
    while (!q_empty(&q)) {
        Node *currentNode;
        int currentId;
        
        // Dequeue node and id
        q_dequeue(&q, &currentNode, &currentId);
        
        // If node has yes child: add to mappings, enqueue with new id
        if (currentNode->yes != NULL) {
            mappings[nextId].node = currentNode->yes;
            mappings[nextId].id = nextId;
            q_enqueue(&q, currentNode->yes, nextId);
            nextId++;
        }
        
        // If node has no child: add to mappings, enqueue with new id
        if (currentNode->no != NULL) {
            mappings[nextId].node = currentNode->no;
            mappings[nextId].id = nextId;
            q_enqueue(&q, currentNode->no, nextId);
            nextId++;
        }
    }
    
    // Step 5: Write header (magic, version, nodeCount)
    uint32_t magic = MAGIC;      // Magic number for file format validation
    uint32_t version = VERSION;  // Version number for compatibility checking
    uint32_t count = nodeCount;  // Total number of nodes in tree
    
    // Write header to file
    if (fwrite(&magic, sizeof(uint32_t), 1, fp) != 1 ||
        fwrite(&version, sizeof(uint32_t), 1, fp) != 1 ||
        fwrite(&count, sizeof(uint32_t), 1, fp) != 1) {
        fclose(fp);
        free(mappings);
        q_free(&q);
        return 0;  // Failed to write header
    }
    
    // Step 6: For each node in mapping order
    for (int i = 0; i < nodeCount; i++) {
        Node *node = mappings[i].node;
        
        // Write isQuestion (1 byte)
        uint8_t isQuestion = node->isQuestion;
        if (fwrite(&isQuestion, sizeof(uint8_t), 1, fp) != 1) {
            fclose(fp);
            free(mappings);
            q_free(&q);
            return 0;  // Failed to write node type
        }
        
        // Write textLen (4 bytes)
        uint32_t textLen = strlen(node->text);
        if (fwrite(&textLen, sizeof(uint32_t), 1, fp) != 1) {
            fclose(fp);
            free(mappings);
            q_free(&q);
            return 0;  // Failed to write text length
        }
        
        // Write text (textLen bytes, no null terminator)
        // Note: We don't write the null terminator to save space
        if (fwrite(node->text, 1, textLen, fp) != textLen) {
            fclose(fp);
            free(mappings);
            q_free(&q);
            return 0;  // Failed to write text content
        }
        
        // Find yes child's id in mappings (or -1 if NULL)
        int32_t yesId = -1;
        if (node->yes != NULL) {
            // Linear search through mappings to find matching node pointer
            for (int j = 0; j < nodeCount; j++) {
                if (mappings[j].node == node->yes) {
                    yesId = mappings[j].id;
                    break;
                }
            }
        }
        
        // Find no child's id in mappings (or -1 if NULL)
        int32_t noId = -1;
        if (node->no != NULL) {
            // Linear search through mappings to find matching node pointer
            for (int j = 0; j < nodeCount; j++) {
                if (mappings[j].node == node->no) {
                    noId = mappings[j].id;
                    break;
                }
            }
        }
        
        // Write yesId, noId to maintain tree structure
        if (fwrite(&yesId, sizeof(int32_t), 1, fp) != 1 ||
            fwrite(&noId, sizeof(int32_t), 1, fp) != 1) {
            fclose(fp);
            free(mappings);
            q_free(&q);
            return 0;  // Failed to write child IDs
        }
    }
    
    // Step 7: Clean up and return 1 on success
    fclose(fp);
    free(mappings);
    q_free(&q);
    
    return 1;  // Successfully saved tree
}

/* TODO 28: Implement load_tree
 * Load a tree from a binary file and reconstruct the structure
 * 
 * Steps:
 * 1. Open file for reading binary ("rb")
 * 2. Read and validate header (magic, version, count)
 * 3. Allocate arrays for nodes and child IDs:
 *    - Node **nodes = calloc(count, sizeof(Node*))
 *    - int32_t *yesIds = calloc(count, sizeof(int32_t))
 *    - int32_t *noIds = calloc(count, sizeof(int32_t))
 * 4. Read each node:
 *    - Read isQuestion, textLen
 *    - Validate textLen (e.g., < 10000)
 *    - Allocate and read text string (add null terminator!)
 *    - Read yesId, noId
 *    - Validate IDs are in range [-1, count)
 *    - Create Node and store in nodes[i]
 * 5. Link nodes using stored IDs:
 *    - For each node i:
 *      - If yesIds[i] >= 0: nodes[i]->yes = nodes[yesIds[i]]
 *      - If noIds[i] >= 0: nodes[i]->no = nodes[noIds[i]]
 * 6. Free old g_root if not NULL
 * 7. Set g_root = nodes[0]
 * 8. Clean up temporary arrays
 * 9. Return 1 on success
 * 
 * Error handling:
 * - If any read fails or validation fails, goto load_error
 * - In load_error: free all allocated memory and return 0
 */
int load_tree(const char *filename) {
    // Step 1: Open file for reading binary ("rb")
    FILE *fp = fopen(filename, "rb");
    if (fp == NULL) {
        return 0;  // File doesn't exist or can't be opened
    }
    
    // Variables for cleanup in case of error
    Node **nodes = NULL;
    int32_t *yesIds = NULL;
    int32_t *noIds = NULL;
    
    // Step 2: Read and validate header (magic, version, count)
    uint32_t magic, version, count;
    
    // Read header from file
    if (fread(&magic, sizeof(uint32_t), 1, fp) != 1 ||
        fread(&version, sizeof(uint32_t), 1, fp) != 1 ||
        fread(&count, sizeof(uint32_t), 1, fp) != 1) {
        goto load_error;  // Failed to read header
    }
    
    // Validate magic number and version for file format compatibility
    if (magic != MAGIC || version != VERSION) {
        goto load_error;  // Invalid file format or incompatible version
    }
    
    // Validate count is reasonable (sanity check)
    if (count == 0 || count > 100000) {
        goto load_error;  // Unreasonable node count (corrupted file?)
    }
    
    // Step 3: Allocate arrays for nodes and child IDs
    // Use calloc to initialize all pointers to NULL
    nodes = calloc(count, sizeof(Node*));
    yesIds = calloc(count, sizeof(int32_t));
    noIds = calloc(count, sizeof(int32_t));
    
    if (nodes == NULL || yesIds == NULL || noIds == NULL) {
        goto load_error;  // Memory allocation failed
    }
    
    // Step 4: Read each node from file
    for (uint32_t i = 0; i < count; i++) {
        uint8_t isQuestion;
        uint32_t textLen;
        
        // Read isQuestion flag and text length
        if (fread(&isQuestion, sizeof(uint8_t), 1, fp) != 1 ||
            fread(&textLen, sizeof(uint32_t), 1, fp) != 1) {
            goto load_error;  // Failed to read node metadata
        }
        
        // Validate textLen to prevent excessive memory allocation
        if (textLen == 0 || textLen >= 10000) {
            goto load_error;  // Invalid text length (corrupted data?)
        }
        
        // Allocate and read text string (add null terminator!)
        char *text = malloc(textLen + 1);  // +1 for null terminator
        if (text == NULL) {
            goto load_error;  // Memory allocation failed
        }
        
        // Read text content from file
        if (fread(text, 1, textLen, fp) != textLen) {
            free(text);
            goto load_error;  // Failed to read text
        }
        text[textLen] = '\0';  // CRITICAL: Add null terminator since file doesn't store it
        
        // Read child node IDs
        if (fread(&yesIds[i], sizeof(int32_t), 1, fp) != 1 ||
            fread(&noIds[i], sizeof(int32_t), 1, fp) != 1) {
            free(text);
            goto load_error;  // Failed to read child IDs
        }
        
        // Validate IDs are in valid range [-1, count)
        if (yesIds[i] < -1 || yesIds[i] >= (int32_t)count ||
            noIds[i] < -1 || noIds[i] >= (int32_t)count) {
            free(text);
            goto load_error;  // Invalid child ID (corrupted data?)
        }
        
        // Create Node and store in nodes array
        Node *newNode = malloc(sizeof(Node));
        if (newNode == NULL) {
            free(text);
            goto load_error;  // Memory allocation failed
        }
        
        // Initialize node with read data
        newNode->text = text;
        newNode->isQuestion = isQuestion;
        newNode->yes = NULL;  // Will link in next phase
        newNode->no = NULL;   // Will link in next phase
        
        nodes[i] = newNode;
    }
    
    // Step 5: Link nodes using stored IDs (second pass)
    for (uint32_t i = 0; i < count; i++) {
        // Link yes child if ID is valid (>= 0)
        if (yesIds[i] >= 0) {
            nodes[i]->yes = nodes[yesIds[i]];
        }
        
        // Link no child if ID is valid (>= 0)
        if (noIds[i] >= 0) {
            nodes[i]->no = nodes[noIds[i]];
        }
    }
    
    // Step 6: Free old g_root if not NULL
    if (g_root != NULL) {
        free_tree(g_root);  // Clean up existing tree before replacing
    }
    
    // Step 7: Set g_root = nodes[0] (root is always first in BFS order)
    g_root = nodes[0];
    
    // Step 8: Clean up temporary arrays (no longer needed)
    free(nodes);
    free(yesIds);
    free(noIds);
    fclose(fp);
    
    // Step 9: Return 1 on success
    return 1;
    
load_error:
    // Error handling: free all allocated memory and return 0
    if (nodes != NULL) {
        // Free all allocated nodes and their text
        for (uint32_t i = 0; i < count; i++) {
            if (nodes[i] != NULL) {
                if (nodes[i]->text != NULL) {
                    free(nodes[i]->text);
                }
                free(nodes[i]);
            }
        }
        free(nodes);
    }
    // Free ID arrays if allocated
    if (yesIds != NULL) {
        free(yesIds);
    }
    if (noIds != NULL) {
        free(noIds);
    }
    fclose(fp);
    return 0;  // Failed to load tree
}