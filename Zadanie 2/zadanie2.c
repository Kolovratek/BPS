#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

#define MAX_USERS 5
#define MAX_PASS 10
#define MAX_PASS_LEN 64
#define MAX_LINE_LEN 1024

char *database[MAX_USERS];
char *keys[MAX_USERS][MAX_PASS];

void read_database() {
  FILE *file = fopen("hesla.csv", "r");

  char line[MAX_LINE_LEN];
  int i = 0;
  while (fgets(line, MAX_LINE_LEN, file)) {
    char *name = strtok(line, ":");
    char *hash = strtok(NULL, ":");
    char *key_string = strtok(NULL, ":");

    database[i] = (char*) malloc((strlen(name) + strlen(hash) + 2) * sizeof(char));
    sprintf(database[i], "%s:%s", name, hash);

    int j = 0;
    char *key = strtok(key_string, ",");
    while (key != NULL) {
      keys[i][j] = (char*) malloc((strlen(key) + 1) * sizeof(char));
      strcpy(keys[i][j], key);
      if(i < (MAX_USERS - 1)){
        char *p = strchr(keys[i][j], '\n');
        if (p != NULL) {
          *p = '\0';
        }
      }
      key = strtok(NULL, ",");
      j++;
    }
    i++;
  }
  fclose(file);
}

void encrypt_password(char *password) {
  int length = strlen(password);
  int shift = strlen(password);
  for (int i = 0; i < length; i++) {
    if (password[i] >= 'a' && password[i] <= 'z') {
      password[i] = ((password[i] - 'a') + shift) % 26 + 'a';
    }
    else if (password[i] >= 'A' && password[i] <= 'Z') {
      password[i] = ((password[i] - 'A') + shift) % 26 + 'A';
    }
    else if (password[i] >= '0' && password[i] <= '9') {
      password[i] = ((password[i] - '0') + shift) % 10 + '0';
    }
  }
}

bool authenticate(char *username, char *password, char *key) {
  FILE *file = fopen("hesla.csv", "r+");
  int index = -1;
  for (int i = 0; i < MAX_USERS; i++) {
    if (database[i] != NULL && strstr(database[i], username) != NULL) {
      index = i;
      break;
    }
  }
  
  if (index == -1) {
    return -1;
  }

  if (strcmp(password, database[index] + strlen(username) + 1) != 0) {
    return -1;
  }
  
  bool key_found = false;
  for (int i = 0; i < 10; i++) {
    if (keys[index][i] != NULL && strcmp(keys[index][i], key) == 0) {
      key_found = true;
      free(keys[index][i]);
      keys[index][i] = NULL;
      break;
    }
  }

  if (!key_found) {
    return -1;
  }

  return 0;
}

void save_to_file(char *filename) {
  FILE *file = fopen(filename, "w");
  for (int i = 0; i < MAX_USERS; i++) {
    if (database[i] != NULL) {
      fprintf(file, "%s:%s", database[i], database[i] + strlen(database[i]) + 1);
      bool key_written = false;
      for (int j = 0; j < 10; j++) {
        if (keys[i][j] != NULL) {
          if (!key_written) {
            fprintf(file, "%s", keys[i][j]);
            key_written = true;
          } else {
            fprintf(file, ",%s", keys[i][j]);
          }
        }
      }
      if(i != (MAX_USERS-1)){
        fprintf(file, "\n");
      }
    }
  }
 fclose(file);
}

 

int main() {
  read_database();
  char username[256];
  char password[MAX_PASS_LEN];
  char otp[64];
  char *key_string = strtok(NULL, ":");

  printf("meno: ");
  scanf("%s", username);

  printf("heslo: ");
  scanf("%s", password);

  printf("overovaci kluc: ");
  scanf("%s", otp);

  encrypt_password(password);

  int result = authenticate(username, password, otp);

  if (result == 0) {
      printf("ok\n");
  } else {
      printf("chyba\n");
  }
  
  save_to_file("hesla.csv");

  return 0;
}