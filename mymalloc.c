#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>     // For sbrk()
#include <stdbool.h>    // For bool type

#define HEAP_SIZE 1024

// This is the header structure that describes each memory block.
struct Header {
    size_t size;         // Size of the data part of the block
    bool is_allocated;   // True if the block is in use, false otherwise
};

// This is our global pointer to the start of the managed memory heap.
struct Header *heap_ptr = NULL;

// This function initializes our heap by getting memory from the OS.
void heap_init() {
    // We only initialize the heap once.
    if (heap_ptr == NULL) {
        // Use sbrk() to request a large chunk of memory from the kernel.
        heap_ptr = (struct Header *)sbrk(HEAP_SIZE);
        
        // sbrk() returns (void*)-1 on failure.
        if (heap_ptr == (void *)-1) {
            perror("Memory allocation failed");
            return;
        }

        // Initialize the first (and only) header for the entire heap.
        // The size is the total heap size minus the header itself.
        heap_ptr->size = HEAP_SIZE - sizeof(struct Header);
        heap_ptr->is_allocated = false;
    }
}
//---------------------------------------------------------------------------------------------------
// Our custom malloc function.
void *mymalloc(size_t bytes) {
    // Initialize the heap if it hasn't been already.
    if (heap_ptr == NULL) {
        heap_init();
    }

    struct Header *current_ptr = heap_ptr;

    // Loop through the blocks to find a suitable one.
    // The loop continues until the current_ptr is beyond our managed heap.
    while ((void *)current_ptr < (void *)heap_ptr + HEAP_SIZE) {

        // Check if the current block is free AND big enough.
        if (current_ptr->is_allocated == false && current_ptr->size >= bytes) {
            
            // This is the correct way to handle splitting a large block.
            // We only split if the remaining memory is large enough for a new header and data.
            if (current_ptr->size > (bytes + sizeof(struct Header))) {
                
                // Create a new header for the remaining free memory.
                // It is placed right after the newly allocated block.
                struct Header *next_ptr = (struct Header *)((char *)current_ptr + sizeof(struct Header) + bytes);
                
                // Calculate the size of the new free block.
                next_ptr->size = current_ptr->size - bytes - sizeof(struct Header);
                next_ptr->is_allocated = false;

                // Update the current block's size to the requested size.
                current_ptr->size = bytes;
            }

            // Mark the current block as allocated.
            current_ptr->is_allocated = true;
            
            // Return a pointer to the data part, which is just after the header.
            return (void *)((char *)current_ptr + sizeof(struct Header));
        }

        // If the current block is not suitable, move to the next one.
        // We add the size of the current block's data and its header.
        current_ptr = (struct Header *)((char *)current_ptr + current_ptr->size + sizeof(struct Header));
    }
    
    // No suitable block was found.
    return NULL;
}
//---------------------------------------------------------------------------------------------------
// Our custom free function.
void myfree(void *free_ptr) {
    if (free_ptr == NULL) {
        return;
    }

    // Move backward from the data pointer to find the header.
    // The cast to char* is essential for byte-by-byte arithmetic.
    struct Header *header_ptr = (struct Header *)((char *)free_ptr - sizeof(struct Header));

    // Mark the block as free.
    header_ptr->is_allocated = false;
}
//---------------------------------------------------------------------------------------------------
int main() {
    // The main function's job is to use our allocator, not to manage the heap directly.
    char *data1 = mymalloc(50);
    if (data1) {
        printf("Allocated 50 bytes at address: %p\n", (void *)data1);
    }

    char *data2 = mymalloc(100);
    if (data2) {
        printf("Allocated 100 bytes at address: %p\n", (void *)data2);
    }
    
    myfree(data1);

    char *data3 = mymalloc(40);
    if (data3) {
        printf("Allocated 40 bytes at address: %p\n", (void *)data3);
    }

    return 0;
}