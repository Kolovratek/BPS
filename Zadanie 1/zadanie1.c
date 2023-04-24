#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_PASSWORD_LENGTH 500

void encrypt(char* password, char* input_file, char* output_file);
void decrypt(char* password, char* input_file, char* output_file);
int utf8_char_len(unsigned char c);

int main(int argc, char** argv) {
  char *input_file = NULL, *output_file = NULL, *password = NULL;
  int encrypt_mode = 0, decrypt_mode = 0;
  
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-s") == 0) {
      encrypt_mode = 1;
    }
    else if (strcmp(argv[i], "-d") == 0) {
      decrypt_mode = 1;
    }
    else if (strcmp(argv[i], "-p") == 0) {
      password = argv[i+1];
      i++;
    }
    else if (strcmp(argv[i], "-i") == 0) {
      input_file = argv[i+1];
      i++;
    }
    else if (strcmp(argv[i], "-o") == 0) {
      output_file = argv[i+1];
      i++;
    }
    else {
      printf("chyba\n");
      return 1;
    }
  }
  if (!encrypt_mode && !decrypt_mode) {
    printf("chyba\n");
    return 1;
  }
  if (password == NULL || input_file == NULL || output_file == NULL) {
    printf("chyba\n");
    return 1;
  }
  if (encrypt_mode) {
    encrypt(password, input_file, output_file);
  }
  else if (decrypt_mode) {
    decrypt(password, input_file, output_file);
  }
  return 0;
}

void encrypt(char* password, char* input_file, char* output_file) {
  FILE* input = fopen(input_file, "r");
  if (input == NULL) {
    printf("chyba\n");
    exit(1);
  }
  FILE* output = fopen(output_file, "w");

  unsigned char shift = 0;
  for (int i = 0; i < strlen(password); i++) {
    shift += password[i];
  }

  while (1) {
    unsigned char c = fgetc(input);
    if(feof(input)){
      break;
    }
    unsigned char shifted_c = (unsigned char)(c + shift);
    fputc(shifted_c, output);
  }
  fclose(input);
  fclose(output);
}

void decrypt(char* password, char* input_file, char* output_file) {
  FILE* input = fopen(input_file, "r");
  if (input == NULL) {
    printf("chyba\n");
    exit(1);
  }

  FILE* output = fopen(output_file, "w");

  unsigned char shift = 0;
  for (int i = 0; i < strlen(password); i++) {
    shift += password[i];
  }

  while (1) {
    unsigned char c = fgetc(input);
    if(feof(input)){
      break;
    }
    unsigned char unshifted_c = (unsigned char)(c - shift);
    fputc(unshifted_c, output);
  }

  fclose(input);
  fclose(output);
}