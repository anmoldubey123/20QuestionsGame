#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include "lab5.h"


extern Node *g_root;
extern EditStack g_undo;
extern EditStack g_redo;
extern Hash g_index;

/* TODO 31: Implement play_game
 * Main game loop using iterative traversal with a stack
 * 
 * Key requirements:
 * - Use FrameStack (NO recursion!)
 * - Push frames for each decision point
 * - Track parent and answer for learning
 * 
 * Steps:
 * 1. Initialize and display game UI
 * 2. Initialize FrameStack
 * 3. Push root frame with answeredYes = -1
 * 4. Set parent = NULL, parentAnswer = -1
 * 5. While stack not empty:
 *    a. Pop current frame
 *    b. If current node is a question:
 *       - Display question and get user's answer (y/n)
 *       - Set parent = current node
 *       - Set parentAnswer = answer
 *       - Push appropriate child (yes or no) onto stack
 *    c. If current node is a leaf (animal):
 *       - Ask "Is it a [animal]?"
 *       - If correct: celebrate and break
 *       - If wrong: LEARNING PHASE
 *         i. Get correct animal name from user
 *         ii. Get distinguishing question
 *         iii. Get answer for new animal (y/n for the question)
 *         iv. Create new question node and new animal node
 *         v. Link them: if newAnswer is yes, newQuestion->yes = newAnimal
 *         vi. Update parent pointer (or g_root if parent is NULL)
 *         vii. Create Edit record and push to g_undo
 *         viii. Clear g_redo stack
 *         ix. Update g_index with canonicalized question
 * 6. Free stack
 */
void play_game() {
    clear();
    attron(COLOR_PAIR(5) | A_BOLD);
    mvprintw(0, 0, "%-80s", " Playing 20 Questions");
    attroff(COLOR_PAIR(5) | A_BOLD);
    
    mvprintw(2, 2, "Think of an animal, and I'll try to guess it!");
    mvprintw(3, 2, "Press any key to start...");
    refresh();
    getch();
    
    // Step 2: Initialize FrameStack
    FrameStack stack;
    fs_init(&stack);
    
    // Step 3: Push root frame with answeredYes = -1
    fs_push(&stack, g_root, -1);
    
    // Step 4: Set parent = NULL, parentAnswer = -1
    Node *parent = NULL;
    int parentAnswer = -1;
    
    // Step 5: While stack not empty
    while (!fs_empty(&stack)) {
        // Step 5a: Pop current frame
        Frame current = fs_pop(&stack);
        Node *currentNode = current.node;
        
        clear();
        attron(COLOR_PAIR(5) | A_BOLD);
        mvprintw(0, 0, "%-80s", " Playing 20 Questions");
        attroff(COLOR_PAIR(5) | A_BOLD);
        
        // Step 5b: If current node is a question
        if (currentNode->isQuestion) {
            // Display question and get user's answer
            int answer = get_yes_no(4, 2, currentNode->text);
            
            // Set parent = current node
            parent = currentNode;
            // Set parentAnswer = answer
            parentAnswer = answer;
            
            // Push appropriate child (yes or no) onto stack
            if (answer) {
                if (currentNode->yes != NULL) {
                    fs_push(&stack, currentNode->yes, 1);
                }
            } else {
                if (currentNode->no != NULL) {
                    fs_push(&stack, currentNode->no, 0);
                }
            }
        } 
        // Step 5c: If current node is a leaf (animal)
        else {
            // Ask "Is it a [animal]?"
            char guessQuestion[256];
            snprintf(guessQuestion, sizeof(guessQuestion), "Is it a %s? (y/n): ", currentNode->text);
            int correct = get_yes_no(4, 2, guessQuestion);
            
            // If correct: celebrate and break
            if (correct) {
                attron(COLOR_PAIR(3) | A_BOLD);
                mvprintw(6, 2, "I guessed it! I'm so smart!");
                attroff(COLOR_PAIR(3) | A_BOLD);
                mvprintw(8, 2, "Press any key to continue...");
                refresh();
                getch();
                break;
            } 
            // If wrong: LEARNING PHASE
            else {
// Step 5c.i: Get correct animal name from user
                clear();
                attron(COLOR_PAIR(5) | A_BOLD);
                mvprintw(0, 0, "%-80s", " Learning New Animal");
                attroff(COLOR_PAIR(5) | A_BOLD);

                mvprintw(2, 2, "I give up! You win!");
                char *correctAnimalInput = get_input(4, 2, "What animal were you thinking of? ");
                // Make a copy of the animal name since get_input uses a static buffer
                char correctAnimal[256];
                strcpy(correctAnimal, correctAnimalInput);

                // Step 5c.ii: Get distinguishing question
                char questionPrompt[512];
                snprintf(questionPrompt, sizeof(questionPrompt), 
                        "Please give me a yes/no question that distinguishes a %s from a %s: ",
                        correctAnimal, currentNode->text);
                char *newQuestionInput = get_input(6, 2, questionPrompt);
                // Make a copy of the question since get_input uses a static buffer
                char newQuestionText[256];
                strcpy(newQuestionText, newQuestionInput);

                // Step 5c.iii: Get answer for new animal (y/n for the question)
                char answerPrompt[512];
                snprintf(answerPrompt, sizeof(answerPrompt),
                        "For a %s, what is the answer to \"%s\"? (y/n): ",
                        correctAnimal, newQuestionText);
                int newAnswer = get_yes_no(8, 2, answerPrompt);

                // Step 5c.iv: Create new question node and new animal node
                Node *newQuestion = create_question_node(newQuestionText);
                Node *newAnimal = create_animal_node(correctAnimal);

                // Step 5c.v: Link them: if newAnswer is yes, newQuestion->yes = newAnimal
                if (newAnswer) {
                    newQuestion->yes = newAnimal;
                    newQuestion->no = currentNode;
                } else {
                    newQuestion->no = newAnimal;
                    newQuestion->yes = currentNode;
                }
                
                // Step 5c.vi: Update parent pointer (or g_root if parent is NULL)
                if (parent == NULL) {
                    // We're replacing the root
                    g_root = newQuestion;
                } else if (parentAnswer == 1) {
                    // We came from the yes branch
                    parent->yes = newQuestion;
                } else {
                    // We came from the no branch
                    parent->no = newQuestion;
                }
                
                // Step 5c.vii: Create Edit record and push to g_undo
                Edit edit;
                edit.type = EDIT_INSERT_SPLIT;
                edit.parent = parent;
                edit.wasYesChild = parentAnswer;
                edit.oldLeaf = currentNode;
                edit.newQuestion = newQuestion;
                edit.newLeaf = newAnimal;
                es_push(&g_undo, edit);
                
                // Step 5c.viii: Clear g_redo stack
                es_clear(&g_redo);
                
                // Step 5c.ix: Update g_index with canonicalized question
                char *canonicalQuestion = canonicalize(newQuestionText);
                // Get ID for the new animal (we can use a simple counter based on tree size)
                int animalId = count_nodes(g_root);
                h_put(&g_index, canonicalQuestion, animalId);
                free(canonicalQuestion);
                
                attron(COLOR_PAIR(3));
                mvprintw(10, 2, "Thanks! I've learned about %s!", correctAnimal);
                attroff(COLOR_PAIR(3));
                mvprintw(12, 2, "Press any key to continue...");
                refresh();
                getch();
                break;
            }
        }
    }
    
    // Step 6: Free stack
    fs_free(&stack);
}

/* TODO 32: Implement undo_last_edit
 * Undo the most recent tree modification
 * 
 * Steps:
 * 1. Check if g_undo stack is empty, return 0 if so
 * 2. Pop edit from g_undo
 * 3. Restore the tree structure:
 *    - If edit.parent is NULL:
 *      - Set g_root = edit.oldLeaf
 *    - Else if edit.wasYesChild:
 *      - Set edit.parent->yes = edit.oldLeaf
 *    - Else:
 *      - Set edit.parent->no = edit.oldLeaf
 * 4. Push edit to g_redo stack
 * 5. Return 1
 * 
 * Note: We don't free newQuestion/newLeaf because they might be redone
 */
int undo_last_edit() 
{
    if(g_undo.size==0)
    {
        return 0;
    }

    EditStack* undoPtr = &g_undo;
    EditStack* redoPtr = &g_redo;
    Edit edit = es_pop(undoPtr);

    if(edit.parent==NULL)
    {
        g_root = edit.oldLeaf;
    }
    else if(edit.wasYesChild)
    {
        edit.parent->yes = edit.oldLeaf;
    }
    else 
    {
        edit.parent->no = edit.oldLeaf;
    }
    es_push(redoPtr, edit);
    return 1;
}

/* TODO 33: Implement redo_last_edit
 * Redo a previously undone edit
 * 
 * Steps:
 * 1. Check if g_redo stack is empty, return 0 if so
 * 2. Pop edit from g_redo
 * 3. Reapply the tree modification:
 *    - If edit.parent is NULL:
 *      - Set g_root = edit.newQuestion
 *    - Else if edit.wasYesChild:
 *      - Set edit.parent->yes = edit.newQuestion
 *    - Else:
 *      - Set edit.parent->no = edit.newQuestion
 * 4. Push edit back to g_undo stack
 * 5. Return 1
 */
int redo_last_edit() 
{
    if(g_redo.size==0)
    {
        return 0;
    }

    EditStack* redoPtr = &g_redo;
    EditStack* undoPtr = &g_undo;
    Edit edit = es_pop(redoPtr);

    if(edit.parent==NULL)
    {
        g_root = edit.newQuestion;
    }
    else if(edit.wasYesChild)
    {
        edit.parent->yes = edit.newQuestion;
    }
    else 
    {
        edit.parent->no = edit.newQuestion;
    }

    es_push(undoPtr, edit);

    return 1;
}
