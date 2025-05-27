#include <stdbool.h>
#include <string.h>
#include <stdio.h>

int main(int argc, char *argv[]){

    bool debugMode = true;
    bool encoderP = false;
    bool encoderN = false;
    char *key = NULL;
    FILE *input = stdin;
    FILE *output = stdout;

    for (int i = 0; i < argc; i += 1){

        if (strncmp(argv[i], "+d", 2) == 0){
            debugMode = true;}

        if (strncmp(argv[i], "-d", 2) == 0){
            debugMode = false;}

        if (debugMode){ 
            fprintf(stderr, "argv[%d]=%s\n", i, argv[i]);}


        if (strncmp(argv[i], "+E", 2) == 0){
            encoderP = true;
            key = argv[i] + 2;}

        if (strncmp(argv[i], "-E", 2) == 0){
            encoderN = true;
            key = argv[i] + 2;}

        if (strncmp(argv[i], "-i", 2) == 0){
            input = fopen(argv[i] + 2, "r");

            if (input == NULL){
                fprintf(stderr, "Error: cannot open input file %s\n", argv[i] + 2);
                return 1;}
        }

        if (strncmp(argv[i], "-o", 2) == 0){
            output = fopen(argv[i] + 2, "w");
            
            if (output == NULL){
                fprintf(stderr, "Error: cannot open output file %s\n", argv[i] + 2);
                return 1;}
        }
    }

    char c;
    int key_index = 0;

    if (encoderN || encoderP){

        while (((c = fgetc(input))) != EOF){

            if (key[key_index] == '\0'){
                key_index = 0;}

            if (((c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9')) && key != NULL){

                int k = key[key_index] - '0'; // convert char to number

                if (encoderP){
                    if (c >= 'A' && c <= 'Z'){
                        c = 'A' + (c - 'A' + k) % 26;}

                    else if (c >= '0' && c <= '9'){
                        c = '0' + (c - '0' + k) % 10;}
                }

                if (encoderN){

                    if (c >= 'A' && c <= 'Z'){
                        c = 'A' + (c - 'A' - k + 26) % 26;}

                    else if (c >= '0' && c <= '9'){
                        c = '0' + (c - '0' - k + 10) % 10;}
                }
            }

            fputc(c, output); // always print character
            key_index += 1;
        }
    }

    fclose(input);
    fclose(output);
    return 0;
}
