# c-dynamic_array

A small, header-only, type-erased dynamic array (vector) library for C.

`vector.h` provides a generic growable array backed by a `void*` buffer, so a
single set of functions works for any element type — you just tell it the
element size at creation time.

## Features

- **Header-only** — drop `vector.h` into your project, no build step.
- **Type-erased** — one `vector` type for `int`, `double`, structs, pointers, etc.
- **Amortized O(1) push** with geometric growth (starts at 4, doubles). *Most pushes are instant; resizes happen rarely and average out to constant time.*
- **Full API** — create/free, push/pop, insert/remove, get/set, front/back, find, clone, swap, fill, resize, reserve, shrink-to-fit.
- **Optional printing** via `_Generic` (define `VEC_STDIO` before including).
- **Assertions** in debug builds, compiled out when `NDEBUG` is defined.

## Quick start

```c
#define VEC_STDIO       // optional: enables vec_print / vec_print_at
#include "vector.h"

int main(void) {
    vector* v = vec_create(0, sizeof(int));
    if (!v) return 1;

    for (int i = 1; i <= 5; i++) {
        vec_push(v, &i);
    }

    vec_print(v);                       // [1, 2, 3, 4, 5]

    int nine = 9;
    vec_insert(v, &nine, 2);            // [1, 2, 9, 3, 4, 5]
    vec_remove(v, 0);                   // [2, 9, 3, 4, 5]

    int* first = (int*)vec_get(v, 0);
    printf("first = %d\n", *first);     // first = 2

    vec_free(v);
    return 0;
}
```

Compile with any C11 (or later) compiler:

```sh
cc -std=c11 -Wall -Wextra main.c -o demo
```

## API overview

### Lifecycle

| Function | Description |
|---|---|
| `vector* vec_create(size_t init_cap, size_t elem_size)` | Allocate a new vector. Pass `0` for `init_cap` to start empty. Returns `NULL` on failure or if `elem_size == 0`. |
| `void vec_free(vector* v)` | Free the buffer and the vector itself. |
| `vector* vec_clone(vector* v)` | Deep-copy the vector (buffer contents copied). |

### Capacity

| Function | Description |
|---|---|
| `void vec_resize(vector* v, size_t cap)` | Shrink or grow capacity. Never shrinks below `len`. |
| `void vec_reserve(vector* v, size_t cap)` | Grow capacity to at least `cap`; no-op if already large enough. |
| `void vec_shrink_to_fit(vector* v)` | Resize capacity down to exactly `len`. |

### Element access

| Function | Description |
|---|---|
| `void* vec_get(vector* v, size_t i)` | Pointer to element `i`. |
| `void vec_set(vector* v, void* elem, size_t i)` | Copy `elem` into slot `i`. |
| `void* vec_front(vector* v)` | Pointer to first element (or `NULL` if empty). |
| `void* vec_back(vector* v)` | Pointer to last element (or `NULL` if empty). |

### Modification

| Function | Description |
|---|---|
| `void vec_push(vector* v, void* elem)` | Append a copy of `elem`. |
| `void vec_pop(vector* v)` | Remove the last element (no-op if empty). |
| `void vec_insert(vector* v, void* elem, size_t i)` | Insert `elem` before index `i`. |
| `void vec_remove(vector* v, size_t i)` | Remove element at index `i`. |
| `void vec_swap(vector* v, size_t i, size_t j)` | Swap two elements. |
| `void vec_fill(vector* v, void* elem)` | Overwrite every slot with `elem`. |
| `void vec_clear(vector* v)` | Set length to 0 (capacity unchanged). |

### Search

| Function | Description |
|---|---|
| `ptrdiff_t vec_find(vector* v, void* elem)` | Index of the first `memcmp`-equal element, or `-1`. |

### Printing (requires `#define VEC_STDIO`)

| Function | Description |
|---|---|
| `void vec_print(vector* v)` | Print all elements, e.g. `[1, 2, 3]`. |
| `void vec_print_at(vector* v, size_t i)` | Print a single element. |

`vec_print` dispatches on `elem_size` for common scalar types (`char`, `short`,
`int`, `long long`, `float`, `double`, `char*`). For structs or any type it
doesn't recognize it falls back to printing a pointer.

## Configuration macros

| Macro | Effect |
|---|---|
| `VEC_STDIO` | Enables the printing functions. Include `<stdio.h>`. |
| `NDEBUG` | Disables `VEC_ASSERT` checks (define for release builds). |

## Notes 

- **Header-only but not single-translation-unit**: functions are `static inline`,
  so every `.c` file that includes `vector.h` gets its own copy. That's fine
  for this design; you just won't have a single shared symbol.
- **`vec_swap` uses a VLA** (`uint8_t temp[vec->elem_size]`). That requires
  C99 VLA support. It's fine on GCC/Clang/MSVC 2019+; if you care about
  portability to strict C11 targets, replace it with a small `malloc` or a
  fixed-size stack buffer plus a fallback path.
- **`vec_find` compares raw bytes** via `memcmp`. For structs that contain
  padding or pointers, two logically-equal values may not compare equal.
- **`vec_push` and friends take a `void*`** and copy `elem_size` bytes from it.
  Passing a temporary compound literal (`&(int){42}`) works fine.
- **No `bool` in the API** — `vec_get` returns `void*`, so cast to your element
  type. Type safety is the caller's job.

## License

MIT — see `LICENSE` for details.
