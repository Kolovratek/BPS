#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_NAME_LEN 256
#define MAX_FILES 100

// STRUCTURES

typedef struct fs_entry {
  char name[MAX_NAME_LEN];
  char owner[MAX_NAME_LEN];
  int permissions;
  struct fs_entry *parent;
  bool is_dir;
  int size;
  void *data;
} FS_ENTRY;

// FUNCTIONS
FS_ENTRY *create_entry(char *name, char *owner, FS_ENTRY *parent) {
  FS_ENTRY *entry = (FS_ENTRY *)malloc(sizeof(FS_ENTRY));
  strcpy(entry->name, name);
  strcpy(entry->owner, owner);
  entry->permissions = 7;
  entry->parent = parent;
  entry->size = 0;
  return entry;
}

FS_ENTRY *create_dir(char *name, char *owner, FS_ENTRY *parent) {
  FS_ENTRY *entry = create_entry(name, owner, parent);
  entry->is_dir = true;
  return entry;
}

FS_ENTRY *create_file(char *name, char *owner, FS_ENTRY *parent) {
  FS_ENTRY *entry = create_entry(name, owner, parent);
  entry->is_dir = false;
  return entry;
}

FS_ENTRY **extract_list(FS_ENTRY *entry) { return (FS_ENTRY **)entry->data; }
bool can_read_entry(FS_ENTRY *entry) { return entry->permissions & 4; }
bool can_write_entry(FS_ENTRY *entry) { return entry->permissions & 2; }
bool can_execute_entry(FS_ENTRY *entry) { return entry->permissions & 1; }

void insert_entry_to_dir_entry(FS_ENTRY *entry, FS_ENTRY *inserted_entry) {
  FS_ENTRY **list = extract_list(entry);
  entry->size++;
  list = realloc(list, sizeof(FS_ENTRY **) * entry->size);
  list[entry->size - 1] = inserted_entry;
  entry->data = (void *)list;
  inserted_entry->parent = entry;
}

void remove_index_from_list(FS_ENTRY *entry, int i) {
  FS_ENTRY **list = extract_list(entry);
  int move_count = entry->size - (i + 1);
  list = memmove(list + i, list + i + 1, move_count * sizeof(FS_ENTRY *));
  entry->size--;
  list = realloc(list, entry->size);
  entry->data = (void *)list;
}

bool rm_child_at_index(FS_ENTRY *entry, int i) {
  if (!entry->is_dir || i - 1 >= entry->size) {
    printf("chyba\n");
    return false;
  }

  FS_ENTRY *removed_child = extract_list(entry)[i];
  if (!can_read_entry(removed_child)) {
    printf("chyba\n");
    return false;
  }

  if (removed_child->is_dir) {
    int skipped = 0;
    while (removed_child->size > skipped) {
      bool was_successful = rm_child_at_index(removed_child, skipped);
      if (!was_successful)
        skipped++;
    };
    if (skipped > 0) {
      return false;
    }
  }

  remove_index_from_list(entry, i);
  if (removed_child->data != NULL) {
    free(removed_child->data);
  }
  free(removed_child);
  return true;
}

bool is_there_file_with_the_same_name(FS_ENTRY *entry, char *name) {
  FS_ENTRY **list = (FS_ENTRY **)entry->data;
  for (int i = 0; i < entry->size; i++) {
    if (strcmp(list[i]->name, name) == 0) {
      return true;
    }
  }
  return false;
}

FS_ENTRY *parse_path_to_fs_entry(FS_ENTRY *actual, char *pch) {
  if (strcmp(pch, ".") == 0) {
    return actual;
  }
  if (strcmp(pch, "..") == 0) {
    if (actual->parent == NULL) {
      printf("chyba\n");
      return NULL;
    }
    return actual->parent;
  }
  if (!actual->is_dir) {
    printf("chyba\n");
    return NULL;
  }
  if (!can_read_entry(actual)) {
    printf("chyba\n");
    return NULL;
  }

  FS_ENTRY **list = extract_list(actual);
  bool found = false;
  for (int i = 0; i < actual->size; i++) {
    if (strcmp(list[i]->name, pch) == 0) {
      actual = list[i];
      found = true;
      return actual;
    }
  }
  if (!found) {
    printf("chyba\n");
    return NULL;
  }

  return actual;
}

int get_count_of_tokens(char *path) {
  char copy_path[MAX_NAME_LEN];
  strcpy(copy_path, path);
  int count = 0;
  char *pch = strtok(copy_path, "/");
  while (pch != NULL) {
    pch = strtok(NULL, "/");
    count++;
  }
  return count;
}

FS_ENTRY *root_dir;
FS_ENTRY *current_dir;
char *current_owner;

FS_ENTRY *get_entry_from_path(FS_ENTRY *start, char *path, bool return_parent) {
  char copy_path[MAX_NAME_LEN];
  strcpy(copy_path, path);

  FS_ENTRY *actual = start;
  char first_part[MAX_NAME_LEN];
  if (strlen(copy_path) == 0) {
    return actual;
  }
  if (strcmp(copy_path, ".") == 0) {
    return actual;
  }
  if (copy_path[0] == '/') {
    actual = root_dir;
  }
  int token_count = get_count_of_tokens(copy_path);
  int count = 0;
  char *pch = strtok(copy_path, "/");
  while (pch != NULL) {
    if (return_parent && token_count - 1 == count) {
      return actual;
    }
    actual = parse_path_to_fs_entry(actual, pch);
    pch = strtok(NULL, "/");
    count++;
  }
  return actual;
}

void extract_filename(char *path, char *filename) {
  char *last_separator = strrchr(path, '/');
  if (last_separator == NULL) {
    strcpy(filename, path);
  } else {
    strcpy(filename, last_separator + 1);
  }
}

void edit_path(char *path) {
  if (path[strlen(path) - 1] == '\n') {
    path[strlen(path) - 1] = '\0';
  }
  memmove(&path[0], &path[1], strlen(path));
}

int check_path(char *path) {
  char *ptr = strchr(path, ' ');
  if (strlen(path) == 0 || ptr != NULL) {
    printf("chyba\n");
    return 0;
  }
  return 1;
}

// COMMAND FUNCTIONS
void mkdir(char *path) {
  FS_ENTRY *entry = get_entry_from_path(current_dir, path, true);
  if (entry == NULL) {
    return;
  }
  if (!can_write_entry(entry)) {
    printf("chyba\n");
    return;
  }
  char filename[MAX_NAME_LEN];
  extract_filename(path, filename);
  if (is_there_file_with_the_same_name(entry, filename)) {
    printf("chyba\n");
    return;
  }
  FS_ENTRY *new = create_dir(filename, current_owner, entry);
  insert_entry_to_dir_entry(entry, new);
}

void touch(char *path) {
  FS_ENTRY *entry = get_entry_from_path(current_dir, path, true);
  if (entry == NULL) {
    return;
  }
  if (!can_write_entry(entry)) {
    printf("chyba\n");
    return;
  }
  char filename[MAX_NAME_LEN];
  extract_filename(path, filename);
  if (is_there_file_with_the_same_name(entry, filename)) {
    printf("chyba\n");
    return;
  }
  FS_ENTRY *new = create_file(filename, current_owner, entry);
  insert_entry_to_dir_entry(entry, new);
}

void rm(char *path) {
  FS_ENTRY *entry = get_entry_from_path(current_dir, path, true);
  if (entry == NULL) {
    return;
  }
  if (!can_write_entry(entry) || !can_execute_entry(entry)) {
    printf("chyba\n");
    return;
  }
  char filename[MAX_NAME_LEN];
  extract_filename(path, filename);
  FS_ENTRY **list = extract_list(entry);
  for (int i = 0; i < entry->size; i++) {
    if (strcmp(list[i]->name, filename) == 0) {
      rm_child_at_index(entry, i);
      return;
    }
  }
}

void cd(char *path) {
  char *ptr;
  ptr = strchr(path, ' ');
  if(ptr){
    printf("chyba\n");
    return;
  }
  if(strlen(path)==0){
    current_dir = root_dir;
    return;
  }
  FS_ENTRY *entry = get_entry_from_path(current_dir, path, false);
  if (entry == NULL) {
    return;
  }
  if (!can_execute_entry(entry)) {
    printf("chyba\n");
    return;
  }
  if (!entry->is_dir) {
    printf("chyba\n");
    return;
  }
  current_dir = entry;
}

void ls(char *path) {
  FS_ENTRY *entry = get_entry_from_path(current_dir, path, false);
  if (entry == NULL) {
    return;
  }
  if (!can_read_entry(entry)) {
    printf("chyba\n");
    return;
  }
  FS_ENTRY **list = extract_list(entry);
  if (entry->size == 0 && entry->parent != NULL) {
    char my_permission[4];
    if (entry->permissions == 0){
      strcpy(my_permission, "---");
    } else if (entry->permissions == 1){
      strcpy(my_permission, "--x");
    } else if(entry->permissions == 2){
      strcpy(my_permission, "-w-");
    } else if(entry->permissions == 3){
      strcpy(my_permission, "-wx");
    } else if(entry->permissions == 4){
      strcpy(my_permission, "r--");
    } else if(entry->permissions == 5){
      strcpy(my_permission, "r-x");
    } else if(entry->permissions == 6){
      strcpy(my_permission, "rw-");
    } else if(entry->permissions == 7){
      strcpy(my_permission, "rwx");
    }
    printf("%s %s %s\n", entry->name, entry->owner, my_permission);
    return;
  }
  for (int i = 0; i < entry->size; i++) {
    FS_ENTRY *entry = list[i];
    char my_permission[4];
    if (entry->permissions == 0){
      strcpy(my_permission, "---");
    } else if (entry->permissions == 1){
      strcpy(my_permission, "--x");
    } else if(entry->permissions == 2){
      strcpy(my_permission, "-w-");
    } else if(entry->permissions == 3){
      strcpy(my_permission, "-wx");
    } else if(entry->permissions == 4){
      strcpy(my_permission, "r--");
    } else if(entry->permissions == 5){
      strcpy(my_permission, "r-x");
    } else if(entry->permissions == 6){
      strcpy(my_permission, "rw-");
    } else if(entry->permissions == 7){
      strcpy(my_permission, "rwx");
    }
    printf("%s %s %s\n", entry->name, entry->owner, my_permission);
  }
  if(entry->size == 0 && entry->parent == NULL){
    printf("ziaden subor\n");
    return;
  }
}

void vypis(char *path) {
  FS_ENTRY *entry = get_entry_from_path(current_dir, path, false);
  if (entry == NULL) {
    return;
  }
  if (!can_read_entry(entry)) {
    printf("chyba\n");
    return;
  }else{
    printf("ok\n");
  }
}

void spusti(char *path) {
  FS_ENTRY *entry = get_entry_from_path(current_dir, path, false);
  if (entry == NULL) {
    return;
  }
  if (!can_execute_entry(entry)) {
    printf("chyba\n");
    return;
  }else{
    printf("ok\n");
  }
}

void zapis(char *path) {
  FS_ENTRY *entry = get_entry_from_path(current_dir, path, false);
  if (entry == NULL) {
    return;
  }
  if (!can_write_entry(entry)) {
    printf("chyba\n");
    return;
  }else{
    printf("ok\n");
  }
}

void chmod(int permissions, char *path) {
  FS_ENTRY *entry = get_entry_from_path(current_dir, path, false);
  if (entry == NULL) {
    return;
  }
  if (permissions > 7 || permissions < 0) {
    printf("chyba\n");
    return;
  }
  entry->permissions = permissions;
}

void chown(char *owner, char *path) {
  FS_ENTRY *entry = get_entry_from_path(current_dir, path, false);
  if (entry == NULL) {
    return;
  }
  strcpy(entry->owner, owner);
}

int main() {
  char command[256];
  root_dir = create_dir("", "root", NULL);
  current_dir = root_dir;
  current_owner = "milan";

  while (1) {
    printf("# ");
    scanf("%s", command);
    if (strcmp(command, "ls") == 0) {
      char path[256];
      fgets(path, sizeof(path), stdin);
      edit_path(path);
      ls(path);
    } else if (strcmp(command, "cd") == 0) {
      char path[256];
      fgets(path, sizeof(path), stdin);
      edit_path(path);
      cd(path);
    } else if (strcmp(command, "touch") == 0) {
      char path[256];
      fgets(path, sizeof(path), stdin);
      edit_path(path);
      check_path(path);
      touch(path);
    } else if (strcmp(command, "mkdir") == 0) {
      char path[256];
      fgets(path, sizeof(path), stdin);
      edit_path(path);
      check_path(path);
      mkdir(path);
    } else if (strcmp(command, "rm") == 0) {
      char path[256];
      fgets(path, sizeof(path), stdin);
      edit_path(path);
      check_path(path);
      rm(path);
    } else if (strcmp(command, "vypis") == 0) {
      char path[256];
      fgets(path, sizeof(path), stdin);
      edit_path(path);
      check_path(path);
      vypis(path);
    } else if (strcmp(command, "spusti") == 0) {
      char path[256];
      fgets(path, sizeof(path), stdin);
      edit_path(path);
      check_path(path);
      spusti(path);
    } else if (strcmp(command, "zapis") == 0) {
      char path[256];
      fgets(path, sizeof(path), stdin);
      edit_path(path);
      check_path(path);
      zapis(path);
    } else if (strcmp(command, "chmod") == 0) {
      char input[256];
      char filename[256];
      int permissions;
      fgets(input, sizeof(input), stdin);
      if (sscanf(input, "%d %s", &permissions, filename) != 2) {
        printf("chyba\n");
      }else{
        check_path(filename);
        chmod(permissions, filename);
      }
    } else if (strcmp(command, "chown") == 0) {
      char owner[256];
      char input[256];
      char filename[256];
      fgets(input, sizeof(input), stdin);
      if (sscanf(input, "%s %s", owner, filename) != 2) {
        printf("chyba\n");
      }else{
        check_path(filename);
        chown(owner, filename);
      }
    } else if (strcmp(command, "quit") == 0) {
      break;
    } else {
      printf("chyba\n");
    }
  }
  return 0;
}