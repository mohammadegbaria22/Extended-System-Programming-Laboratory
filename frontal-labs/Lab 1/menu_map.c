#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define BUFFER_SIZE 128
 
char* map(char *array, int array_length, char (*f) (char)){
  char* mapped_array = (char*)(malloc(array_length*sizeof(char)));

  for(int i = 0 ; i < array_length ; i +=1){
    mapped_array[i] = f(array[i]);
  }
  return mapped_array;
}


char my_get(char c){
  return fgetc(stdin);  
}


char cprt(char c){
  if (0x20 <= c && c <= 0x7E){
    printf("%c\n",c);}

  else{
    printf(".\n");}
    
  return c;
}

char encrypt(char c) {
  if (c >= 0x21 && c <= 0x7F) {
      return c - 1;}

  return c;
}

char decrypt(char c) {
  if (c >= 0x1F && c <= 0x7E) {
      return c + 1;
  }
  return c;
}

char oprt(char c) {
  printf("%o\n", c);  
  return c;           
}



struct fun_desc {
  char *name;
  char (*fun)(char);
  };


 
int main(int argc, char **argv){

  struct fun_desc menu[] = { {"Get String",my_get}, {"Print String",cprt },{"Encrypt" ,encrypt},{"Decrypt" ,decrypt},{"Print in Octal" ,oprt} ,{ NULL, NULL } };
  char* carray = calloc(5,sizeof(char));
  int bound = (sizeof(menu) / sizeof(struct fun_desc)) -1 ;
  char input[BUFFER_SIZE];

  while (1) {
    printf("Select operation from the following menu:\n");
    for (int i = 0;i < bound ; i++){
      printf("%d: %s\n", i, menu[i].name);     
  }
  
  printf("Option: ");
  if (fgets(input, BUFFER_SIZE, stdin) == NULL){
    break;}

  int number;
  sscanf(input, "%d\n", &number);

  if (number < 0 || number >= bound){
    printf("\nNot within bounds\n");
    break;
  }
  printf("\nWithin bounds\n");
  char *new_array = map(carray, 5, menu[number].fun);
  free(carray);
  carray = new_array;
  
  printf("finish!!\n\n");

   

  }
  free(carray);
}
