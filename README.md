# 20 Questions: Guess the Animal

## Interactive Learning Game with Data Structures in C

## Overview

A terminal-based implementation of the classic "20 Questions" game that learns from its mistakes. When the computer fails to guess your animal, it asks you to teach it a distinguishing question and adds the new knowledge to its decision tree. The game features persistent storage, undo/redo functionality, and a visual tree explorer.

## Author

- **Anmol Dubey** 

## Course

ECE 312 - Introduction to Programming | The University of Texas at Austin

## Features

| Feature | Description |
|---------|-------------|
| 🎯 Interactive Gameplay | Classic 20 Questions with yes/no navigation through decision tree |
| 🧠 Machine Learning | Game learns new animals and distinguishing questions from user |
| 💾 Persistent Storage | Binary file format preserves learned knowledge across sessions |
| ↩️ Undo/Redo | Full edit history with dual-stack implementation |
| 🌳 Tree Visualization | Scrollable ncurses display of entire decision tree |
| 🔍 Integrity Checking | BFS validation ensures tree structure correctness |
| ⚡ Iterative Traversal | Explicit stack-based gameplay (no recursion) |

## How It Works

The game maintains a binary decision tree where internal nodes contain yes/no questions and leaf nodes contain animal names. Starting from the root, the game asks questions and follows the appropriate branch based on user responses until reaching a leaf node (guess).

```
Initial Tree:
       "Does it live in water?"
        /                    \
      YES                    NO
       |                      |
    "Fish"                  "Dog"

After Learning "Cat":
       "Does it live in water?"
        /                    \
      YES                    NO
       |                      |
    "Fish"           "Does it meow?"
                      /            \
                    YES            NO
                     |              |
                   "Cat"          "Dog"
```

When the game guesses incorrectly, it enters learning mode: the user provides the correct animal and a distinguishing question. The old leaf node is replaced with a new question node, with the new and old animals as children.

## Data Structures Implemented

### Binary Decision Tree
Stores the knowledge base with question nodes (internal) and animal nodes (leaves). Each node contains text, child pointers, and a type flag.

### Dynamic Array Stack (FrameStack)
Used for iterative tree traversal during gameplay. Automatically doubles capacity when full, providing amortized O(1) push operations.

### Linked List Queue
Implements BFS traversal for tree serialization and integrity checking. Standard FIFO with front/rear pointers.

### Hash Table with Separate Chaining
Indexes question attributes for potential query optimization. Uses djb2 hashing algorithm with collision resolution via linked lists.

### Edit Stack
Tracks tree modifications for undo/redo functionality. Stores complete edit records including parent pointers and old/new node references.

## Building and Running

### Prerequisites

**Ubuntu/Debian:**
```bash
sudo apt-get install build-essential libncurses5-dev libncursesw5-dev
```

**macOS:**
```bash
brew install gcc make ncurses
```

**Windows:** Use WSL with Ubuntu instructions.

### Build Commands

```bash
cd src

make              # Build main program
make test         # Build and run unit tests
make run          # Build and launch game
make valgrind     # Run with memory leak detection
make clean        # Remove build artifacts
```

## Usage

Launch the game with `make run`. The main menu provides these options:

| Key | Action |
|-----|--------|
| P | Play a round of 20 Questions |
| V | View the decision tree |
| U | Undo last learned animal |
| R | Redo undone edit |
| S | Save tree to `animals.dat` |
| L | Load tree from `animals.dat` |
| I | Check tree integrity |
| Q | Quit |

### Example Session

```
Think of an animal, and I'll try to guess it!

Does it live in water? (y/n): n
Is it a Dog? (y/n): n

I give up! You win!
What animal were you thinking of? Cat
Please give me a yes/no question that distinguishes a Cat from a Dog: Does it meow?
For a Cat, what is the answer to "Does it meow?"? (y/n): y

Thanks! I've learned about Cat!
```

## Project Structure

```
20Questions/
├── README.md                    # This file
├── Documentation/
│   ├── README.md               # Implementation guide
│   ├── BUILD.md                # Build and debugging reference
│   └── HINTS.md                # Implementation hints
└── src/
    ├── Makefile                # Build system
    ├── lab5.h                  # Type definitions and function prototypes
    ├── main.c                  # ncurses UI and program entry
    ├── ds.c                    # Data structure implementations
    ├── game.c                  # Game logic and undo/redo
    ├── persist.c               # Binary file I/O
    ├── utils.c                 # Integrity checker
    ├── visualize.c             # Tree visualization
    ├── tests.c                 # Unit test suite
    └── test_globals.c          # Test harness globals
```

## Implementation Details

### Memory Management

All dynamic memory is manually managed with careful attention to allocation/deallocation pairs. The `strdup()` function is used for string copying, and recursive `free_tree()` ensures proper cleanup using post-order traversal.

### Binary File Format

Trees are serialized using BFS traversal with node ID assignment:

| Field | Size | Description |
|-------|------|-------------|
| Magic | 4 bytes | File format identifier (`0x41544C35`) |
| Version | 4 bytes | Format version number |
| Count | 4 bytes | Total node count |
| Per Node: | | |
| - isQuestion | 1 byte | Node type flag |
| - textLen | 4 bytes | Text string length |
| - text | N bytes | Text content (no null terminator) |
| - yesId | 4 bytes | Yes child ID (-1 if NULL) |
| - noId | 4 bytes | No child ID (-1 if NULL) |

### Undo/Redo System

Edits are recorded as complete snapshots containing parent pointer, branch direction, old leaf, new question node, and new animal node. Undo restores the old leaf at the recorded location and moves the edit to the redo stack. Nodes are not freed during undo/redo to allow reversal.

## Testing

The test suite validates each data structure independently:

```bash
make test
```

Expected output:
```
=== Running Unit Tests ===

Testing Node Functions...
  ✓ Node tests passed
Testing Frame Stack...
  ✓ Stack tests passed
Testing Edit Stack...
  ✓ Edit stack tests passed
Testing Queue...
  ✓ Queue tests passed
Testing Canonicalization...
  ✓ Canonicalization tests passed
Testing Hash Table...
  ✓ Hash table tests passed
Testing Persistence...
  ✓ Persistence tests passed
Testing Integrity Checker...
  ✓ Integrity tests passed

=== All Tests Passed! ===
```

### Memory Validation

```bash
make valgrind-test
```

Expected result: "All heap blocks were freed -- no leaks are possible"

## Algorithm Complexity

| Operation | Time | Space |
|-----------|------|-------|
| Tree traversal (play) | O(h) | O(h) |
| Learn new animal | O(1) | O(1) |
| Save tree (BFS) | O(n) | O(n) |
| Load tree | O(n) | O(n) |
| Integrity check | O(n) | O(n) |
| Undo/Redo | O(1) | O(1) |
| Hash put/contains | O(1) avg | O(1) |

Where h = tree height, n = number of nodes.

## Key Design Decisions

**Iterative vs. Recursive Traversal:** The game loop uses explicit stacks rather than recursion to demonstrate how compilers transform recursive calls and to maintain parent tracking for tree modification.

**BFS for Serialization:** Breadth-first traversal assigns contiguous IDs, simplifying the binary format and enabling single-pass reconstruction during load.

**Dual Stack Undo/Redo:** Separate undo and redo stacks with preserved node references allow unlimited undo depth without memory duplication.

**Separate Chaining Hash Table:** Chosen for simplicity and predictable worst-case behavior. The djb2 hash function provides good distribution for string keys.

## References

- [ncurses Programming HOWTO](https://tldp.org/HOWTO/NCURSES-Programming-HOWTO/)
- [djb2 Hash Function](http://www.cse.yorku.ca/~oz/hash.html)
- Data Structures and Algorithm Analysis in C (Weiss)
