#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TABLE_SIZE 1000
#define MAX_WORD_LEN 100
#define MAX_MATCHES 100

// Hash Table Node (Singly Linked List)
typedef struct HashNode {
  char word[MAX_WORD_LEN];
  int frequency;
  struct HashNode *next;
} HashNode;

// Binary Search Tree Node
typedef struct TreeNode {
  char word[MAX_WORD_LEN];
  int frequency;
  struct TreeNode *left;
  struct TreeNode *right;
} TreeNode;

// Array Structure for Sorting
typedef struct Match {
  char word[MAX_WORD_LEN];
  int frequency;
} Match;

HashNode *hashTable[TABLE_SIZE] = {NULL}; // Array of pointers
TreeNode *bstRoot = NULL;                 // Root of the BST
Match matchArray[MAX_MATCHES]; // Temporary array for auto-complete results
int matchCount = 0;            // Tracks number of matches

// Hash Function
int hashFunction(char *word) {
  int sum = 0;
  for (int i = 0; word[i] != '\0'; i++) {
    sum += tolower(word[i]);
  }
  return sum % TABLE_SIZE;
}

// Hash Table Insertion
void insertHash(char *word) {
  // Lower Case all the words
  char lowerWord[MAX_WORD_LEN];
  int i;
  for (i = 0; word[i] != '\0'; i++) {
      lowerWord[i] = tolower(word[i]);
  }
  lowerWord[i] = '\0';

  int index = hashFunction(lowerWord);

  // Traverse the singly linked list at that index
  HashNode *current = hashTable[index];
  while (current != NULL) {
    if (strcmp(current->word, lowerWord) == 0) {
      current->frequency++; // word found then increment frequency
      return;
    }
    current = current->next;
  }

  // If word is not found, dynamically allocate a new node(Collision Resolution)
  HashNode *newNode = (HashNode *)malloc(sizeof(HashNode));
  strcpy(newNode->word, lowerWord);
  newNode->frequency = 1;

  newNode->next = hashTable[index]; // look at this again
  hashTable[index] = newNode;
}

// Create a New BST Node
TreeNode *createTreeNode(char *word, int frequency) {
  TreeNode *newNode = (TreeNode *)malloc(sizeof(TreeNode));
  strcpy(newNode->word, word);
  newNode->frequency = frequency;
  newNode->left = NULL;
  newNode->right = NULL;
  return newNode;
}

// BST insertion
TreeNode *insertBST(TreeNode *root, char *word, int frequency) {
  if (root == NULL) {
    return createTreeNode(word, frequency);
  }

  // lexicographically comparing the words to maintain the order
  int cmp = strcmp(word, root->word);

  // if word is smaller, go left
  if (cmp < 0) {
    root->left = insertBST(root->left, word, frequency);
  }
  // if word is larger, go right
  else if (cmp > 0) {
    root->right = insertBST(root->right, word, frequency);
  }

  // return root node pointer
  return root;
}

// Transfer Data from Hash Table to BST
void transferHashToBST() {
  for (int i = 0; i < TABLE_SIZE; i++) {
    HashNode *current = hashTable[i];
    while (current != NULL) {
      // Insert each unique word and its total frequency into the BST
      bstRoot = insertBST(bstRoot, current->word, current->frequency);
      current = current->next;
    }
  }
}

// Prefix Search in BST
void searchPrefixBST(TreeNode *root, char *prefix) {
  if (root == NULL || matchCount >= MAX_MATCHES) {
    return;
  }

  // Compare the prefix with the first letters of the current node's word
  int prefixLen = strlen(prefix);
  int cmp = strncmp(prefix, root->word, prefixLen);

  // If prefix is lexicographically smaller, the matches must be in the left
  // subtree
  if (cmp < 0) {
    searchPrefixBST(root->left, prefix);
  }
  // If prefix is larger, the matches must be in the right subtree
  else if (cmp > 0) {
    searchPrefixBST(root->right, prefix);
  }
  // If it's an exact match for the prefix(like professional,procastinate)
  else {
    // Since there could be matches on both sides (e.g., prefix "pro",
    // left could be "probe", root is "program", right is "project"), traverse
    // both.
    searchPrefixBST(root->left, prefix);

    // Add the current matched node to our array
    strcpy(matchArray[matchCount].word, root->word);
    matchArray[matchCount].frequency = root->frequency;
    matchCount++;

    searchPrefixBST(root->right, prefix);
  }
}

// Display Dictionary (In-Order Traversal)
void displayDictionary(TreeNode *root) {
  if (root != NULL) {
    displayDictionary(root->left);
    printf("Word: %-15s | Frequency: %d\n", root->word, root->frequency);
    displayDictionary(root->right);
  }
}

// Free Hash Table
void freeHashTable() {
  for (int i = 0; i < TABLE_SIZE; i++) {
    HashNode *current = hashTable[i];
    while (current != NULL) {
      HashNode *temp = current; // Store current node
      current = current->next;  // Move pointer to the next node
      free(temp);               // Free the temp node
    }
    hashTable[i] = NULL; // Reset bucket to NULL
  }
}
// Free BST Memory
void freeBST(TreeNode *root) {
  if (root == NULL) {
    return;
  }
  freeBST(root->left);
  freeBST(root->right);
  free(root);
}

// Quick Sort
void swap(Match *a, Match *b) {
  Match temp = *a;
  *a = *b;
  *b = temp;
}
int partition(Match arr[], int low, int high) {
  int pivot = arr[high].frequency; // Pivot is the frequency of the last element
  int i = (low - 1);

  for (int j = low; j <= high - 1; j++) {
    // Sort in DESCENDING order (highest frequency first)
    if (arr[j].frequency >= pivot) {
      i++;
      swap(&arr[i], &arr[j]);
    }
  }
  swap(&arr[i + 1], &arr[high]);
  return (i + 1);
}

void quickSort(Match arr[], int low, int high) {
  if (low < high) {
    int pi = partition(arr, low, high);

    quickSort(arr, low, pi - 1);
    quickSort(arr, pi + 1, high);
  }
}

int main() {
  int choice;
  char prefix[MAX_WORD_LEN];
  char filename[MAX_WORD_LEN];
  FILE *file;
  char buffer[MAX_WORD_LEN];
  int isTrained = 0; // Flag to check if data is loaded

  while (1) {
    printf("\n=== NLP Auto-Complete Engine ===\n");
    printf("1. Train Model from Text File\n");
    printf("2. Test Auto-Complete Prediction\n");
    printf("3. Display Dictionary\n");
    printf("4. Exit\n");
    printf("Enter your choice: ");

    // Handle invalid string inputs
    if (scanf("%d", &choice) != 1) {
      while (getchar() != '\n')
        ; // Clear input
      choice = -1;
    }

    switch (choice) {
    case 1:
//      file = fopen("demo.txt", "r");
       printf("Enter training filename (e.g., training_data.txt): ");
       scanf("%99s", filename);

       file = fopen(filename, "r");
      if (file == NULL) {
        printf("Error: Could not open %s.\n", filename);
      } else {
        // printf("Training AI model...\n");
        while (fscanf(file, "%99s", buffer) == 1) {
          insertHash(buffer);
        }
        fclose(file);

        transferHashToBST();
        isTrained = 1;
        printf("Training complete! Dictionary organized.\n");
      }
      break;

    case 2:
      if (!isTrained) {
        printf("Error: Please train the model (Option 1) first!\n");
        break;
      }

      printf("Enter a prefix to auto-complete: ");
      scanf("%99s", prefix);

      for (int i = 0; prefix[i]; i++) {
        prefix[i] = tolower(prefix[i]);
      }

      matchCount = 0;
      searchPrefixBST(bstRoot, prefix);

      if (matchCount == 0) {
        printf("No matches found for '%s'.\n", prefix);
      } else {
        quickSort(matchArray, 0, matchCount - 1);

        printf("\n--- Auto-Complete Suggestions ---\n");
        int limit = (matchCount < 3) ? matchCount : 3;
        for (int i = 0; i < limit; i++) {
          printf("%d. %s (Frequency: %d)\n", i + 1, matchArray[i].word,
                 matchArray[i].frequency);
        }
        printf("---------------------------------\n");
      }
      break;

    case 3:
      if (!isTrained) {
        printf("Error: Please train the model (Option 1) first!\n");
        break;
      }
      printf("\n--- Full Alphabetical Dictionary ---\n");
      displayDictionary(bstRoot);
      printf("------------------------------------\n");
      break;

    case 4:
      printf("Cleaning up memory...\n");
      freeHashTable();
      freeBST(bstRoot);
      printf("Memory completely freed. Exiting Engine. Goodbye!\n");
      return 0;

    default:
      printf("Invalid choice. Please enter 1, 2, 3, or 4.\n");
    }
  }
  return 0;
}