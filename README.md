# 🔗 Chain Reactor

A minimal **reactive programming library in C**.  
It allows you to create **input cells** and **computed cells** whose values automatically update when their dependencies change.  
Callbacks can be attached to cells to react to changes in real time.

---

## ✨ Features
- Create **input cells** with initial values
- Create **compute cells** that depend on 1 or 2 other cells
- Automatic **propagation of changes**
- Register/unregister **callbacks** triggered on value updates
- Clean memory management with `destroy_reactor`

---

## 📖 Example Usage
```c
#include "reactor.h"
#include <stdio.h>

int add(int a, int b) { return a + b; }

void on_change(void *obj, int new_val)
{
    (void)obj;
    printf("Cell changed: %d\n", new_val);
}

int main(void)
{
    reactor_t    *r = create_reactor();
    struct cell  *a = create_input_cell(r, 2);
    struct cell  *b = create_input_cell(r, 3);
    struct cell  *sum = create_compute2_cell(r, a, b, &add);

    add_callback(sum, NULL, on_change);

    set_cell_value(a, 10); // triggers callback
    set_cell_value(b, 20); // triggers callback

    printf("Final sum: %d\n", get_cell_value(sum));

    destroy_reactor(r);
    return (0);
}
