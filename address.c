#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "lab4.h"


Page_Entry* page_table;
TLB_Table tlb;
Main_Memory main_mem;
FILE* backing_store_fp;

// Implement get_page_from() and get_offset_from() functions below this line

int get_pahe_from(int page_number, int page_size) {
    if(page_number < 1 || page_size < 1) {
        printf(" Page number and page size must be greater than 0.\n");
        return -1;// Indicates an error
    }
    return (page_number - 1 ) * page_size;
}
int main()
    int page_number = 2;
    int page_size = 10;
    int start_index = get_page_from(page_number, page size);
    if(start_index != -1) {
        printf("Start index for page %d: %d\n", page_number, start_index);
    }
return 0;

}
// Implement the page table functions below this line

typedef struct {
    int frame_number;
    int valid; // 1 if the page is in memory, 0 otherwise
}   PageTableEntry;
PageTableEntry page_table[NUM_PAGES];

//Initialize the page table
void initialize_page_table(){
    for (int i = 0; i < NUM_PAGES; i++) {
        page_table[i].frame_number = -1; // -1 indicates that the page is not in memory 
        page_table[i].valid = 0;
    }
}
// function to load a page intro memory 
void load_page(int page_number, int frame_number){
    if(page_number < 0 || page_number >= NUM_PAGES) {
        printf("Invalid page number .\n");
        retun;
    }
    page_table[page_number].frame_number = frame_number;
    page_table[page_number].valid = 1;
    printf("Page %d loaded into frame %d.\n",page_number, frame_number);
}

int main(){
    initialize_page_table();
}
//Example usage
load_page(0,5); // Load page 0 into frame 5
load_page(1,10); // load page 1 into frame 10

int_frame = get_frame_number(0);
if(framw != -1){
    printf("Frame number for page 0: %d\n", frame);
}
    frame = get_frame_number(2); // This will cause a page fault 
    if(frame ! =-1){
        printf("Frame number for page 2: %d\n", frame);
    }
    return 0;
}

// Implement the TLB functions below this line



// Implement the Physical Memory functions below this line



// Implement the Backing Store functions below this line

