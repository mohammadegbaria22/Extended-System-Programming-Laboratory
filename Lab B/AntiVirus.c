#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAX_INPUT 256
#define BUFFER_SIZE 10000
int is_big_endian = 0;


//Virus struct
typedef struct virus {
    unsigned short SigSize;
    unsigned char* VirusName;
    unsigned char* Sig;
} virus;



//from binary to hexadecimal printing
void PrintHex(unsigned char *buffer, size_t length) {
    for (size_t i = 0; i < length; i++) {
        printf("%02X", buffer[i]);
        if (i < length - 1) printf(" ");
    }
    printf("\n");
}


virus* readVirus(FILE* file){
    virus* v = malloc(sizeof(virus));
    if (!v) return NULL;

      // Read the signature size (2 bytes)
      if (fread(&v->SigSize, sizeof(unsigned short), 1, file) != 1) {
        free(v);
        return NULL;
    }


      // Handle endian conversion
      if (is_big_endian) {
        v->SigSize = (v->SigSize >> 8) | (v->SigSize << 8);
    }


    // Allocate and read the virus name (16 bytes)
    v->VirusName = malloc(16);
    if (!v->VirusName || fread(v->VirusName, 1, 16, file) != 16) {
        free(v->VirusName);
        free(v);
        return NULL;
    }

     // Allocate and read the signature
     v->Sig = malloc(v->SigSize);
     if (!v->Sig || fread(v->Sig, 1, v->SigSize, file) != v->SigSize) {
         free(v->Sig);
         free(v->VirusName);
         free(v);
         return NULL;
     }
    return v;
}



void printVirus(virus* v, FILE* output) {
    if (!v || !output) return;

    // Print name and size
    fprintf(output, "Virus name: %s\n", v->VirusName);
    fprintf(output, "Virus size: %u\n", v->SigSize);

    // Print signature in hex
    fprintf(output, "Signature: ");
    for (int i = 0; i < v->SigSize; i++) {
        fprintf(output, "%02X", v->Sig[i]);
        if (i < v->SigSize - 1)
            fprintf(output, " ");
    }
    fprintf(output, "\n\n");
}





//////////////////////////////////////////////////////////////////////////////////////////////////////

//link struct
typedef struct link {
    struct link* nextVirus;
    virus* vir;
} link;


void list_print(link *virus_list, FILE* file) {
    link *current = virus_list;
    while (current != NULL) {
        printVirus(current->vir, file);
        current = current->nextVirus;
    }
}


//Adds the new virus to the end of the list
link* list_append(link* virus_list, virus* data) {
    link* new_link = malloc(sizeof(link));
    new_link->vir = data;
    new_link->nextVirus = NULL;

    if (virus_list == NULL) {
        return new_link; // new list starts here
    }

    link* current = virus_list;
    while (current->nextVirus != NULL) {
        current = current->nextVirus;
    }
    current->nextVirus = new_link;
    return virus_list;
}



void list_free(link *virus_list) {
    while (virus_list != NULL) {
        link* next = virus_list->nextVirus;
        free(virus_list->vir->Sig);
        free(virus_list->vir->VirusName);
        free(virus_list->vir);
        free(virus_list);
        virus_list = next;
    }
}
////////////////////////////////////////////////////////////////////////////////////////////

void detect_virus(char *buffer, unsigned int size, link *virus_list) {
    link* cur = virus_list;
    while (cur != NULL) {
        virus* v = cur->vir;
        for (int i = 0; i <= size - v->SigSize; i+=1) {
            if (memcmp(buffer + i, v->Sig, v->SigSize) == 0) {
                printf("Virus detected!\n");
                printf("Starting byte location: %u\n", i);
                printf("Virus name: %s\n", v->VirusName);
                printf("Virus signature size: %u\n\n", v->SigSize);
            }
        }
        cur = cur->nextVirus;
    }
}


void neutralize_virus(char *fileName, int signatureOffset) {
    FILE *fp = fopen(fileName, "r+b");  // Open for read/write
    if (!fp) {
        perror("Error opening file for neutralization");
        return;
    }

    if (fseek(fp, signatureOffset, SEEK_SET) != 0) {
        perror("fseek failed");
        fclose(fp);
        return;
    }

    unsigned char ret = 0xC3;  // x86 RET instruction
    if (fwrite(&ret, 1, 1, fp) != 1) {
        perror("Failed to write RET");
    }

    fclose(fp);
}





int main(int argc, char *argv[]) {

    char input[MAX_INPUT];
    link* virus_list = NULL;

    while (1) {
        printf("1) Load signatures\n");
        printf("2) Print signatures\n");
        printf("3) Detect viruses\n");
        printf("4) Fix file\n");
        printf("5) Quit\n");

        printf("Option: ");
        if (fgets(input, MAX_INPUT, stdin) == NULL) {
            break;
        }

        int option;
        if (sscanf(input, "%d", &option) != 1) {
            printf("Invalid input\n");
            continue;
        }

        switch (option) {
            case 1: {
                // Load signatures
                char filename[256];
                printf("Enter signature file name: ");
                if (fgets(filename, sizeof(filename), stdin) == NULL)
                    break;
                filename[strcspn(filename, "\n")] = 0; // remove newline

                FILE* file = fopen(filename, "rb");
                if (!file) {
                    perror("Error opening file");
                    break;
                }

                // Free old list if exists
                list_free(virus_list);
                virus_list = NULL;

                // Read and check magic
                char magic[5] = {0};
                if (fread(magic, 1, 4, file) != 4 || (strcmp(magic, "VIRL") != 0 && strcmp(magic, "VIRB") != 0)) {
                    printf("Invalid magic number.\n");
                    fclose(file);
                    break;
                }
                is_big_endian = (strcmp(magic, "VIRB") == 0);

                // Read viruses
                virus* v;
                while ((v = readVirus(file)) != NULL) {
                    virus_list = list_append(virus_list, v);
                }

                fclose(file);
                break;
            }


                case 2:
                // Print signatures
                if (virus_list != NULL)
                    list_print(virus_list, stdout);
                break;


                case 3: {
                    if (argc < 2) {
                        printf("No suspect file provided as command-line argument.\n");
                        break;
                    }
    
                    FILE *suspect = fopen(argv[1], "rb");
                    if (!suspect) {
                        perror("Error opening suspect file");
                        break;
                    }
    
                    char buffer[BUFFER_SIZE];
                    size_t size = fread(buffer, 1, sizeof(buffer), suspect);
                    fclose(suspect);
    
                    detect_virus(buffer, size, virus_list);
                    break;
                }

                case 4: {
                    if (argc < 2) {
                        printf("No suspect file provided.\n");
                        break;
                    }
                
                    FILE *suspect = fopen(argv[1], "rb");
                    if (!suspect) {
                        perror("Error opening suspect file");
                        break;
                    }
                
                    char buffer[BUFFER_SIZE];
                    size_t size = fread(buffer, 1, sizeof(buffer), suspect);
                    fclose(suspect);
                
                    // Loop through virus list and detect/neutralize
                    link* curr = virus_list;
                    while (curr != NULL) {
                        virus* v = curr->vir;
                        for (int i = 0; i <= size - v->SigSize; i++) {
                            if (memcmp(buffer + i, v->Sig, v->SigSize) == 0) {
                                printf("Neutralizing virus: %s at offset %d\n", v->VirusName, i);
                                neutralize_virus(argv[1], i);
                            }
                        }
                        curr = curr->nextVirus;
                    }
                
                    break;
                }

            case 5:
                // Quit
                list_free(virus_list);
                return 0;

            default:
                printf("Invalid option\n");
                break;    

        }
    }
   return 0;
}  

