#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

#define COUNTER 10000

// Define semaphores
sem_t oddSem, evenSem;

// Node structure
typedef struct Node {
    int data;
    struct Node* next;
} Node;

// LinkedList structure
typedef struct {
    Node* head;
    unsigned int size;
} LinkedList;

// Function to initialize the linked list
void initList(LinkedList* list) {
    list->head = NULL;
    list->size = 0;
}

// Function to create a new node
Node* createNode(int value) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    if (!newNode) {
        printf("Memory allocation failed\n");
        exit(1);
    }
    newNode->data = value;
    newNode->next = NULL;
    return newNode;
}

// Function to push a value into the linked list
void push(LinkedList* list, int value) {
    Node* newNode = createNode(value);

    if (list->head == NULL) {
        list->head = newNode;
    } else {
        Node* temp = list->head;
        while (temp->next != NULL) {
            temp = temp->next;
        }
        temp->next = newNode;
    }

    if(value%2==0){
        printf("Even: %d\n", value);
        }else{
        printf("odd: %d\n", value);
    }
}

// Recursive function to clear the stack (delete all nodes)
void clearStackRecursive(Node* node) {
    if (node == NULL) {
        return;
    }
    clearStackRecursive(node->next);
    free(node);
}

// Function to clear the linked list
void clearStack(LinkedList* list) {
    clearStackRecursive(list->head);
    list->head = NULL;
    list->size = 0;
}

// Function to display the list
void display(LinkedList* list) {
    Node* temp = list->head;
    while (temp != NULL) {
        list->size++;
        printf("%d\n", temp->data);
        temp = temp->next;
    }
    printf("NULL\n");
}

// Function to add odd numbers
void* addOddNumbers(void* arg) {
    LinkedList* list = (LinkedList*)arg;
    for (int i = 1; i <= COUNTER; i += 2) {
        sem_wait(&oddSem);  // Wait for our turn

        push(list, i);

        sem_post(&evenSem);  // Give turn to even numbers
    }
    return NULL;
}

// Function to add even numbers
void* addEvenNumbers(void* arg) {
    LinkedList* list = (LinkedList*)arg;
    for (int i = 2; i <= COUNTER; i += 2) {
        sem_wait(&evenSem);  // Wait for our turn

        push(list, i);

        sem_post(&oddSem);  // Give turn to odd numbers
    }
    return NULL;
}

// Main function
int main() {
    LinkedList list;
    initList(&list);
    
    // Initialize semaphores
    sem_init(&oddSem, 0, 1);  // Odd thread starts first
    sem_init(&evenSem, 0, 0); // Even thread starts locked

    // Create two threads
    pthread_t evenThread, oddThread;
    pthread_create(&oddThread, NULL, addOddNumbers, &list);
    pthread_create(&evenThread, NULL, addEvenNumbers, &list);

    // Wait for both threads to complete
    pthread_join(oddThread, NULL);
    pthread_join(evenThread, NULL);


    display(&list);

    printf("Finished iterating, size: %u\n", list.size);
    clearStack(&list);

    // Destroy semaphores
    sem_destroy(&oddSem);
    sem_destroy(&evenSem);

    return 0;
}
