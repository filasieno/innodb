# Quick Start Guide: Literate Programming

Get started with literate programming in 5 minutes.

---

## What is Literate Programming?

Instead of writing code with comments, you write an essay that *contains* code. The documentation and implementation live together in one beautiful, readable document.

**Traditional Code:**
```python
# Fibonacci function
def fibonacci(n):
    # Handle base cases
    if n <= 1:
        return n
    # Recursive case
    return fibonacci(n-1) + fibonacci(n-2)
```

**Literate Program:**
```markdown
# Understanding Fibonacci

The Fibonacci sequence starts with 0 and 1, and each 
subsequent number is the sum of the two preceding ones.

```python ⟨ fibonacci function ⟩
def fibonacci(n):
    ⟨ handle base cases ⟩
    ⟨ recursive case ⟩
```

Let's handle the simple cases first:

```python ⟨ handle base cases ⟩
if n <= 1:
    return n
```

For other values, we use the recurrence relation:

```python ⟨ recursive case ⟩
return fibonacci(n-1) + fibonacci(n-2)
```
```

---

## Installation

### Using pip (when available)
```bash
pip install literate-programming
```

### From source
```bash
git clone https://github.com/xinnodb/literate-programming
cd literate-programming
pip install -e .
```

### Verify installation
```bash
literate --version
```

---

## Your First Literate Program

### Step 1: Create a file

Create `hello.lit.md`:

````markdown
# Hello, World!

This is my first literate program. Let's write the classic
"Hello, World!" in Python.

## The Complete Program

The program is simple - just print a greeting:

```python ⟨ * ⟩
#!/usr/bin/env python3
"""⟨ docstring ⟩"""

⟨ main function ⟩

if __name__ == '__main__':
    main()
```

## Documentation

```python ⟨ docstring ⟩
Hello World Program

This is my first literate program!
```

## Implementation

The main function prints our greeting:

```python ⟨ main function ⟩
def main():
    print("Hello, World!")
    print("This was generated from a literate program!")
```
````

### Step 2: Extract the code (Tangle)

```bash
literate tangle hello.lit.md --chunk "*" -o hello.py
```

This creates `hello.py`:

```python
#!/usr/bin/env python3
"""
Hello World Program

This is my first literate program!
"""

def main():
    print("Hello, World!")
    print("This was generated from a literate program!")

if __name__ == '__main__':
    main()
```

### Step 3: Run it!

```bash
python hello.py
```

Output:
```
Hello, World!
This was generated from a literate program!
```

---

## Key Concepts

### 1. Chunks

A **chunk** is a named piece of code:

````markdown
```python ⟨ chunk name ⟩
code here
```
````

### 2. Chunk Definition (≡)

The first time you define a chunk:

````markdown
```python ⟨ config ⟩
HOST = "localhost"
PORT = 8080
```
````

The `≡` is implicit for the first definition.

### 3. Chunk Extension (+)

Add more to an existing chunk:

````markdown
```python ⟨ config ⟩+
TIMEOUT = 30
DEBUG = True
```
````

### 4. Chunk Reference

Use a chunk inside another:

````markdown
```python ⟨ main ⟩
⟨ imports ⟩
⟨ config ⟩
⟨ run server ⟩
```
````

### 5. Root Chunk

The top-level chunk (usually `⟨ * ⟩`) that contains everything:

````markdown
```python ⟨ * ⟩
⟨ imports ⟩
⟨ classes ⟩
⟨ main ⟩
```
````

---

## Common Patterns

### Pattern 1: Separate Concerns

Split code into logical pieces:

````markdown
# Web Server

```python ⟨ * ⟩
⟨ imports ⟩
⟨ middleware ⟩
⟨ routing ⟩
⟨ server ⟩
⟨ main ⟩
```

## Dependencies

```python ⟨ imports ⟩
from http.server import HTTPServer
import json
```

## Middleware Layer

```python ⟨ middleware ⟩
def logging_middleware(req, res):
    print(f"Request: {req['path']}")
    return True
```

... and so on
````

### Pattern 2: Incremental Building

Start simple, add complexity:

````markdown
# Algorithm Evolution

Version 1 - Simple but slow:

```python ⟨ sort ⟩
def sort(items):
    return sorted(items)  # Use built-in
```

Later, we optimize:

```python ⟨ sort ⟩+
def quicksort(items):
    if len(items) <= 1:
        return items
    pivot = items[0]
    less = [x for x in items[1:] if x < pivot]
    greater = [x for x in items[1:] if x >= pivot]
    return quicksort(less) + [pivot] + quicksort(greater)
```
````

### Pattern 3: Explain Complex Code

Break down difficult algorithms:

````markdown
# Binary Search

```python ⟨ binary search ⟩
def binary_search(arr, target):
    ⟨ initialize bounds ⟩
    
    while left <= right:
        ⟨ calculate midpoint ⟩
        ⟨ check if found ⟩
        ⟨ adjust bounds ⟩
    
    return -1  # Not found
```

We start with the entire array:

```python ⟨ initialize bounds ⟩
left = 0
right = len(arr) - 1
```

The midpoint helps us decide which half to search:

```python ⟨ calculate midpoint ⟩
mid = (left + right) // 2
```

... explain each piece
````

---

## Commands

### Tangle (Extract Code)

Extract a specific chunk:
```bash
literate tangle FILE --chunk "chunk name" -o output.py
```

Extract root chunk (everything):
```bash
literate tangle FILE --chunk "*" -o output.py
```

### Check (Validate)

Check for errors:
```bash
literate check FILE
```

Checks for:
- Undefined chunk references
- Circular dependencies
- Duplicate definitions

### List (Show Chunks)

See all chunks in a file:
```bash
literate list FILE
```

### Graph (Visualize)

Create a dependency graph:
```bash
literate graph FILE -o deps.dot
dot -Tpng deps.dot -o deps.png
```

### Weave (Generate Docs)

Generate PDF documentation:
```bash
literate weave FILE.typ -o output.pdf
```

---

## IDE Support

### VS Code

1. Install the "Literate Programming" extension
2. Open any `.lit.md` file
3. Enjoy:
   - Syntax highlighting in code chunks
   - Go to definition (`F12` on chunk names)
   - Find references
   - Auto-completion of chunk names
   - Error checking

### Neovim

Add to your config:
```lua
require('literate').setup()
```

### Other Editors

The files are plain Markdown - they work in any editor! LSP features require the literate-programming LSP server.

---

## Tips & Tricks

### 1. Start with the story

Write the documentation first, then fill in the code.

### 2. Use descriptive chunk names

Good: `⟨ validate user input ⟩`  
Bad: `⟨ check ⟩`

### 3. Keep chunks focused

Each chunk should do one thing. If it's long, break it into smaller chunks.

### 4. Use hierarchy for organization

```
⟨ server ⟩
  ⟨ server.init ⟩
  ⟨ server.methods ⟩
    ⟨ server.methods.start ⟩
    ⟨ server.methods.stop ⟩
```

### 5. Add metadata

````markdown
---lp-meta
title: My Project
language: python
author: Your Name
namespace: myproject
---
````

### 6. Use Markdown features

You're writing Markdown - use it!
- **Bold**, *italic*
- Lists
- Tables
- Links
- Images
- Code blocks (for examples that aren't chunks)

---

## Example Workflow

### 1. Plan your program

Write the high-level structure:

````markdown
# My Program

```python ⟨ * ⟩
⟨ setup ⟩
⟨ process data ⟩
⟨ output results ⟩
```
````

### 2. Fill in details

Expand each chunk:

````markdown
```python ⟨ setup ⟩
import sys
config = load_config()
data = load_data()
```
````

### 3. Test as you go

```bash
literate tangle program.lit.md --chunk "*" -o program.py
python program.py
```

### 4. Generate documentation

```bash
literate weave program.lit.md -o docs.pdf
```

### 5. Commit to git

```bash
git add program.lit.md
git commit -m "Add data processing module"
```

The `.lit.md` file gives clean diffs and merge-friendly text!

---

## Troubleshooting

### Error: "Undefined chunk: ⟨ foo ⟩"

You referenced a chunk that doesn't exist. Make sure you defined it:

````markdown
```python ⟨ foo ⟩
# define it here
```
````

### Error: "Circular dependency detected"

Chunk A references chunk B, which references A. This creates a loop.

Fix: Reorganize your chunks to remove the cycle.

### Warning: "Chunk defined but never used"

You defined a chunk but never referenced it. This might be intentional (root chunk) or a mistake (dead code).

### Chunks not expanding correctly

Check indentation - chunk content is indented to match the reference location.

---

## Next Steps

1. **Read the RFC**: `RFC-001-literate-programming-format.md` for complete specification
2. **Study examples**: `example-webserver.lit.md` for a real-world example
3. **Try it yourself**: Convert one of your existing programs to literate style
4. **Join the community**: Share your literate programs!

---

## Resources

- **RFC**: Full specification
- **Examples**: Real-world literate programs
- **Tutorial Videos**: Coming soon
- **Forum**: Ask questions and share tips

---

## Philosophy

> "Let us change our traditional attitude to the construction of programs: 
> Instead of imagining that our main task is to instruct a computer what to do, 
> let us concentrate rather on explaining to human beings what we want a 
> computer to do."
> 
> — Donald Knuth, "Literate Programming" (1984)

Write programs for humans first, computers second.

---

Happy Literate Programming! 🎉


