#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lab5.h"


/* ========== Node Functions ========== */

/* TODO 1: Implement create_question_node
 * - Allocate memory for a Node structure
 * - Use strdup() to copy the question string (heap allocation)
 * - Set isQuestion to 1
 * - Initialize yes and no pointers to NULL
 * - Return the new node
 */
Node *create_question_node(const char *question) 
{
    // Allocate memory for new node
    Node* newNode = (Node*)malloc(sizeof(Node));
    if(newNode==NULL)
    {
        return NULL;  // Return NULL if allocation fails
    }

    // Copy question string to heap and set node properties
    newNode->text = strdup(question);  // strdup allocates and copies string
    newNode->yes = NULL;  // Will be set later when children are added
    newNode->no = NULL;   // Will be set later when children are added
    newNode->isQuestion = 1;  // Mark as question node (not a leaf)

    return newNode;
}

/* TODO 2: Implement create_animal_node
 * - Similar to create_question_node but set isQuestion to 0
 * - This represents a leaf node with an animal name
 */
Node *create_animal_node(const char *animal) 
{
    // Allocate memory for new animal node
    Node* animalNode = (Node*)malloc(sizeof(Node));
    if(animalNode==NULL)
    {
        return NULL;  // Return NULL if allocation fails
    }

    // Copy animal name to heap and set node properties
    animalNode->text = strdup(animal);  // strdup allocates and copies string
    animalNode->yes = NULL;  // Leaf nodes have no children
    animalNode->no = NULL;   // Leaf nodes have no children
    animalNode->isQuestion = 0;  // Mark as leaf node (animal)

    return animalNode;
}

/* TODO 3: Implement free_tree (recursive)
 * - This is one of the few recursive functions allowed
 * - Base case: if node is NULL, return
 * - Recursively free left subtree (yes)
 * - Recursively free right subtree (no)
 * - Free the text string
 * - Free the node itself
 * IMPORTANT: Free children before freeing the parent!
 */
void free_tree(Node *node) 
{
    // Base case: nothing to free if node is NULL
    if(node==NULL)
    {
        return;
    }
    
    // Recursively free children first (post-order traversal)
    free_tree(node->yes);  // Free left subtree
    free_tree(node->no);   // Free right subtree
    
    // Then free this node's data
    free(node->text);  // Free the string allocated by strdup
    free(node);        // Free the node structure itself
}

/* TODO 4: Implement count_nodes (recursive)
 * - Base case: if root is NULL, return 0
 * - Return 1 + count of left subtree + count of right subtree
 */
int count_nodes(Node *root) 
{
    // Base case: NULL tree has 0 nodes
    if(root==NULL)
    {
        return 0;
    }

    // Count this node (1) plus all nodes in both subtrees
    return 1 + count_nodes(root->yes) + count_nodes(root->no);
}

/* ========== Frame Stack (for iterative tree traversal) ========== */

/* TODO 5: Implement fs_init
 * - Allocate initial array of frames (start with capacity 16)
 * - Set size to 0
 * - Set capacity to 16
 */
void fs_init(FrameStack *s) 
{
    // Allocate initial array with capacity 16
    s->frames = (Frame*)malloc(16*sizeof(Frame));
    if(s->frames==NULL)
    {
        // If allocation fails, set everything to 0/NULL
        s->size = 0;
        s->capacity = 0;
        return;
    }

    // Initialize stack as empty with capacity 16
    s->size = 0;       // Stack starts empty
    s->capacity = 16;  // Initial capacity for dynamic array
}

/* TODO 6: Implement fs_push
 * - Check if size >= capacity
 *   - If so, double the capacity and reallocate the array
 * - Store the node and answeredYes in frames[size]
 * - Increment size
 */
void fs_push(FrameStack *s, Node *node, int answeredYes) 
{
    int newCapacity;
    
    // Check if we need to resize the array
    if(s->size >= s->capacity)
    {
        // Calculate new capacity (handle edge case where capacity is 0)
        if(s->capacity==0)
        {
            newCapacity = 1;
        }
        else 
        {
            newCapacity = s->capacity * 2;  // Double the capacity
        }
        
        // Reallocate array with new capacity
        Frame* temp = realloc(s->frames, newCapacity*sizeof(Frame));
        if(temp==NULL)
        {
            return;  // Failed to resize, don't push
        }

        // Update stack with new array and capacity
        s->frames = temp;
        s->capacity = newCapacity;
    }

    // Add new frame to top of stack
    s->frames[s->size].node = node;
    s->frames[s->size].answeredYes = answeredYes;

    // Increment size to reflect new element
    s->size = s->size + 1;
}

/* TODO 7: Implement fs_pop
 * - Decrement size
 * - Return the frame at frames[size]
 * Note: No need to check if empty - caller should use fs_empty() first
 */
Frame fs_pop(FrameStack *s) 
{
    // Decrement size first (top element is at size-1)
    s->size = s->size - 1;
    
    // Return the frame that was at the top
    return s->frames[s->size];
}

/* TODO 8: Implement fs_empty
 * - Return 1 if size == 0, otherwise return 0
 */
int fs_empty(FrameStack *s) 
{
    // Check if stack has no elements
    if(s->size==0)
    {
        return 1;  // Stack is empty
    }

    return 0;  // Stack has elements
}

/* TODO 9: Implement fs_free
 * - Free the frames array
 * - Set frames pointer to NULL
 * - Reset size and capacity to 0
 */
void fs_free(FrameStack *s) 
{
    // Free the dynamically allocated array
    free(s->frames);
    
    // Reset all fields to indicate empty/freed stack
    s->frames = NULL;
    s->size = 0;
    s->capacity = 0;
}

/* ========== Edit Stack (for undo/redo) ========== */

/* TODO 10: Implement es_init
 * Similar to fs_init but for Edit structs
 */
void es_init(EditStack *s) 
{
    // Allocate initial array with capacity 16 for Edit structs
    s->edits = (Edit*)malloc(16*sizeof(Edit));
    if(s->edits==NULL)
    {
        // If allocation fails, set everything to 0/NULL
        s->size = 0;
        s->capacity = 0;
        return;
    }

    // Initialize stack as empty with capacity 16
    s->size = 0;       // Stack starts empty
    s->capacity = 16;  // Initial capacity for dynamic array
}

/* TODO 11: Implement es_push
 * Similar to fs_push but for Edit structs
 * - Check capacity and resize if needed
 * - Add edit to array and increment size
 */
void es_push(EditStack *s, Edit e) 
{
    int newCapacity;
    
    // Check if we need to resize the array
    if(s->size >= s->capacity)
    {
        // Calculate new capacity (handle edge case where capacity is 0)
        if(s->capacity == 0)
        {
            newCapacity = 1;
        }
        else
        {
            newCapacity = s->capacity * 2;  // Double the capacity
        }

        // Reallocate array with new capacity
        Edit* temp = realloc(s->edits, newCapacity*sizeof(Edit));
        if(temp==NULL)
        {
            return;  // Failed to resize, don't push
        }

        // Update stack with new array and capacity
        s->edits = temp;
        s->capacity = newCapacity;
    }

    // Add new edit to top of stack (copy struct)
    s->edits[s->size] = e;
    
    // Increment size to reflect new element
    s->size = s->size + 1;
}

/* TODO 12: Implement es_pop
 * Similar to fs_pop but for Edit structs
 */
Edit es_pop(EditStack *s) 
{
    // Decrement size first (top element is at size-1)
    s->size = s->size - 1;
    
    // Return the edit that was at the top
    return s->edits[s->size];
}

/* TODO 13: Implement es_empty
 * Return 1 if size == 0, otherwise 0
 */
int es_empty(EditStack *s) 
{
    // Check if stack has no elements
    if(s->size==0)
    {
        return 1;  // Stack is empty
    }
    return 0;  // Stack has elements
}

/* TODO 14: Implement es_clear
 * - Set size to 0 (don't free memory, just reset)
 * - This is used to clear the redo stack when a new edit is made
 */
void es_clear(EditStack *s) 
{
    // Reset size without freeing memory (keeps capacity for reuse)
    s->size = 0;
}

void es_free(EditStack *s) {
    free(s->edits);
    s->edits = NULL;
    s->size = 0;
    s->capacity = 0;
}

void free_edit_stack(EditStack *s) {
    es_free(s);
}

/* ========== Queue (for BFS traversal) ========== */

/* TODO 15: Implement q_init
 * - Set front and rear to NULL
 * - Set size to 0
 */
void q_init(Queue *q) 
{
    // Initialize empty queue
    q->front = NULL;  // No elements at front
    q->rear = NULL;   // No elements at rear
    q->size = 0;      // Queue is empty
}

/* TODO 16: Implement q_enqueue
 * - Allocate a new QueueNode
 * - Set its treeNode and id fields
 * - Set its next pointer to NULL
 * - If queue is empty (rear == NULL):
 *   - Set both front and rear to the new node
 * - Otherwise:
 *   - Link rear->next to the new node
 *   - Update rear to point to the new node
 * - Increment size
 */
void q_enqueue(Queue *q, Node *node, int id) 
{
    // Create new queue node
    QueueNode* newQNode = (QueueNode*)malloc(sizeof(QueueNode));
    newQNode->treeNode = node;  // Store the tree node
    newQNode->id = id;          // Store the ID
    newQNode->next = NULL;      // This will be the last node

    // Add to queue
    if(q->rear==NULL)
    {
        // Queue was empty, new node is both front and rear
        q->front = newQNode;
        q->rear = newQNode;
    }
    else
    {
        // Queue has elements, add to rear
        q->rear->next = newQNode;  // Link old rear to new node
        q->rear = newQNode;        // Update rear pointer
    }

    // Increment queue size
    q->size = q->size + 1;
}

/* TODO 17: Implement q_dequeue
 * - If queue is empty (front == NULL), return 0
 * - Save the front node's data to output parameters (*node, *id)
 * - Save front in a temp variable
 * - Move front to front->next
 * - If front is now NULL, set rear to NULL too
 * - Free the temp node
 * - Decrement size
 * - Return 1
 */
int q_dequeue(Queue *q, Node **node, int *id) 
{
    // Check if queue is empty
    if(q->front==NULL)
    {
        return 0;  // Nothing to dequeue
    }

    // Extract data from front node
    *node = q->front->treeNode;  // Return tree node via pointer
    *id = q->front->id;           // Return ID via pointer

    // Remove front node from queue
    QueueNode* temp = q->front;  // Save for freeing
    q->front = q->front->next;   // Move front to next node

    // If queue is now empty, update rear too
    if(q->front==NULL)
    {
        q->rear = NULL;
    }

    // Free the old front node
    free(temp);
    
    // Decrement size and indicate success
    q->size--;
    return 1;  // Successfully dequeued
}

/* TODO 18: Implement q_empty
 * Return 1 if size == 0, otherwise 0
 */
int q_empty(Queue *q) {
    // Check if queue has no elements
    if(q->size==0)
    {
        return 1;  // Queue is empty
    }

    return 0;  // Queue has elements
}

/* TODO 19: Implement q_free
 * - Dequeue all remaining nodes
 * - Use a loop with q_dequeue until queue is empty
 */
void q_free(Queue *q) 
{
    Node* n;
    int id; 

    // Dequeue all elements to free queue nodes
    while(q_dequeue(q, &n, &id))
    {
        // q_dequeue frees the QueueNode, just continue
    }

    // Ensure queue is in clean state
    q->front = NULL;
    q->rear = NULL;
    q->size = 0;
}

/* ========== Hash Table ========== */

/* TODO 20: Implement canonicalize
 * Convert a string to canonical form for hashing:
 * - Convert to lowercase
 * - Keep only alphanumeric characters
 * - Replace spaces with underscores
 * - Remove punctuation
 * Example: "Does it meow?" -> "does_it_meow"
 * 
 * Steps:
 * - Allocate result buffer (strlen(s) + 1)
 * - Iterate through input string
 * - For each character:
 *   - If alphanumeric: add lowercase version to result
 *   - If whitespace: add underscore
 *   - Otherwise: skip it
 * - Null-terminate result
 * - Return the new string
 */
char *canonicalize(const char *s) 
{
    // Allocate buffer for canonicalized string
    char* resultBuffer = (char*)malloc(strlen(s)+1);
    int j = 0;  // Index for result buffer
    
    // Process each character in input string
    for(int i = 0; s[i]!='\0'; i++)
    {
        if(isalnum(s[i]))
        {
            // Keep alphanumeric characters, convert to lowercase
            resultBuffer[j] = tolower(s[i]);
            j++;
        }
        else if(isspace(s[i]))
        {
            // Replace spaces with underscores
            resultBuffer[j] = '_';
            j++;
        }
        else 
        {
            // Skip punctuation and other characters
            continue;
        }
    }

    // Null-terminate the result string
    resultBuffer[j] = '\0';
    return resultBuffer;
}

/* TODO 21: Implement h_hash (djb2 algorithm)
 * unsigned hash = 5381;
 * For each character c in the string:
 *   hash = ((hash << 5) + hash) + c;  // hash * 33 + c
 * Return hash
 */
unsigned h_hash(const char *s) {
    unsigned hash = 5381;  // Initial hash value (djb2 magic number)
    
    // Process each character in string
    for(int i = 0; s[i]!='\0'; i++)
    {
        // hash * 33 + c (using bit shift for efficiency)
        hash = ((hash << 5) + hash) + s[i]; 
    }
    return hash;
}

/* TODO 22: Implement h_init
 * - Allocate buckets array using calloc (initializes to NULL)
 * - Set nbuckets field
 * - Set size to 0
 */
void h_init(Hash *h, int nbuckets) 
{
    // Allocate array of bucket pointers (calloc initializes to NULL)
    h->buckets = (Entry**)calloc(nbuckets, sizeof(Entry*));
    
    // Initialize hash table properties
    h->nbuckets = nbuckets;  // Number of buckets
    h->size = 0;             // Start with no entries
}

/* TODO 23: Implement h_put
 * Add animalId to the list for the given key
 * 
 * Steps:
 * 1. Compute bucket index: idx = h_hash(key) % nbuckets
 * 2. Search the chain at buckets[idx] for an entry with matching key
 * 3. If found:
 *    - Check if animalId already exists in the vals list
 *    - If yes, return 0 (no change)
 *    - If no, add animalId to vals.ids array (resize if needed), return 1
 * 4. If not found:
 *    - Create new Entry with strdup(key)
 *    - Initialize vals with initial capacity (e.g., 4)
 *    - Add animalId as first element
 *    - Insert at head of chain (buckets[idx])
 *    - Increment h->size
 *    - Return 1
 */
int h_put(Hash *h, const char *key, int animalId) 
{
    // Calculate which bucket this key belongs in
    int idx = h_hash(key) % h->nbuckets;

    // Search for existing entry with this key
    Entry* curr = h->buckets[idx];
    while(curr!=NULL)
    {
        if(strcmp(curr->key, key)==0)
        {
            break;  // Found matching key
        }
        curr = curr->next;
    }

    if(curr!=NULL)
    {
        // Entry exists, check if animalId already in list
        for(int i = 0; i<curr->vals.count; i++)
        {
            if(curr->vals.ids[i]==animalId)
            {
                return 0;  // Animal already associated with this key
            }
        }

        // Need to add animalId to existing entry
        if(curr->vals.count==curr->vals.capacity)
        {
            // Need to resize the ids array
            int newCap;
            if(curr->vals.capacity>0)
            {
                newCap = curr->vals.capacity * 2;  // Double capacity
            }
            else
            {
                newCap = 4;  // Initial capacity
            }

            // Reallocate ids array
            int* newID = (int*)realloc(curr->vals.ids, newCap*sizeof(int));
            if(newID==NULL)
            {
                return 0;  // Failed to resize
            }
            curr->vals.ids = newID;
            curr->vals.capacity = newCap;
        }

        // Add animalId to the list
        curr->vals.ids[curr->vals.count] = animalId;
        curr->vals.count +=1; 
        return 1;  // Successfully added
    }
    else 
    {
        // Create new entry for this key
        Entry* newEntry = (Entry*)malloc(sizeof(Entry));
        if(newEntry==NULL)
        {
            return 0;  // Failed to allocate
        }
        
        // Copy key string
        newEntry->key = strdup(key);
        if(newEntry->key==NULL)
        {
            free(newEntry);
            return 0;  // Failed to copy key
        }
        
        // Initialize vals list with initial capacity
        newEntry->vals.capacity = 4;
        newEntry->vals.count = 0;
        newEntry->vals.ids = (int*)malloc(4*sizeof(int));
        if(newEntry->vals.ids == NULL)
        {
            free(newEntry->key);
            free(newEntry);
            return 0;  // Failed to allocate ids array
        }
        
        // Add first animalId
        newEntry->vals.ids[newEntry->vals.count++] = animalId;

        // Insert at head of bucket chain
        newEntry->next = h->buckets[idx];
        h->buckets[idx] = newEntry;
        h->size +=1;  // Increment entry count

        return 1;  // Successfully added
    }
}

/* TODO 24: Implement h_contains
 * Check if the hash table contains the given key-animalId pair
 * 
 * Steps:
 * 1. Compute bucket index
 * 2. Search the chain for matching key
 * 3. If found, search vals.ids array for animalId
 * 4. Return 1 if found, 0 otherwise
 */
int h_contains(const Hash *h, const char *key, int animalId) 
{
    // Calculate which bucket this key would be in
    int idx = h_hash(key) % h->nbuckets;
    
    // Search for entry with matching key
    Entry* curr = h->buckets[idx];
    while(curr!=NULL)
    {
        if(strcmp(curr->key, key)==0)
        {
            break;  // Found matching key
        }
        curr = curr->next;
    }

    if(curr!=NULL)
    {
        // Entry found, search for animalId in its list
        for(int i = 0; i<curr->vals.count; i++)
        {
            if(curr->vals.ids[i]==animalId)
            {
                return 1;  // Found the key-animalId pair
            }
        }
    }

    return 0;  // Key not found or animalId not in list
}

/* TODO 25: Implement h_get_ids
 * Return pointer to the ids array for the given key
 * Set *outCount to the number of ids
 * Return NULL if key not found
 * 
 * Steps:
 * 1. Compute bucket index
 * 2. Search chain for matching key
 * 3. If found:
 *    - Set *outCount = vals.count
 *    - Return vals.ids
 * 4. If not found:
 *    - Set *outCount = 0
 *    - Return NULL
 */
int *h_get_ids(const Hash *h, const char *key, int *outCount) 
{
    // Calculate which bucket this key would be in
    int idx = h_hash(key) % h->nbuckets;
    
    // Search for entry with matching key
    Entry* curr = h->buckets[idx];
    while(curr!=NULL)
    {
        if(strcmp(curr->key, key)==0)
        {
            // Found the key, return its ids array
            *outCount = curr->vals.count;
            return curr->vals.ids;
        }
        curr = curr->next;
    }

    // Key not found
    *outCount = 0;
    return NULL;
}

/* TODO 26: Implement h_free
 * Free all memory associated with the hash table
 * 
 * Steps:
 * - For each bucket:
 *   - Traverse the chain
 *   - For each entry:
 *     - Free the key string
 *     - Free the vals.ids array
 *     - Free the entry itself
 * - Free the buckets array
 * - Set buckets to NULL, size to 0
 */
void h_free(Hash *h) 
{
    // Free all entries in each bucket
    for(int i = 0; i<h->nbuckets; i++)
    {
        Entry* curr = h->buckets[i];
        Entry* next;
        
        // Traverse the chain at this bucket
        while(curr!=NULL)
        {
            // Free entry's data
            free(curr->key);      // Free the key string
            free(curr->vals.ids); // Free the ids array
            
            // Move to next and free current entry
            next = curr->next;
            free(curr);
            curr = next;
        }
    }

    // Free the buckets array itself
    free(h->buckets);
    
    // Reset hash table to empty state
    h->buckets = NULL;
    h->size = 0;
}