
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "lab4.h"


Page_Entry* page_table;
TLB_Table tlb;
Main_Memory main_mem;
FILE* backing_store_fp;


int get_page_from(int logical_address, int page_size) {
    if (page_size <= 0) {
        fprintf(stderr, "Page size must be greater than 0.\n");
        return -1;
    }
    return logical_address / page_size;
}
// Define PageTableEntry structure (if not already in lab4.h)
typedef struct {
    int frame_number;
    int valid; // 1 if the page is in memory, 0 otherwise
} PageTableEntry;

// Global page table declaration
PageTableEntry page_table[NUM_PAGES];

// Initialize the page table
void initialize_page_table() {
    for (int i = 0; i < NUM_PAGES; i++) {
        page_table[i].frame_number = -1; // -1 indicates that the page is not in memory 
        page_table[i].valid = 0;
    }
}

// Function to load a page into memory 
void load_page(int page_number, int frame_number) {
    if(page_number < 0 || page_number >= NUM_PAGES) {
        printf("Invalid page number.\n");
        return; // Corrected 'retun' to 'return'
    }
    page_table[page_number].frame_number = frame_number;
    page_table[page_number].valid = 1;
    printf("Page %d loaded into frame %d.\n", page_number, frame_number);
}

// Function to get frame number for a page
int get_frame_number(int page_number) {
    if(page_number < 0 || page_number >= NUM_PAGES) {
        printf("Invalid page number.\n");
        return -1;
    }
    
    if(page_table[page_number].valid) {
        return page_table[page_number].frame_number;
    }
    
    printf("Page fault: Page %d not in memory.\n", page_number);
    return -1;
}

int main() {
    // Initialize the page table
    initialize_page_table();

    // Example usage of page table functions
    load_page(0, 5);   // Load page 0 into frame 5
    load_page(1, 10);  // Load page 1 into frame 10

    // Get frame number for existing pages
    int frame = get_frame_number(0);
    if(frame != -1) {
        printf("Frame number for page 0: %d\n", frame);
    }

    // Attempt to get frame number for a page not in memory
    frame = get_frame_number(2);
    if(frame != -1) {
        printf("Frame number for page 2: %d\n", frame);
    }

    return 0;
}

// Implement the TLB functions below this line
void init_tlb() {
    tlb.entry = malloc(sizeof(TLB_Entry) * TLB_LEN);
    if (tlb.entry == NULL) {
        fprintf(stderr, "Failed to allocate TLB memory\n");
        exit(1);
    }
    tlb.length = TLB_LEN;
    tlb.head = 0;
    tlb.tail = 0;
    tlb.is_full = 0;

    // Initialize all entries as invalid
    for (int i = 0; i < tlb.length; i++) {
        tlb.entry[i].is_valid = 0;
    }
}

short tlb_lookup(unsigned char page) {
    for (int i = 0; i < tlb.length; i++) {
        if (tlb.entry[i].is_valid && tlb.entry[i].page == page) {
            // Assuming increment_tlb_hit() is defined elsewhere
            increment_tlb_hit();
            return tlb.entry[i].frame;
        }
    }
    return -1;
}
void update_tlb(unsinged char page, unsigned char frame ) {
    // if TLB is ful we use the character buffer replacement 
    if(tlb.is_full) {
        //Replace at head, then move head forward
        tlb.entry[tlb.head].page = page;
        tlb.entry[tlb.head].frame = frame;
        tlb.entry[tlb.head].is_valid = 1;

        tlb.head = (tlb.head + 1) % tlb.length;
    }else{
        tlb.entry[tlb.tail].page = page;
        tlb.entry[tlb.tail].frame = frame;
        tlb.entry[tlb.tail].is_valid = 1;

        tlb.tail = (tlb.tail + 1) % tlb.length;

        // we now check if TLB is full
        if(tlb.tail == 0) {
            tlb.is_full = 1;
        }
    }
}

void close_tlb() {
    if(tlb.entry) {
        free(tlb.entry);
        tlb.entry = NULL;
    }
}

// physical memory functions 
void init_physical_memory() {
    main_mem.mem = malloc(1 << (FRAME_ADDR_BITS + OFFSET_ADDR_BITS));
    if (main_mem.mem == NULL) {
        fprintf(stderr, "Failed to allocate physical memory\n");
        exit(1);
    }
    main_mem.next_available_frame = 0;
}

void free_physical_memory(){
    if(main_mem.mem){
        free(main_mem.mem);
        main_mem.mem = NULL;
    }
}

//Blocking store functions 
void init_backing_store(char*filename){
    backing_store_file = fopen(filename,"rb");
    if(!backing_store_file){
        perror("Error opening backing store"){
            exit(1);
        }
    }
}
int roll_in(char*filename) {
    if(fseek(backing_store_file,page*(1 << OFFSET_ADDR_BITS),SEEK_SET)! = 0) {
        perror("Error seeking in backing store");
        return -1
    }
}
if (fread(
    main_mem.mem + (frame * (1 << OFFSET_ADDR_BITS)), 
    1,                                                 
    1 << OFFSET_ADDR_BITS,                            
    backing_store_file                                 
) != (1 << OFFSET_ADDR_BITS)) {
    // Check if end of file or read error
    if (feof(backing_store_file)) {
        fprintf(stderr, "Unexpected end of file\n");
    } else {
        perror("Error reading from backing store");
    }
    return -1;
}
// Implement the Physical Memory functions below this line
extern Main_Memory main_mem;

// Initialize physical memory
void init_physical_memory() {
    // Allocate memory based on the total memory size 
    // (2^(FRAME_ADDR_BITS + OFFSET_ADDR_BITS))
    main_mem.mem = malloc(1 << (FRAME_ADDR_BITS + OFFSET_ADDR_BITS));
    
    if (main_mem.mem == NULL) {
        perror("Failed to allocate physical memory");
        exit(1);
    }
    
    // Reset next available frame to 0
    main_mem.next_available_frame = 0;
}

// Free physical memory
void free_physical_memory() {
    if (main_mem.mem) {
        free(main_mem.mem);
        main_mem.mem = NULL;
    }
    
    // Reset next available frame
    main_mem.next_available_frame = 0;
}

// Get value from a specific logical address
signed char get_value(int logical_address) {
    // Extract page number and offset
    unsigned char page = get_page_from(logical_address);
    unsigned char offset = get_offset_from(logical_address);
    
    // Look up frame in TLB first
    short frame = tlb_lookup(page);
    
    // If not in TLB, check page table
    if (frame == -1) {
        frame = page_table_lookup(page);
        
        // If not in page table, we have a page fault
        if (frame == -1) {
            // Increment page fault count
            increment_page_fault_count();
            
            frame = main_mem.next_available_frame;
            if (roll_in(page, frame) == -1) {
                perror("Failed to roll in page");
                return -1;
            }
            
            // Update page table and TLB
            update_page_table(page, frame);
            update_tlb(page, frame);
            
            // Increment next available frame (with wraparound)
            main_mem.next_available_frame = 
                (main_mem.next_available_frame + 1) % (1 << FRAME_ADDR_BITS);
        }
    }
    
    // Calculate physical address
    int physical_address = (frame << OFFSET_ADDR_BITS) | offset;
    
    // Return value from physical memory
    return main_mem.mem[physical_address];
}

// Implement the Backing Store functions below this line
FILE* backing_store_file = NULL;

// Initialize backing store 
void init_backing_store(char* filename) {
    // Open the backing store file in read-binary mode
    backing_store_file = fopen(filename, "rb");
    
    // Check if file opened successfully
    if (!backing_store_file) {
        perror("Error opening backing store");
        exit(1);
    }
}

// Roll in a page from backing store to physical memory
int roll_in(unsigned char page, unsigned char frame) {
    // Validate input parameters
    if (backing_store_file == NULL) {
        perror("Backing store not initialized");
        return -1;
    }

    // Calculate file offset for the page
    // Offset is page number * page size (determined by OFFSET_ADDR_BITS)
    long page_offset = page * (1 << OFFSET_ADDR_BITS);

    // Seek to the correct position in the backing store
    if (fseek(backing_store_file, page_offset, SEEK_SET) != 0) {
        perror("Error seeking in backing store");
        return -1;
    }

    // Calculate the destination in physical memory
    // Physical memory address = frame * page size
    char* dest = main_mem.mem + (frame * (1 << OFFSET_ADDR_BITS));

    // Read page-sized chunk from backing store to physical memory
    size_t bytes_read = fread(dest, 1, 1 << OFFSET_ADDR_BITS, backing_store_file);
    
    // Check if full page was read
    if (bytes_read != (1 << OFFSET_ADDR_BITS)) {
        perror("Error reading from backing store");
        return -1;
    }

    return 0; // Success
}

// Close backing store file
void close_backing_store() {
    if (backing_store_file) {
        fclose(backing_store_file);
        backing_store_file = NULL;
    }
}
