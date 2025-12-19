#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

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

typedef struct {
    unsigned char byte; // byte em construção
    int bitCount;       // quantos bits já foram colocados
} BitBuffer;

typedef struct {
    unsigned char byte;
    int bitPos;   // posição atual (0 a 7)
} BitReader;

// funcao para retornar tamanho do arquivo (peguei do stackOverflow)
long getFileSize(FILE* fptr) {
    fseek(fptr, 0L, SEEK_END);
    long size = ftell(fptr);
    return size;
}

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

void gerarCodigosHuffman(Node* root, char* caminho, int depth, char** codigos){
    if(!root) return;

    // Se for folha
    if(!root->left && !root->right){
        caminho[depth] = '\0';
        codigos[root->byte] = strdup(caminho);
        return;
    }

    // Esquerda
    caminho[depth] = '0';
    gerarCodigosHuffman(root->left, caminho, depth + 1, codigos);

    // Direita
    caminho[depth] = '1';
    gerarCodigosHuffman(root->right, caminho, depth + 1, codigos);
}

void inicializaBuffer(BitBuffer* buf){
    buf->byte = 0;
    buf->bitCount = 0;
}

void incializaLeitorBits(BitReader* br){
    br->byte = 0;
    br->bitPos = 8; 
}

void escreveBitOutput(FILE* out, BitBuffer* buf, bool bit){
    buf->byte <<= 1;
    if(bit){
        buf->byte |= 1;
    }

    buf->bitCount++;

    if(buf->bitCount == 8){
        fwrite(&buf->byte, 1, 1, out);
        buf->byte = 0;
        buf->bitCount = 0;
    }
}

int readBitCodificado(FILE* in, BitReader* br){
    if(br->bitPos == 8){
        if(fread(&br->byte, 1, 1, in) != 1)
            return -1; // fim do arquivo
        br->bitPos = 0;
    }

    int bit = (br->byte & (1 << (7 - br->bitPos))) != 0;
    br->bitPos++;

    return bit;
}

// Escreve o codigo em binario no arquivo de saída
void escreveCodigoNoArquivo(FILE* out, BitBuffer* buf, const char* code){
    for(int i = 0; code[i] != '\0'; i++){
        escreveBitOutput(out, buf, code[i] == '1');
    }
}

// Funcao que garante que o ultimo byte vai ter um padding de 0s para fechar ele caso ele nao seja um byte completo
void preencheUltimoByte(FILE* out, BitBuffer* buf){
    if(buf->bitCount > 0){
        buf->byte <<= (8 - buf->bitCount);
        fwrite(&buf->byte, 1, 1, out);
    }
}

void decodificar(FILE* in, FILE* out, Node* raizHuffman, long originalSize){
    BitReader br;
    incializaLeitorBits(&br);

    Node* atual = raizHuffman;
    long escritos = 0;

    // percorre até chegar no tamanho de bytes do arquivo original
    while(escritos < originalSize){
        int bit = readBitCodificado(in, &br);
        if(bit == -1){
            break;
        }

        // esquerda
        if(bit == 0){
            atual = atual->left;
        }
        // direita
        else{
            atual = atual->right;
        }

        // chegou numa folha
        if(!atual->left && !atual->right){
            fwrite(&atual->byte, 1, 1, out);
            escritos++;
            atual = raizHuffman;
        }
    }
}

void main(){
    int freq[256] = {0}; // array utilizado pra armazenar a frequencia de cada byte
    char* huffmanCode[256] = {0}; // array utilizado pra armazenar os códigos gerados
    char caminho[256];

    FILE* input = fopen("palavras.txt", "r");
    FILE* output = fopen("palavras.bin", "wb");

    if(input == NULL){
        printf("Erro ao abrir arquivo");
        exit(1);
    }

    
    printf("\nGerando array de frequencia...");
    gerarArrayDeFrequencia(input, freq);
    rewind(input);
    printf("\nConstruindo MinHeap...");
    MinHeap* heap = buildMinHeapFromArray(freq);

    printf("\nMontando arvore de Huffman...");
    // Monta arvore de huffman
    while(heap->size > 1){
        // Pega os dois de menor frequencia
        Node* left  = extractMin(heap);
        Node* right = extractMin(heap);
        
        // Soma suas frequencias em um nodo pai
        Node* parent = createNode(0, left->freq + right->freq);
        parent->left = left;
        parent->right = right;

        // Insere o pai de volta na heap
        insertMinHeap(heap, parent);
    }
    
    //pega a raiz que vai ser o ultimo que sobrar na minHeap
    Node* raizHuffman = extractMin(heap);

    gerarCodigosHuffman(raizHuffman, caminho, 0, huffmanCode);
    
    // buffer para armazenar os bits do código de huffman
    BitBuffer buf;
    inicializaBuffer(&buf);

    unsigned char c;
    while(fread(&c, 1, 1, input)){
        //pega cada caracter do arquivo input e escreve o codigo respectivo q está armazenado na tabela huffmanCode
        escreveCodigoNoArquivo(output, &buf, huffmanCode[c]);
    }
    
    // o arquivo só salva em byte, entao a gente tem que garantir que o final do arquivo seja multiplo de 8, pra fechar o byte
    preencheUltimoByte(output, &buf);
    
    printf("\n\nTamanho do arquivo de input: %ld bytes", getFileSize(input));
    printf("\nTamanho do arquivo comprimido: %ld bytes", getFileSize(output));

    fclose(input);
    fclose(output);
}