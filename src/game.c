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
    
    // TODO: Implement the game loop
    // Initialize FrameStack
    // Push root
    // Loop until stack empty or guess is correct
    // Handle question nodes and leaf nodes differently
    
    //Step 2
    FrameStack stack;
    fs_init(&stack);

    //Step 3
    fs_push(&stack, g_root, -1);

    //Step 4
    Node* parent = NULL;
    int parentAnswer = -1;

    //Step 5
    while(stack.size!=0)
    {
        clear();
        attron(COLOR_PAIR(5) | A_BOLD);
        mvprintw(0, 0, "%-80s", " Playing 20 Questions");
        attroff(COLOR_PAIR(5) | A_BOLD);
        mvprintw(2, 2, "Think of an animal, and I'll try to guess it!");
        refresh();


        Frame currFrame = fs_pop(&stack);
        Node* currNode = currFrame.node;
        if(currNode->isQuestion)
        {
            char prompt[256];
            snprintf(prompt, sizeof(prompt), "%s (y/n): ", currNode->text);
            int ans = get_yes_no(5, 2, prompt);

            parent = currNode;
            parentAnswer = ans;

            Node* nextNode; 
            if(ans==1)
            {
                nextNode = currNode->yes;
            }
            else 
            {
                nextNode = currNode->no;
            }

            fs_push(&stack, nextNode, ans);
        }
        else 
        {
            char guessPrompt[256];
            snprintf(guessPrompt, sizeof(guessPrompt), "Is it a %s? (y/n): ", currNode->text);
            int correct = get_yes_no(5, 2, guessPrompt);

            if(correct==1)
            {
                mvprintw(7, 2, "Yay! I guessed it! 🎉  Press any key...");
                refresh();
                getch();
                break;
            }

            char* newAnimalName_in  = get_input(7, 2, "What animal were you thinking of? ");
            char* newQuestionText_in= get_input(9, 2, "Give me a yes/no question to distinguish them: ");

            char* newAnimalName  = strdup(newAnimalName_in);
            char* newQuestionText= strdup(newQuestionText_in);

            int newAns = get_yes_no(11, 2, "For your animal, what is the answer? (y/n): ");

            Node* newQuestion = create_question_node(newQuestionText);
            Node* newLeaf = create_animal_node(newAnimalName);

            free(newAnimalName);
            free(newQuestionText);

            if(newAns==1)
            {
                newQuestion->yes = newLeaf;
                newQuestion->no = currNode;
            }
            else 
            {
                newQuestion->yes = currNode;
                newQuestion->no = newLeaf;
            }

            clear();
            mvprintw(15, 2, "[dbg] newQ=%p  yes=%p  no=%p  oldLeaf=%p  newLeaf=%p",
            (void*)newQuestion, (void*)newQuestion->yes, (void*)newQuestion->no,
            (void*)currNode, (void*)newLeaf);
            mvprintw(16, 2, "[dbg] yes txt: %s", newQuestion->yes ? newQuestion->yes->text : "<null>");
            mvprintw(17, 2, "[dbg]  no txt: %s", newQuestion->no  ? newQuestion->no->text  : "<null>");
            refresh(); getch();

            if(parent==NULL)
            {
                g_root = newQuestion;
            }
            else 
            {
                if(parentAnswer==1)
                {
                    parent->yes = newQuestion;
                }
                else 
                {
                    parent->no = newQuestion;
                }
            }

            Edit e;
            e.type = EDIT_INSERT_SPLIT;
            e.parent = parent;
            e.wasYesChild = (parent == NULL) ? -1 : parentAnswer;
            e.oldLeaf = currNode;
            e.newQuestion = newQuestion;
            e.newLeaf = newLeaf;
            es_push(&g_undo, e);

            es_clear(&g_redo);

            char* canon = canonicalize(newQuestion->text);

            static int nextAnimalId = 1;

            if(newAns==1)
            {
                h_put(&g_index, canon, nextAnimalId++);
            }

            free(canon);

            mvprintw(13, 2, "Thanks! I learned something new. Press any key to continue...");
            refresh();
            getch();

            break;
        }
    }
    
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
