#ifndef VECTOR_H
#define VECTOR_H

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

//generic type-safe printing
#ifdef VEC_STDIO
    #include <stdio.h>
    #define PRINT(X) fprintf(stdout, \
        _Generic((X), \
            bool:               "%s", \
            char:               "%c", \
            signed char:        "%d", \
            unsigned char:      "%u", \
            short:              "%d", \
            unsigned short:     "%u", \
            int:                "%d", \
            unsigned int:       "%u", \
            long:               "%ld", \
            unsigned long:      "%lu", \
            long long:          "%lld", \
            unsigned long long: "%llu", \
            float:              "%f", \
            double:             "%lf", \
            char*:              "%s", \
            const char*:        "%s", \
            default:            "%p" \
        ), \
        _Generic((X), \
            bool:    (X) ? "true" : "false", \
            default: X \
        ) \
    )
#endif

//debug
#ifdef NDEBUG
   #define VEC_ASSERT(cond) ((void)0)
#else
   #include <stdio.h>
   #define VEC_ASSERT(cond) do { \
      if (!(cond)) { \
         fprintf(stderr, "Assertion failed: %s, file %s, line %d\n", #cond, __FILE__, __LINE__); \
         abort(); \
      } \
   } while(0)
#endif

#define BASE_CAP 4

typedef struct {
   void* buffer;
   size_t elem_size;
   size_t cap;
   size_t len;
} vector;

static inline vector* vec_create(size_t init_cap, size_t elem_size) {
   if (elem_size == 0) return NULL;

   vector* vec = (vector*)malloc(sizeof(vector));
   if (!vec) return NULL;
   
   vec->elem_size = elem_size;
   vec->cap = init_cap;
   vec->len = 0;
   
   if (init_cap > 0) {
      vec->buffer = malloc(init_cap * elem_size);
      if (!vec->buffer) {
         free(vec); 
         return NULL;
      }
   } else {
      vec->buffer = NULL;
   }

   return vec;
}

static inline void vec_free(vector* vec) {
   if (!vec) return;
   free(vec->buffer);
   free(vec);
}

static inline void vec_resize(vector* vec, size_t target_cap) {
   VEC_ASSERT(vec != NULL);
   if (target_cap < vec->len) target_cap = vec->len;

   if (target_cap == 0) {
      free(vec->buffer);
      vec->buffer = NULL;
      vec->cap = 0;
      return;
   }

   void* target_buffer = realloc(vec->buffer, target_cap * vec->elem_size);
   if (!target_buffer) return;
   vec->buffer = target_buffer;
   vec->cap = target_cap;
}

// Pre-allocate capacity to optimize bulk insertions 
static inline void vec_reserve(vector* vec, size_t new_cap) {
   VEC_ASSERT(vec != NULL);
   if (new_cap > vec->cap) {
      vec_resize(vec, new_cap);
   }
}

static inline void vec_push(vector* vec, void* elem) {
   VEC_ASSERT(vec != NULL);
   VEC_ASSERT(elem != NULL);

   if (vec->len >= vec->cap) {
      size_t target_cap = (vec->len == 0) ? BASE_CAP : 
                          (vec->cap > SIZE_MAX / 2) ? SIZE_MAX : vec->cap * 2;
      vec_resize(vec, target_cap);
      if (vec->len >= vec->cap) return;
   }
   void* target = (uint8_t*)vec->buffer + (vec->len * vec->elem_size);
   memcpy(target, elem, vec->elem_size);
   vec->len++;
}

static inline void vec_pop(vector* vec) {
   VEC_ASSERT(vec != NULL);
   if (vec->len > 0) {
      vec->len--;
   }
}

static inline void vec_fill(vector* vec, void* elem) {
   VEC_ASSERT(vec != NULL);
   VEC_ASSERT(elem != NULL);

   if (vec->len == 0) return;
   memcpy(vec->buffer, elem, vec->elem_size);
   size_t copied = 1;
   while (copied < vec->len) {
      size_t remaining = vec->len - copied;
      size_t chunk = (copied < remaining) ? copied : remaining;
      memcpy((uint8_t*)vec->buffer + (copied * vec->elem_size), 
             vec->buffer, chunk * vec->elem_size);
      copied += chunk;
   }
}

static inline void vec_set(vector* vec, void* elem, size_t i) {
   VEC_ASSERT(vec != NULL);
   VEC_ASSERT(elem != NULL);
   VEC_ASSERT(i < vec->len);
   
   memcpy((uint8_t*)vec->buffer + (i * vec->elem_size), elem, vec->elem_size);
}

static inline void* vec_get(vector* vec, size_t i) {
   VEC_ASSERT(vec != NULL);
   VEC_ASSERT(i < vec->len);
   
   return ((uint8_t*)vec->buffer + (i * vec->elem_size));
}

static inline void vec_shrink_to_fit(vector* vec) {
   if (!vec) return;
   vec_resize(vec, vec->len);
}

static inline void vec_clear(vector* vec) {
   if (!vec) return;
   vec->len = 0;
}

static inline void vec_insert(vector* vec, void* elem, size_t i) {
   VEC_ASSERT(vec != NULL);
   VEC_ASSERT(elem != NULL);
   VEC_ASSERT(i <= vec->len);

   if (vec->cap - vec->len < 1) {
      size_t target_cap = (vec->len == 0) ? BASE_CAP : 
                          (vec->cap > SIZE_MAX / 2) ? SIZE_MAX : vec->cap * 2;
      vec_resize(vec, target_cap);
   }
   memmove((uint8_t*)vec->buffer + ((i + 1) * vec->elem_size), 
           (uint8_t*)vec->buffer + (i * vec->elem_size), 
           (vec->len - i) * vec->elem_size);
           
   memcpy((uint8_t*)vec->buffer + (i * vec->elem_size), elem, vec->elem_size);
   vec->len++;
}

static inline void vec_remove(vector* vec, size_t i) {
   VEC_ASSERT(vec != NULL);
   VEC_ASSERT(vec->len > 0);
   VEC_ASSERT(i < vec->len);
   
   memmove((uint8_t*)vec->buffer + (i * vec->elem_size),
           (uint8_t*)vec->buffer + ((i + 1) * vec->elem_size), 
           (vec->len - 1 - i) * vec->elem_size);
         
   vec->len--;
}

static inline void* vec_front(vector* vec) {
   VEC_ASSERT(vec != NULL);
   return vec->buffer;
}

static inline void* vec_back(vector* vec) {
   VEC_ASSERT(vec != NULL);
   if (vec->len == 0) return NULL;
   return ((uint8_t*)vec->buffer + ((vec->len - 1) * vec->elem_size));
}

static inline ptrdiff_t vec_find(vector* vec, void* elem){
   VEC_ASSERT(vec != NULL);
   VEC_ASSERT(elem != NULL);
   for(size_t i = 0; i < vec->len; i++){
      if(memcmp(vec_get(vec, i), elem, vec->elem_size) == 0){
         return (ptrdiff_t)i;
      }
   }
   return -1;
}

static inline vector* vec_clone(vector* vec){
   VEC_ASSERT(vec != NULL);
   vector* new_vec = vec_create(vec->cap, vec->elem_size);
   if(!new_vec) return NULL;
   
   if(vec->len > 0){
      memcpy(new_vec->buffer, vec->buffer, vec->len * vec->elem_size);
      new_vec->len = vec->len;
   }
   return new_vec;
}

static inline void vec_swap(vector* vec, size_t i, size_t j) {
   VEC_ASSERT(vec != NULL);
   VEC_ASSERT(i < vec->len);
   VEC_ASSERT(j < vec->len);
   if (i == j) return;

   uint8_t temp[vec->elem_size];
   
   uint8_t* a = (uint8_t*)vec->buffer + (i * vec->elem_size);
   uint8_t* b = (uint8_t*)vec->buffer + (j * vec->elem_size);

   memcpy(temp, a, vec->elem_size);
   memcpy(a, b, vec->elem_size);
   memcpy(b, temp, vec->elem_size);
}

//STDIO printing functions
#ifdef VEC_STDIO

static inline void vec_print_at(vector* vec, size_t i) {
   if (!vec || i >= vec->len) return;
   void* ptr = vec_get(vec, i);
   
   if (vec->elem_size == sizeof(char)) {
       PRINT(*(char*)ptr);
   } else if (vec->elem_size == sizeof(short)) {
       PRINT(*(short*)ptr);
   } else if (vec->elem_size == sizeof(int)) {
       PRINT(*(int*)ptr);
   } else if (vec->elem_size == sizeof(long long)) {
       PRINT(*(long long*)ptr);
   } else if (vec->elem_size == sizeof(float)) {
       PRINT(*(float*)ptr);
   } else if (vec->elem_size == sizeof(double)) {
       PRINT(*(double*)ptr);
   } else if (vec->elem_size == sizeof(char*)) {
       PRINT(*(char**)ptr);
   } else {
       fprintf(stdout, "%p", ptr);
   }
}

static inline void vec_print(vector* vec) {
   if (!vec) {
       fprintf(stdout, "vector(NULL)\n");
       return;
   }
   fprintf(stdout, "[");
   for (size_t i = 0; i < vec->len; i++) {
       vec_print_at(vec, i);
       if (i + 1 < vec->len) {
           fprintf(stdout, ", ");
       }
   }
   fprintf(stdout, "]\n");
}

#endif /* VEC_STDIO */

#endif /* VECTOR_H */
