#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>
#include <sys/stat.h>

#define _FILE_OFFSET_BITS 64
#define MAX_FILE_NAME 128
#define MEM_BUF_SIZE 10000

char debug_mode = 0;
char file_name[MAX_FILE_NAME];
int unit_size = 1;
unsigned char mem_buf[MEM_BUF_SIZE];
size_t mem_count = 0;
int display_mode = 0; // 0 = hex (default), 1 = dec

typedef struct {
    char *name;
    void (*func)();
} menu_option;

// ==== Function declarations ====
void toggle_debug_mode();
void set_file_name();
void set_unit_size();
void quit();
void not_implemented();
void load_into_memory();
void toggle_display_mode();
void memory_display();
void save_into_file();
void memory_modify();
// ==== Menu ====
menu_option menu[] = {
    {"Toggle Debug Mode", toggle_debug_mode},
    {"Set File Name", set_file_name},
    {"Set Unit Size", set_unit_size},
    {"Load Into Memory", load_into_memory},
    {"Toggle Display Mode", toggle_display_mode},
    {"Memory Display", memory_display},
    {"Save Into File", save_into_file},
    {"Memory Modify", memory_modify},
    {"Quit", quit},
    {NULL, NULL}
};





void toggle_debug_mode() {
    debug_mode = !debug_mode;
    fprintf(stderr, "Debug flag now %s\n", debug_mode ? "on" : "off");
}





void set_file_name() {
    printf("Enter file name: ");
    fgets(file_name, MAX_FILE_NAME, stdin);
    file_name[strcspn(file_name, "\n")] = 0; // remove newline
    if (debug_mode) fprintf(stderr, "Debug: file name set to '%s'\n", file_name);
}





void set_unit_size() {
    int size;
    printf("Enter unit size (1, 2, or 4): ");
    scanf("%d", &size);
    getchar(); // consume newline
    if (size == 1 || size == 2 || size == 4) {
        unit_size = size;
        if (debug_mode) fprintf(stderr, "Debug: set size to %d\n", unit_size);
    } else {
        printf("Invalid unit size.\n");
    }
}





void quit() {
    if (debug_mode) fprintf(stderr, "quitting\n");
    exit(0);
}





void not_implemented() {
    printf("Not implemented yet.\n");
}





void load_into_memory() {
    if (strlen(file_name) == 0) {
        printf("Error: file name is not set\n");
        return;
    }

    int fd = open(file_name, O_RDONLY);
    if (fd < 0) {
        perror("Error opening file");
        return;
    }

    char input[128];
    unsigned int location = 0;
    int length = 0;

    printf("Please enter <location> <length>\n");
    fgets(input, sizeof(input), stdin);
    sscanf(input, "%x %d", &location, &length);

    if (debug_mode) {
        fprintf(stderr, "Debug: file_name='%s'\n", file_name);
        fprintf(stderr, "Debug: location=0x%x, length=%d\n", location, length);
    }

    size_t bytes_to_read = length * unit_size;
    if (bytes_to_read > sizeof(mem_buf)) {
        printf("Error: requested read size exceeds memory buffer\n");
        close(fd);
        return;
    }

    //Moves the file pointer to location , This is where reading will start from in the file.
    if (lseek(fd, location, SEEK_SET) < 0) {
        perror("Error seeking in file");
        close(fd);
        return;
    }

    //Actually reads the data from the file into mem_buf.
    ssize_t bytes_read = read(fd, mem_buf, bytes_to_read);
    if (bytes_read < 0) {
        perror("Error reading file");
        close(fd);
        return;
    }

    //mem_count tells how many raw bytes are now in memory.
    mem_count = bytes_read;
    printf("Loaded %d units into memory\n", length);

    close(fd);
}





void toggle_display_mode() {
    if (display_mode == 0) {
        display_mode = 1;
        printf("Display flag now on, decimal representation\n");
    } else {
        display_mode = 0;
        printf("Display flag now off, hexadecimal representation\n");
    }
}





void memory_display() {
    char input[128];
    unsigned int addr = 0;
    int length = 0;

    printf("Enter address and length\n");
    fgets(input, sizeof(input), stdin);
    sscanf(input, "%x %d", &addr, &length);

    void *start;

    if (addr == 0) {
        if (length * unit_size > mem_count) {
            printf("Error: Requested display range is out of memory bounds\n");
            return;
        }
        start = mem_buf;
    } else {
        start = (void *)(uintptr_t)addr;
    }

    printf("%s\n", display_mode ? "Decimal" : "Hexadecimal");
    printf("============\n");


    for (int i = 0; i < length; i++) {
        int val = 0;
        memcpy(&val, (char *)start + i * unit_size, unit_size);

        static char* hex_formats[] = {"%#hhx ", "%#hx ", "No such unit", "%#x "};
        static char* dec_formats[] = {"%hhd ", "%hd ", "No such unit", "%d "};

        if (display_mode) {
            printf(dec_formats[unit_size - 1], val);
        } else {
            printf(hex_formats[unit_size - 1], val);
        }
    }

    printf("\n");
}





void save_into_file() {
    //If the user hasn't used option 1 to set a file name, don't proceed.
    if (strlen(file_name) == 0) {
        printf("Error: file name is not set\n");
        return;
    }


    printf("Please enter <source-address> <target-location> <length>\n");
    char input[128];
    unsigned int source_addr = 0;
    unsigned int target_offset = 0;
    int length = 0;

    fgets(input, sizeof(input), stdin);
    sscanf(input, "%x %x %d", &source_addr, &target_offset, &length);

    void* source;
    //If the user enters 0 for source-address, use mem_buf.
    //Otherwise, use a specific memory address 
    if (source_addr == 0) {
        source = mem_buf;
    } else {
        source = (void *)(uintptr_t)source_addr;
    }

    size_t bytes_to_write = length * unit_size;

    // Open file for reading/writing (not truncating)
    int fd = open(file_name, O_RDWR);
    if (fd < 0) {
        perror("Error opening file");
        return;
    }

    // Get file size
    struct stat st;
    if (fstat(fd, &st) < 0) {
        perror("Error getting file size");
        close(fd);
        return;
    }

    //If writing would go beyond the end of the file — abort safely.
    if (target_offset + bytes_to_write > st.st_size) {
        printf("Error: target location exceeds file size\n");
        close(fd);
        return;
    }

    //Move the file pointer to the correct offset so writing starts there.
    if (lseek(fd, target_offset, SEEK_SET) < 0) {
        perror("Error seeking in file");
        close(fd);
        return;
    }

    //Write length * unit_size bytes from memory to the file at the offset.
    ssize_t written = write(fd, source, bytes_to_write);
    if (written < 0) {
        perror("Error writing to file");
    }

    if (debug_mode) {
        fprintf(stderr, "Debug: wrote %d units (%zu bytes) from 0x%x to file '%s' at offset 0x%x\n",
                length, bytes_to_write, source_addr, file_name, target_offset);
    }

    close(fd);
}





void memory_modify() {
    char input[128];
    unsigned int location;
    unsigned int val;

    printf("Please enter <location> <val>\n");
    fgets(input, sizeof(input), stdin);
    sscanf(input, "%x %x", &location, &val);

    if (debug_mode) {
        fprintf(stderr, "Debug: location=0x%x, val=0x%x\n", location, val);
    }

    if (location + unit_size > mem_count) {
        printf("Error: location out of bounds\n");
        return;
    }

    // Write value into mem_buf
    memcpy(mem_buf + location, &val, unit_size);
}




void patch_mem_buf_with_count_digits() {
    unsigned char patch[56] = {
        0x55, 0x89, 0xe5, 0x83, 0xec, 0x10, 0xc7, 0x45, 0xfc, 0x00, 0x00, 0x00, 0x00,
        0xeb, 0x1c, 0x8b, 0x45, 0x08, 0x0f, 0xb6, 0x00, 0x3c, 0x2f, 0x7e, 0x0e, 0x8b,
        0x45, 0x08, 0x0f, 0xb6, 0x00, 0x3c, 0x39, 0x7f, 0x04, 0x83, 0x45, 0xfc, 0x01,
        0x83, 0x45, 0x08, 0x01, 0x8b, 0x45, 0x08, 0x0f, 0xb6, 0x00, 0x84, 0xc0, 0x75,
        0xda, 0x8b, 0x45, 0xfc, 0xc9, 0xc3
    };
    memcpy(mem_buf, patch, 56);
    mem_count = 56;
    fprintf(stderr, "Debug: mem_buf patched with count_digits (56 bytes)\n");
}





///////////////////////main/////////////////
int main() {
    while (1) {
        if (debug_mode) {
            fprintf(stderr, "unit_size: %d\n", unit_size);
            fprintf(stderr, "file_name: '%s'\n", file_name);
            fprintf(stderr, "mem_count: %u\n", mem_count);

        }
        printf("\n");

        printf("Choose action:\n");
        for (int i = 0; menu[i].name != NULL; i++) {
            printf("%d-%s\n", i, menu[i].name);
        }
        printf("\n");

        printf("Option: ");
        int choice;
        scanf("%d", &choice);
        getchar(); // consume newline

        if (choice >= 0 && menu[choice].func != NULL) {
            menu[choice].func();
        } else {
            printf("Invalid option\n");
        }
    }
}
