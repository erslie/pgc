#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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

int free_page_frames[PAGE_FRAMES];
int free_page_frame_count = 0;

int get_free_page_frame() {
    if (free_page_frame_count > 0) {
        return free_page_frames[--free_page_frame_count];
    }
    return -1;
}

int handle_page_fault(unsigned int virtual_address) {
    unsigned int page_number = virtual_address / PAGE_SIZE;

    printf("-> Page fault occuerd. Page loading...\n");

    int frame_num = get_free_page_frame();
    if (frame_num == -1) {
        printf("error: physical memory is full\n");
        return -1;
    }

    char filename[50];
    sprintf(filename, "virtual_page_%u.dat", page_number);

    if (access(filename, F_OK) != 0) {
        printf("-> Created new page file\n");
        FILE *fp = fopen(filename, "wb");
        if (fp) {
            unsigned char empty_page[PAGE_SIZE] = {0};
            fwrite(empty_page, 1, PAGE_SIZE, fp);
            fclose(fp);
        }
    }
    FILE *fp = fopen(filename, "rb");
    if (fp == NULL) {
        return -1;
    }
    fseek(fp, 0, SEEK_SET);
    fread(&physical_memory[frame_num * PAGE_SIZE], 1, PAGE_SIZE, fp);;
    fclose(fp);

    printf("-> Page load succeeded\n");
    
    page_table[page_number].present = 1;
    page_table[page_number].page_frame_number = frame_num;

    return 0;
}


unsigned char* translate_address(unsigned int virtual_address) {

    unsigned int page_number = virtual_address / PAGE_SIZE;
    unsigned int offset = virtual_address % PAGE_SIZE;

    if (page_number >= VIRTUAL_PAGES) {
        printf("error: Virtual address:%d is out of range\n", virtual_address);
        return NULL;
    }

    if (page_table[page_number].present == 0) {
        if (handle_page_fault(virtual_address) != 0) {
            return NULL;
        }
    }

    unsigned int physical_address = (page_table[page_number].page_frame_number * PAGE_SIZE) + offset;
    if (physical_address >= PHYSICAL_MEMORY_SIZE) {
        printf("error: Physical address:%d is out of range\n", physical_address);
        return NULL;
    }

    return &physical_memory[physical_address];

}

void init_state() {
    for (int i = 0; i < PHYSICAL_MEMORY_SIZE; i++) {
        physical_memory[i] = 0;
    }
    for (int i = 0; i < VIRTUAL_PAGES; i++) {
        page_table[i].present = 0;
        page_table[i].page_frame_number = -1;
    }
}

void init_free_page_frame() {
    for (int i = 0; i < PAGE_FRAMES; i++) {
        free_page_frames[i] = i;
    }
    free_page_frame_count = PAGE_FRAMES;
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

    init_state();
 
    load_physical_memory();

    load_page_table();

    init_free_page_frame();

    for (int i = 0; i < VIRTUAL_PAGES; i++) {
        if (page_table[i].present == 1) {
            free_page_frame_count--;
        }
    }

    if (argc < 3) {
        printf("usage: ./pgc <read/write> <virtual_address> [value:0-255]\n");
        return 1;
    }

    char *operation = argv[1];
    unsigned int virtual_address = atoi(argv[2]);
    unsigned char *physical_ptr;
    


    if (strcmp(operation, "read") == 0) {
        //----- read -----
        physical_ptr = translate_address(virtual_address);
        if (physical_ptr == NULL) {
            return 1;
        }
        printf("reading data from virtual address:%u\n", virtual_address);
        printf("-> physical address:%ld, value: %d\n",physical_ptr - physical_memory, *physical_ptr);

    } else if (strcmp(operation, "write") == 0) {
        //------ write -----
        if (argc < 4) {
            printf("Please specify a value for writing\n");
            return 1;
        }
        int value = atoi(argv[3]);
        physical_ptr = translate_address(virtual_address);
        if (physical_ptr == NULL) {
            return 1;
        }
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

