/** @file huffman.c
 *  @brief Implementação do algoritmo de compressão de Huffman. 
 *  
 *  Esta implementação contém as funcionalidades de construção da árvore de Huffman
 *  e das funcionalidades de compressão e descompressão de arquivos de texto. 
 *  Este projeto foi desenvolvido como trabalho de atividade acadêmica do curso
 *  de Ciência da Computação da Universidade Federal Rural do Rio de Janeiro (UFRRJ). 
 *  Ao término da funcionalidade de compressão, é exibida a taxa de compressão, 
 *  bem como o tempo de execução do algoritmo. 
 *  
 *  @author Rodolpho Rosa 
 *  @author Diogo Vieira
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/*! Número máximo de caracteres. */
#define CHAR_RANGE  257 
/*! Marcador de final de arquivo (compactado). */
#define END_OF_FILE 256 
/*! Numero de bits por caracter (1 byte). */
#define CHAR_BITS 8 

/*! Tabela de códigos. */
char *table[CHAR_RANGE]; 
/*! Buffer para leitura e escrita de bytes. */
int buffer = 0; 
/*! Contador do tamanho do buffer. */
int buffercount = 0; 

/** 
 * Estrutura de um nó da árvore de Huffman. 
 * 
 */
typedef struct huffman 
{
    /*! Letra ou símbolo. */
    int letter; 
    /*! Número de ocorrências. */
    int freq; 
    /*! Ponteiro para a subárvore esquerda. */
    struct huffman *left; 
    /*! Ponteiro para a subárvore direita. */
    struct huffman *right; 
} huffman_t;

/** @brief Compara dois nós. 
 *  
 *  Dados dois nós (x) e (y), são comparadas as frequências de (x) e (y).
 *  Retorna (0) se as duas frequências forem iguais, (1), caso a frequência 
 *  de x seja menor que a de y, ou (-1), caso contrário. 
 *  
 *  @param a Nó da árvore de Huffman.
 *  @param b Nó da árvore de Huffman.
 *  @return Resultado da comparação.
 */ 
int compare (const void *a, const void *b);

/** @brief Concatena uma string e um caracter. 
 *  
 *  Dadas uma string e um caracter, cria-se uma nova string com o caracter 
 *  adicionado ao final da primeira. 
 *  É utilizada para a formação do código que representará um dado caracter. 
 *  
 *  @param prefix Prefixo (string). 
 *  @param c Caracter a ser concatenado. 
 *  @return Prefixo e caracter concatenados. 
 */ 
char *concat (char *prefix, char c);

/** @brief Calcula a taxa de compressão. 
 * 
 * A partir do vetor de frequências, calcula o número de bytes do arquivo de entrada 
 * e o número de bytes do arquivo resultante da compressão e calcula a pontagem
 * de dados compactados. 
 * 
 * @param frequencies Frequências dos caracteres. 
 * @return Taxa de compressão. 
 */
double compression_rate (int frequencies[]);

/** @brief Constrói a árvore de Huffman. 
 *  
 *  A partir do vetor de frequências, contrói a árvore de Huffman. 
 *  Para cada caracter, é criado um nó que será adicionado à fila de prioridades (heap). 
 *  Após isso, são removidos os dois nós de maior prioridade (menor frequência)
 *  da fila, e cria-se um novo nó cujas subárvores sejam esses dois nós e a frequência deste 
 *  seja igual à soma da frequência dos dois. E este nó é adicionado ao final da fila. 
 *  Repete-se esse procedimento até que reste apenas um nó. Este será a raiz da árvore. 
 *  
 *  @param frequencies Frequências dos caracteres. 
 *  @return heap[--len] Ponteiro para a raiz da árvore. 
 */
huffman_t *create_tree (int frequencies[]);

/** @brief Constrói a tabela de códigos.
 *  
 *  Percorre a árvore de Huffman recursivamente. 
 *  Para cada nó, adiciona-se '0' ao prefixo caso este nó seja uma subárvore esquerda, 
 *  ou '1', se for subárvore direita. 
 *  Este procedimento é realizado até que se chegue a uma folha. 
 *  
 *  @param h Árvore de Huffman.
 *  @param table Tabela de códigos. 
 *  @param prefix Prefixo (código). 
 *  @return void
 */
void searchtree(huffman_t *h, char **table, char *prefix);

/** @brief Inicializa a tabela de códigos. 
 *  
 *  Cria uma árvore de Huffman e usa searchtree() para inicilizar a tabela. 
 *  
 *  @param frequencies Vetor de frequências.
 *  @return Tabela de códigos construída. 
 */
char **begin_table (int frequencies[]);

/** @brief Escreve o cabeçalho (header) do arquivo compactado. 
 *  
 *  Para cada caracter cuja frequência seja maior que 0 (zero), 
 *  este e sua frequência são escritos no início do arquivo. 
 *  A informação da quantidade de caracteres também é escrita. 
 *  
 *  @param out Arquivo de saída (compactado).
 *  @param freqs Vetor de frequências. 
 *  @return void
 */
void write_header (FILE *out, int freqs[]);

/** @brief Escreve um byte no arquivo de saída. 
 *  
 *  Para cada bit do código que representa um caracter a ser escrito, 
 *  adiciona-se este ao buffer até que se obtenha um byte (caracter) 
 *  e este é escrito no arquivo de saída (compactado). 
 *  A adição dos bits no buffer é feito por meio de operação lógica 'ou'
 *  e operação de shift left (<<). 
 *  
 *  @param out Arquivo de saída (compactado). 
 *  @param code Código do caracter. 
 *  @return void
 */
void write_byte (FILE *out, const char *code);

/** @brief Lê o cabeçalho do arquivo compactado. 
 *  
 *  Lê o cabelhaço do arquivo a ser descompactado, consistindo 
 *  nos caracteres originais, seguidos da quantidade de ocorrências 
 *  no arquivo original. 
 *  
 *  @param in Arquivo de entrada. 
 *  @return frequencies Vetor de frequências dos caracteres. 
 */
int *read_header (FILE *in);

/** @brief Lê um bit do arquivo compactado. 
 *  
 *  Lê o arquivo compactado bit a bit. 
 *  Para cada caracter lido do arquivo, adiciona-se um byte ao buffer
 *  e lê-se o bit mais significativo. 
 *  
 *  @param in Arquivo de entrada (compactado). 
 *  @return bit Bit mais significativo do buffer. 
 */
int read_bit (FILE *in);

/** @brief Retorna um caracter decodificado. 
 *  
 *  Para cada bit (mais significativo) do buffer, percorre a árvore de Huffman
 *  até que se chegue a um nó, isto é, até que se encontre um caracter. 
 *  
 *  @param in Arquivo de entrada (compactado). 
 *  @param h Ponteiro para a raiz da árvore de Huffman 
 *  @return h->letter Caracter decodificado. 
 */
int read_char (FILE *in, huffman_t *h);

/** @brief Realiza compressão de Huffman. 
 *  
 *  Para cada caracter do arquivo de entrada, calcula sua frequência. 
 *  A partir das frequências, é gerada uma tabela de códigos. 
 *  Volta-se ao início do arquivo e, até que se chegue ao final do arquivo, 
 *  é escrita a codificação de cada carater. 
 *  Ao final, escreve-se um código para marcar o final do arquivo compactado (END_OF_FILE) 
 *  e mais alguns bits são adicionados para preencher o buffer. 
 *  
 *  @param in Arquivo de entrada (compactado). 
 *  @param out Arquivo de saída (descompactado). 
 *  @return void
 */
void compress (FILE *in, FILE *out);

/** @brief Realiza a decompressão de Huffman. 
 *  
 *  Lê o cabelhaço do arquivo compactado e cria uma árvore de Huffman. 
 *  Até que se chegue ao final do arquivo, é codificado um caracter 
 *  e este é escrito no arquivo de saída (descompatado). 
 *  Ao final é exibido o tempo de execução da funcionalidade. 
 *  
 *  @param in Arquivo de entrada (compactado). 
 *  @param out Arquivo de saída (descompactado).
 *  @return void
 */
void decompress (FILE *in, FILE *out);

/** @brief Função principal. 
 *  
 *  Recebe os arquivos de entrada e o modo de execução do algoritmo. 
 *  Verifica os parâmetros de executa a compressão ou descompressão. 
 *  Os parâmetros necessários para a execução do algoritmo são 
 *  o nome do arquivo de entrada, nome do arquivo de saída e modo de execução, 
 *  que determina se o arquivo será compactado ou descompactado. 
 *  
 *  @param argc Número de argumentos da entrada. 
 *  @param argv Arquivos de entrada e saída e modo de execução. 
 *  @return 0
 */
int main(int argc, char *argv[])
{
    char const *input, *output; 

    if (argc < 3) 
    {
        printf ("Missing arguments.\nArgs: <inputname> <outputname>", argv[0]); 
        exit (1);
    } 

    if (argc > 3) 
    {
        printf ("Too many arguments.\nArgs: <inputname> <outputname>", argv[0]); 
        exit (1);
    } 

    input   = argv[1]; 
    output  = argv[2]; 

    printf ("Running program %s with files \"%s\" and \"%s\".\n", argv[0], argv[1], argv[2]);

    FILE *in;
    in = fopen (input, "rb"); 
    if (!in) 
    {
        fprintf (stderr, "Error opening file \"%s\": No such file or directory.\n", argv[1]); 
        exit (1);
    }

    FILE *out;
    out = fopen (output, "wb"); 

    char mode;

    printf ("Press 'c' to compress or 'd' to decompress...\n");
    printf ("[c/d]: ");
    scanf ("%c", &mode);

    clock_t tic, toc;
    double time_spent;

    if (mode == 'c') 
    {
        tic = clock();
        compress (in, out); 
        toc = clock();
        time_spent = ((double)(toc - tic) / CLOCKS_PER_SEC);
        printf ("Compression time: %f.\n", time_spent);
    }
    else if (mode == 'd') 
    {
        tic = clock();
        decompress (in, out); 
        toc = clock();
        time_spent = ((double)(toc - tic) / CLOCKS_PER_SEC);
        printf ("Decompression time: %f.\n", time_spent);
    }
    fclose (in); 
    fclose (out); 
    
    return 0;
}

int compare (const void *a, const void *b)
{
    const huffman_t **x = (const huffman_t **) a;
    const huffman_t **y = (const huffman_t **) b;
    
    if ((*x)->freq == (*y)->freq) 
        return 0;
    else 
        return ((*x)->freq < (*y)->freq) ? 1 : -1;
}

char *concat (char *prefix, char c)
{
    char *result = (char*) malloc (strlen(prefix) + 2);
    sprintf(result, "%s%c", prefix, c);
    return result;
}

double compression_rate (int frequencies[])
{
    int nbytes = 0; /* Tamanho do arquivo original. */
    int nbytes_compressed = 0; /* Tamanho do arquivo compactado. */

    int i;
    for (i = 0; i < CHAR_RANGE - 1; ++i) 
    {
        if (frequencies[i])
        {
            nbytes += frequencies[i];
            nbytes_compressed += frequencies[i] * strlen(table[i]);
        }
    }
    
    nbytes_compressed /= 8;
    double rate = ((double)((double) (nbytes - nbytes_compressed) / (double) nbytes)) * 100.0;
    return rate;
}

huffman_t *create_tree (int frequencies[])
{
    int i, len = 0;
    huffman_t *heap[CHAR_RANGE];
    
    for(i = 0; i < CHAR_RANGE; i++)
    {
        if(frequencies[i])
        {
            huffman_t *node = (huffman_t *) malloc (sizeof(huffman_t));
            node->left      = NULL;
            node->right     = NULL;
            node->letter    = i;
            node->freq      = frequencies[i];

            heap[len++]     = node;
        }
    }
    
    while(len > 1)
    {
        huffman_t *node = (huffman_t *) malloc (sizeof(huffman_t));
        
        qsort(heap, len, sizeof(huffman_t *), compare);
        
        node->left  = heap[--len];
        node->right = heap[--len];
        node->freq  = node->left->freq + node->right->freq;
        
        heap[len++] = node;
    }
    
    return heap[--len];
}

void searchtree(huffman_t *h, char **table, char *prefix)
{
    if (!h->left && !h->right) 
        table[h->letter] = prefix;
    else
    {
        if (h->left) 
            searchtree(h->left, table, concat(prefix, '0'));
        if (h-> right) 
            searchtree(h->right, table, concat(prefix, '1'));
        free(prefix);
    }
}

char **begin_table (int frequencies[])
{
    char *prefix = (char*) malloc (sizeof(char));  
    memset (prefix, 0, sizeof prefix); 
    
    huffman_t *h = create_tree (frequencies);
    searchtree(h, table, prefix);
    
    return table;
}

void write_header (FILE *out, int freqs[])
{
    int i;
    int charscount = 0;

    for (i = 0; i < CHAR_RANGE; ++i) {
        if (freqs[i]) {
            charscount++;
        }
    }

    fprintf (out, "%d\n", charscount);

    for (i = 0; i < CHAR_RANGE; ++i) {
        if (freqs[i]) {
            fprintf (out, "%d %d\n", i, freqs[i]);
        }
    }
}

void write_byte (FILE *out, const char *code)
{    
    while(*code)
    {
        buffer = (buffer << 1) | ( *code == '1' ? 1 : 0);
        buffercount++;
        
        if(buffercount == CHAR_BITS)
        {
            fputc (buffer, out);
            buffer = 0;
            buffercount = 0;
        }
        
        code++;
    }
}

int *read_header (FILE *in)
{
    static int frequencies[CHAR_RANGE];
    int i, count, letter, freq;
    
    if (fscanf(in, "%d", &count) != 1) 
        printf ("Invalid input file.");
    
    for (i = 0; i < count; ++i)
    {
        if ((fscanf(in, "%d %d", &letter, &freq) != 2) || letter < 0 || letter >= CHAR_RANGE)
            printf ("Invalid input file.");         
        frequencies[letter] = freq;
    }

    fgetc(in); 
    
    return frequencies;
}

int read_bit (FILE *in)
{
    if (buffercount == 0) 
    {
        buffer = fgetc (in);
        buffercount = CHAR_BITS;
    }

    int mask = 1 << (buffercount - 1);
    int bit  = (buffer & mask) >> (buffercount - 1);
    buffercount--;

    return bit;
}

int read_char (FILE *in, huffman_t *h)
{
    while(h->left || h->right)
    {
        h = read_bit (in) ? h->right : h->left;
        
        if (!h)
            printf ("Iinvalid input file.");
    }
    
    return h->letter;
}

void compress (FILE *in, FILE *out)
{
    printf("Compressing file...\n");

    int c, frequencies[CHAR_RANGE];

    memset (frequencies, 0, CHAR_RANGE * sizeof(int));
    
    while ((c = fgetc(in)) != EOF) 
    {
        frequencies[c]++;
    }
    
    frequencies[END_OF_FILE] = 1;
    rewind(in);

    begin_table (frequencies);
    write_header (out, frequencies); 
    
    while ((c = fgetc(in)) != EOF) 
    {
        write_byte (out, table[c]);
    }        

    write_byte (out, table[END_OF_FILE]); 
    write_byte (out, "0000000"); 

    double c_rate = compression_rate (frequencies);
    printf ("Compression ended with compression rate of %.2f %%.\n", c_rate);
}

void decompress (FILE *in, FILE *out)
{
    printf("Decompressing file...\n");

    int *frequencies;
    int c;

    frequencies = read_header (in);

    huffman_t *h;
    h = create_tree (frequencies);

    while ((c = read_char (in, h)) != END_OF_FILE)
        fputc (c, out);

    printf ("Decompression ended.\n");
}