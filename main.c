#define VECTOR_STDIO 
#include "vector.h"

int main(void){
   vector* vec = vec_create(2, sizeof(int32_t));
   vec->len = vec->cap;
   int32_t val = 0;
   vec_fill(vec, &val);
   for(size_t i = 0; i < vec->len; i++){
      fprintf(stdout, "%zu: %d\n", i, *(int32_t*)vec_get(vec, i));
   }
   val = 69;
   vec_insert(vec, &val, 5);
   for(size_t i = 0; i < vec->len; i++){
      fprintf(stdout, "%zu: %d\n", i, *(int32_t*)vec_get(vec, i));
   }
   vec_free(vec);
   return 0;
}
