#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PHYSICAL_MEMORY_SIZE (256 * 1024) //256KB
#define PAGE_SIZE            (4 * 1024)
#define PAGE_FRAMES          (PHYSICAL_MEMORY_SIZE / PAGE_SIZE)
#define VIRTUAL_MEMORY_SIZE (1024 * 1024) //1MB
#define VIRTUAL_PAGES       (VIRTUAL_MEMORY_SIZE / PAGE_SIZE)

#define PAGE_TABLE_FILE "page_table.dat"
#define PHYSICAL_MEMORY_FILE "physical_memory.dat"

typedef struct {
    int present;
    int page_frame_number;
} page_table_entry;

page_table_entry page_table[VIRTUAL_PAGES];
unsigned char physical_memory[PHYSICAL_MEMORY_SIZE];

unsigned char* translate_address(unsigned int virtual_address) {

    unsigned int page_number = virtual_address / PAGE_SIZE;
    unsigned int offset = virtual_address % PAGE_SIZE;

    if (page_number >= VIRTUAL_PAGES) {
        printf("error: Virtual address:%d is out of range\n", virtual_address);
        return NULL;
    }

    page_table_entry pte = page_table[page_number];
    if (pte.present = 0) {
        printf("Page fault! virtual address:%d is not found\n", virtual_address);
        return NULL;
    }

    unsigned int physical_address = (pte.page_frame_number * PAGE_SIZE) + offset;
    if (physical_address >= PHYSICAL_MEMORY_SIZE) {
        printf("error: Physical address:%d is out of range\n", physical_address);
        return NULL;
    }

    return &physical_memory[physical_address];

}

void init() {

    //--- init page table ---

    for (int i = 0; i < VIRTUAL_PAGES; i++) {
        page_table[i].present = 0;
        page_table[i].page_frame_number = -1;
    }

    for (int i = 0; i < PHYSICAL_MEMORY_SIZE; i++) {
        physical_memory[i] = 0;
    }

}

void save_physical_memory() {
    FILE *fp = fopen(PHYSICAL_MEMORY_FILE, "wb");
    if (fp == NULL) {
        printf("Failed to read physical memory\n");
        return;
    }
    fwrite(physical_memory, sizeof(unsigned char), PHYSICAL_MEMORY_SIZE, fp);
    fclose(fp);
    printf("saved to physical memory\n");
}

void save_page_table() {
    FILE *fp = fopen(PAGE_TABLE_FILE, "wb");
    if (fp == NULL) {
        printf("Failed to read page table\n");
        return;
    }
    fwrite(page_table, sizeof(page_table_entry), VIRTUAL_PAGES, fp);
    fclose(fp);
    printf("saved to page table\n");
}

void load_physical_memory() {
    FILE *fp = fopen(PHYSICAL_MEMORY_FILE, "rb");
    if (fp == NULL) {
        for (int i = 0; i < PHYSICAL_MEMORY_SIZE; i++) {
            physical_memory[i] = 0;
        }
        return;
    } 
    fread(physical_memory, sizeof(unsigned char), PHYSICAL_MEMORY_SIZE, fp);
    fclose(fp); 
    printf("reading physical memory\n");
}

void load_page_table() {
    FILE *fp = fopen(PAGE_TABLE_FILE, "rb");
    if (fp == NULL) {
        for (int i = 0; i < VIRTUAL_PAGES; i++) {
            page_table[i].present = 0;
            page_table[i].page_frame_number = -1;
        }
        return;
    }
    fread(page_table, sizeof(page_table_entry), VIRTUAL_PAGES, fp);
    fclose(fp);
    printf("reading page table\n");
}


int main(int argc, char *argv[]) {

    init();
 
    load_physical_memory();

    load_page_table();

    page_table[0].present = 1;
    page_table[0].page_frame_number = 0;

    page_table[1].present = 1;
    page_table[1].page_frame_number = 1;

    page_table[10].present = 1;
    page_table[10].page_frame_number = 5;

    if (argc < 3) {
        printf("usage: ./pgc <read/write> <virtual_address> [value]\n");
        return 1;
    }

    char *operation = argv[1];
    unsigned int virtual_address = atoi(argv[2]);
    unsigned char *physical_ptr = translate_address(virtual_address);
    
    if (physical_ptr == NULL) {
        return 1;
    }

    if (strcmp(operation, "read") == 0) {
        //----- read -----
        printf("reading data from virtual address:%u\n", virtual_address);
        printf("-> physical address:%ld, value: %d\n",physical_ptr - physical_memory, *physical_ptr);

        return 0;

    } else if (strcmp(operation, "write") == 0) {
        //------ write -----
        if (argv < 4) {
            printf("Please specify a value for writing\n");
            return 1;
        }
        int value = atoi(argv[3]);
        *physical_ptr = (unsigned char)value;
        printf("-> virtual address:%u, write value:%d to ", virtual_address, value);

    } else {
        printf("Invalid operation");
        return 1;
    }

    save_physical_memory();
    save_page_table();

    return 0;

}


int test() {

    //--- paging test ---
    page_table[0].present = 1;
    page_table[0].page_frame_number = 0;

    page_table[1].present = 1;
    page_table[1].page_frame_number = 1;

    page_table[30].present = 1;
    page_table[30].page_frame_number = 5;

    //initial address
    unsigned char* ptr1 = translate_address(0);
    if (ptr1) {
        printf("print 'A' to virtual address 0\n");
        *ptr1 = 'A';
    }

    //4096 = 4KB
    unsigned char* ptr2 = translate_address(4096);
    if (ptr2) {
        printf("print 'B' to virtual address 4096\n");
        *ptr2 = 'B';
    }

    //122880 = 30 * 4096
    unsigned char* ptr3 = translate_address(122880);
    if (ptr3) {
        printf("print 'C' to virtual address 122880\n");
        *ptr3 = 'C';
    }

    //paging fault
    //8192 = virtual page 2
    unsigned char* ptr4 = translate_address(8192);
    if (ptr4 == NULL) {
        printf("page fault!\n");
    }

    //--- physical memory ---
    printf("\n --- physical memory data ---\n");
    printf("physical address 0    : %c\n", physical_memory[0]);
    printf("physical address 4096 : %c\n", physical_memory[4096]);
    printf("physical address 20480: %c\n", physical_memory[20480]);

    return 0;

}

