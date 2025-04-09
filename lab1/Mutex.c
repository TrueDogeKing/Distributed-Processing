#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define COUNTER 100

// Define mutex and condition variables
pthread_mutex_t lock;
pthread_cond_t cond;
int turn = 1;  // 1 = Odd thread's turn, 2 = Even thread's turn

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
        pthread_mutex_lock(&lock);  // Lock before modifying shared resource

        while (turn != 1) {  // Wait until it's odd thread's turn
            pthread_cond_wait(&cond, &lock);
        }

        push(list, i);
        turn = 2;  // Change turn to even thread

        pthread_cond_signal(&cond);  // Signal the even thread
        pthread_mutex_unlock(&lock); // Unlock mutex
    }
    return NULL;
}

// Function to add even numbers
void* addEvenNumbers(void* arg) {
    LinkedList* list = (LinkedList*)arg;
    for (int i = 2; i <= COUNTER; i += 2) {
        pthread_mutex_lock(&lock);  // Lock before modifying shared resource

        while (turn != 2) {  // Wait until it's even thread's turn
            pthread_cond_wait(&cond, &lock);
        }

        push(list, i);
        turn = 1;  // Change turn to odd thread

        pthread_cond_signal(&cond);  // Signal the odd thread
        pthread_mutex_unlock(&lock); // Unlock mutex
    }
    return NULL;
}

// Main function
int main() {
    LinkedList list;
    initList(&list);
    
    // Initialize mutex and condition variable
    pthread_mutex_init(&lock, NULL);
    pthread_cond_init(&cond, NULL);

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

    // Destroy mutex and condition variable
    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&cond);

    return 0;
}
