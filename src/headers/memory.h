#include "types.h"

RI_context context;

#define RI_realloc(__ptr, __size) written_RI_realloc(__ptr, __size, __func__, __LINE__, context)
#define RI_malloc(__size) written_RI_malloc(__size, __func__, __LINE__, context)
#define RI_calloc(__nmemb, __size) written_RI_calloc(__nmemb, __size, __func__, __LINE__, context)
#define RI_free(__ptr) written_RI_free(__ptr, __func__, __LINE__, context)

void* written_RI_realloc(void *__ptr, size_t __size, const char *caller, int line, RI_context context){
    void *pointer = realloc(__ptr, __size);

    if (context.memory.debug_memory) {
        int current_allocation_index = 0;
        int checking = 1;

        while (checking){
            if (!context.memory.allocation_table[current_allocation_index].reallocated_free && context.memory.allocation_table[current_allocation_index].pointer == __ptr){
                context.memory.allocation_table[current_allocation_index].reallocated_free = 1;
                
                checking = 0;
            }

            current_allocation_index++;
            
            if (current_allocation_index >= context.memory.allocation_search_limit){
                checking = 0;
            }
        }

        if (context.memory.current_allocation_index >= context.memory.allocation_table_length){
            context.memory.allocation_table_length += 50;
            context.memory.allocation_search_limit += 50;
            
            context.memory.allocation_table = RI_realloc(context.memory.allocation_table, sizeof(RI_memory_allocation) * context.memory.allocation_table_length);
        }

        context.memory.allocation_table[context.memory.current_allocation_index].allocated = 1;
        context.memory.allocation_table[context.memory.current_allocation_index].reallocated_alloc = 1;
        context.memory.allocation_table[context.memory.current_allocation_index].reallocated_free = 0;
        context.memory.allocation_table[context.memory.current_allocation_index].freed = 0;
        context.memory.allocation_table[context.memory.current_allocation_index].line = line;
        context.memory.allocation_table[context.memory.current_allocation_index].pointer = pointer;        
        context.memory.allocation_table[context.memory.current_allocation_index].size = __size;

        context.memory.current_allocation_index++;
    }

    return pointer;
}

void* written_RI_malloc(size_t __size, const char *caller, int line, RI_context context){
    void *pointer = malloc(__size);
    
    if (context.memory.debug_memory) {
        if (context.memory.current_allocation_index >= context.memory.allocation_table_length){
            context.memory.allocation_table_length += 50;
            context.memory.allocation_search_limit += 50;
            
            context.memory.allocation_table = RI_realloc(context.memory.allocation_table, sizeof(RI_memory_allocation) * context.memory.allocation_table_length);
        }

        context.memory.allocation_table[context.memory.current_allocation_index].allocated = 1;
        context.memory.allocation_table[context.memory.current_allocation_index].reallocated_free = 0;
        context.memory.allocation_table[context.memory.current_allocation_index].reallocated_alloc = 0;
        context.memory.allocation_table[context.memory.current_allocation_index].freed = 0;
        context.memory.allocation_table[context.memory.current_allocation_index].line = line;
        context.memory.allocation_table[context.memory.current_allocation_index].pointer = pointer;        
        context.memory.allocation_table[context.memory.current_allocation_index].size = __size;

        context.memory.current_allocation_index++;
    }

    return pointer;
}

void* written_RI_calloc(size_t __nmemb, size_t __size, const char *caller, int line, RI_context context){
    void *pointer = calloc(__nmemb, __size);
    
    if (context.memory.debug_memory) {
        if (context.memory.current_allocation_index >= context.memory.allocation_table_length){
            context.memory.allocation_table_length += 50;
            context.memory.allocation_search_limit += 50;
            
            context.memory.allocation_table = RI_realloc(context.memory.allocation_table, sizeof(RI_memory_allocation) * context.memory.allocation_table_length);
        }

        context.memory.allocation_table[context.memory.current_allocation_index].allocated = 1;
        context.memory.allocation_table[context.memory.current_allocation_index].reallocated_free = 0;
        context.memory.allocation_table[context.memory.current_allocation_index].reallocated_alloc = 0;
        context.memory.allocation_table[context.memory.current_allocation_index].freed = 0;
        context.memory.allocation_table[context.memory.current_allocation_index].line = line;
        context.memory.allocation_table[context.memory.current_allocation_index].pointer = pointer;        
        context.memory.allocation_table[context.memory.current_allocation_index].size = __size * __nmemb;
            
        context.memory.current_allocation_index++;
    }

    return pointer;
}

void written_RI_free(void *__ptr, const char *caller, int line){
    if (context.memory.debug_memory) {
        // size_t size = 0;
        
        int current_allocation_index = 0;
        int checking = 1;
        
        while (checking){
            if (!context.memory.allocation_table[current_allocation_index].reallocated_free && context.memory.allocation_table[current_allocation_index].pointer == __ptr){
                // i dont know what this does?
                // size = context.memory.allocation_table[current_allocation_index].size;
                context.memory.allocation_table[current_allocation_index].freed = 1;
                
                checking = 0;
            }
            
            current_allocation_index++;
            
            if (current_allocation_index >= context.memory.allocation_search_limit){
                checking = 0;
            }
        }
    }
 
    free(__ptr);
}