#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHAR_RANGE  257
#define END_OF_FILE    256
#define CHAR_BITS   8

#ifndef percentage
#define percentage(x, y) (100.0 * y)/x
#endif

char *table[CHAR_RANGE];
int buffer = 0;
int buffercount = 0;

/* Huffman tree structure */
typedef struct huffman
{
    int letter;
    int freq;
    struct huffman *left; 
    struct huffman *right; 
} huffman_t;

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

huffman_t *create_tree (int frequencies[])
{
    int i, len = 0;
    huffman_t *heap[CHAR_RANGE];
    
    for(i = 0; i < CHAR_RANGE; i++)
    {
        if(frequencies[i])
        {
            huffman_t *node = (huffman_t *) calloc (1, sizeof(huffman_t));
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
        //buffer = buffer * 2 + *code - '0';
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

    int mask    = 1 << (buffercount - 1);
    int nextbit = (buffer & mask) >> (buffercount - 1);
    buffercount--;

    return nextbit;
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
}

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

    printf("Press 'c' to compress or 'd' to decompress...\n");
    printf("[c/d]:");
    scanf("%c", &mode);

    if (mode == 'c') compress (in, out); 
    else if (mode == 'd') decompress (in, out); 
    fclose (in); 
    fclose (out); 
    
    return 0;
}
