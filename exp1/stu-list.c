#include <stdio.h>
#include <stdlib.h>

#define BUF_SIZE 256

typedef struct stuInfo {
  char stuName[10];
  int Age;
} Student;

typedef struct node {
  Student data;
  struct node* next;
} ListNode, *ListPtr;

ListPtr createListFromFile(char* path);
ListPtr insertList(ListPtr l, Student s);
void printList(ListPtr l);

ListPtr createListFromFile(char* path) {
  FILE* listFile = fopen(path, "r");
  if (listFile == NULL) {
    printf("Error: cannot open the file specified.\n");
    return NULL;
  }

  Student s;
  ListPtr listHead = NULL;
  // fscanf indicates reading the info successful or not (including EOF)
  while (fscanf(listFile, "%s%d", &(s.stuName), &(s.Age)) == 2) {
    listHead = insertList(listHead, s);
  }
  fclose(listFile);
  return listHead;
}

ListPtr insertList(ListPtr l, Student s) {
  ListPtr newHead = (ListPtr)malloc(sizeof(ListNode));
  newHead->next = l;
  newHead->data = s;
  return newHead;
}

void printList(ListPtr l) {
  ListPtr cursor = l;
  printf("[");
  while (cursor->next != NULL) {
    printf("(%s,%d), ", cursor->data.stuName, cursor->data.Age);
    cursor = cursor->next;
  }
  // reaches the end
  printf("(%s,%d)]\n", cursor->data.stuName, cursor->data.Age);
}

int main() {
  printf("Option and parameters: \n");
  printf("1. Create a new list from a file;\n");
  printf("2. Insert the info of an student to a list;\n");
  printf("3. Print a list or all.\n");

  ListPtr globalList = NULL;

  while (1) {
    printf("Your choice (1/2/3): ");
    char buf[BUF_SIZE];
    scanf("%s", buf);

    switch (buf[0]) {
      case '1':
        printf("Enter the file's path: ");
        scanf("%s", buf);
        globalList = createListFromFile(buf);
        if (globalList != NULL) {
          printf("Successfully Loaded.\n");
        }
        break;

      case '2':
        printf("Enter the student info to insert (name, age): ");
        int tmpage;
        if (scanf("%s%d", buf, &tmpage) == 2) {
          Student s;
          snprintf(s.stuName, sizeof(s.stuName), "%s", buf);
          s.Age = tmpage;
          globalList = insertList(globalList, s);
        }
        printf("Inserted.\n");
        break;

      case '3':
        printList(globalList);
        break;

      default:
        printf("Enter 1-3\n");
        break;
    }
  }
}