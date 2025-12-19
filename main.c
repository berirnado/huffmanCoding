#include <stdio.h>
#include <stdlib.h>

typedef struct Node {
    unsigned char byte;
    int freq;    
    struct Node *left;
    struct Node *right;
} Node;

typedef struct MinHeap {
    int size;
    int capacity;
    Node** array;
} MinHeap;

Node* createNode(unsigned char byte, int freq){
    Node* node = malloc(sizeof(Node));
    node->byte = byte;
    node->freq = freq;
    node->left = node->right = NULL;
    return node;
}

MinHeap* createMinHeap(int capacity){
    MinHeap* heap = malloc(sizeof(MinHeap));
    heap->size = 0;
    heap->capacity = capacity;
    heap->array = malloc(capacity * sizeof(Node*));
    return heap;
}

void swap(Node** a, Node** b){
    Node* temp = *a;
    *a = *b;
    *b = temp;
}

void minHeapify(MinHeap* heap, int i){
    int smallest = i;
    int left = 2 * i + 1;
    int right = 2 * i + 2;

    if(left < heap->size &&
       heap->array[left]->freq < heap->array[smallest]->freq)
        smallest = left;

    if(right < heap->size &&
       heap->array[right]->freq < heap->array[smallest]->freq)
        smallest = right;

    if(smallest != i){
        swap(&heap->array[smallest], &heap->array[i]);
        minHeapify(heap, smallest);
    }
}

void insertMinHeap(MinHeap* heap, Node* node){
    if(heap->size == heap->capacity) return;

    int i = heap->size++;
    heap->array[i] = node;

    while(i && heap->array[(i - 1) / 2]->freq > heap->array[i]->freq){
        swap(&heap->array[i], &heap->array[(i - 1) / 2]);
        i = (i - 1) / 2;
    }
}

Node* extractMin(MinHeap* heap){
    if(heap->size == 0)
        return NULL;

    Node* root = heap->array[0];
    heap->array[0] = heap->array[--heap->size];
    minHeapify(heap, 0);
    return root;
}

MinHeap* buildMinHeapFromArray(int freq[]){
    MinHeap* heap = createMinHeap(256);

    for(int i = 0; i < 256; i++){
        if(freq[i] > 0){
            insertMinHeap(heap, createNode((unsigned char)i, freq[i]));
        }
    }

    return heap;
}

void gerarArrayDeFrequencia(FILE* fptr, int* freq){
    int c;

    while((c = fgetc(fptr)) != EOF){ // cada linha do arquivo
        freq[(unsigned char)c]++;
    }
}

void main(){
    int freq[256] = {0}; // array utilizado pra armazenar a frequencia de cada byte

    FILE* fptr = fopen("palavras.txt", "r");

    if(fptr == NULL){
        printf("Erro ao abrir arquivo");
        exit(1);
    }

    gerarArrayDeFrequencia(fptr, freq);
    fclose(fptr);

    for(int i = 0; i < 256; ++i){
        printf("%d\n", freq[i]);
    }
}