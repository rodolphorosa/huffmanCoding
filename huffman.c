#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
 
#define BYTES 256
#define BITS_PER_CHAR 8

/*
	** huffman_t huffman **

	Estrutura de um no da arvore de Huffman. 
	
		int symbol		: caracter que ocupa o no 
		int freq 		: frequencia do caracter no arquivo 
		struct left 	: subarvore esquerda 
		struct right 	: subarvore direita 
*/
struct huffman 
{
	int symbol; 
	int freq; 
	struct huffman *left;
	struct huffman *right;
};
typedef struct huffman huffman_t;

/*
	** concat **

	Concatena uma string e um caracter. 

	Parametros:
		char *prefix 	: prefixo do no
		char c 			: caracter a ser concatenado 

	Saida:
		char* result	: concatenacao da string e o caracter 
*/
static char *concat (char *prefix, char c); 

/*
	** heapsort **

	Ordena uma lista de nos atraves do algoritmo de Heapsort. 

	Parametros:
		huffman_t *nodes[]	: lista de nos a ser ordenados 
		int len 			: tamanho (quantidade de nos) da lista 

	Saida:
		Lista ordenada. 
*/
void heapsort (huffman_t *nodes[], int len); 

/*
	** create_huffman **

	Constroi a arvore de Huffman. 
	Para cada caracter (de 0 a 255), cria um no e o adiciona na lista de prioridade (heap).
	Enquanto houver nos na heap, retira os dois nos de maior prioridade (menor frequencia) 
	e gera um novo no cuja frequencia e' a soma das frequencias destes dois nos e o adiciona na heap. 
	O ultimo no a sair da lista e' a raiz da arvore. 

	Paramtros:
		long freqs[] 		: frequencia dos caracteres 

	Saida:
		huffman_t heap[0] 	: raiz da arvore de Huffman 
*/
huffman_t *create_huffman (long freqs[]); 

/*
	** create_table **

	Inicializa a table de codigos, onde cada caracter e' representado por um codigo binario. 
	Dado um caracter, verifica sua posicao na arvore e cria o simbolo com base no caminho da raiz ate este no. 

	Parametros:	
		huffman_t *huff : raiz da arvore de Huffman 
		char **table 	: tabela de codigos 
		char *prefix 	: prefixo do no 

	Saida:
		Tabela de codigos criada. 
*/
void create_table (huffman_t *huff, char **table, char *prefix); 

/*
	** write_bits **

	Escreve no arquivo o codigo de um dado caracter. 
	Dado o codigo de um caracter, normaliza a quantidade de bits deste 
	e o escreve como um caracter simples. 

	Parametros:
		FILE *out 	: arquivo de saida (compatado)
		char *code 	: codigo do caracter 

	Saida:
		Codigo escrito no arquivo. 
*/
void write_bits (FILE *out, char *code); 

/*
	** encode **

	Realiza a codificacao de um arquivo. 
	A codificacao e' inicializada com a contagem da frequencia de cada caracter. 
	Uma arvore de Huffman e' criada com base nessas frequencias. 
	Em seguida, cria-se uma tabela de codigos e, para cada caracter, 
	escreve no arquivo de saida	o seu codigo correspondente. 

	Parametros:	
		FILE *in 	: arquivo de entrada (leitura/descompactado) 
		FILE *out 	: arquivo de saida (escrita/compactado) 

	Saida: 
		Arquivo compactado. 
*/
void encode (FILE *in, FILE *out); 

/*
	** decode ** 

	Realiza a decodificacao (descompactacao) de um arquivo. 

	Parametros:
		FILE *in 	: arquivo de entrada (leitura/compactado)
		FILE *out 	: arquivo de saida (escrita/descompatado)

	Saida:

*/
void decode (FILE *in, FILE *out); 

int main ( int argc, char* argv[] ) 
{ 
	char const *input, *output; 

	if (argc < 3) {
		printf ("Missing arguments.\nProgram %s takes exactly 3 arguments.", argv[0]);
		exit (1);
	} 

	if (argc > 3) {
		printf ("Too many arguments.\nProgram %s takes exactly 3 arguments.", argv[0]);
		exit (1);
	} 

	input 	= argv[1]; 
	output 	= argv[2]; 

	printf ("Running program %s with files \"%s\" and \"%s\".\n", argv[0], argv[1], argv[2]);

	FILE *in;
	in = fopen (input, "rb"); 
	if (!in) {
		fprintf (stderr, "Error opening file \"%s\": No such file or directory.\n", argv[1]); 
		exit (1);
	}

	FILE *out;
	out = fopen (output, "wb"); 

	encode (in, out); 
	fclose (in); 
	fclose (out); 

	return 0; 
}

static char *concat (char *prefix, char c) 
{
	char *result = (char*) malloc (strlen(prefix) + sizeof(char));
	sprintf (result, "%s%c", prefix, c);
	return result;
}

void heapsort (huffman_t *nodes[], int len) 
{
	int i, n, father, child;
	huffman_t *h;
	
	i = len / 2; 
	n = len; 

	for (;;) {
		if (i > 0) {
			i--;
			h = nodes[i];
		} else {
			n--;
			if (n == 0) return;
			h = nodes[n];
			nodes[n] = nodes[0];
		}
	 	father 	= i;
	 	child 	= i * 2 + 1; 

		while (child < n) {
			if ((child + 1 < n) && (nodes[child + 1]->freq < nodes[child]->freq)) {
				child++;
			} 			
			if (nodes[child]->freq < h->freq) {
				nodes[father] = nodes[child];
				father = child;
				child = father * 2 + 1;
			} else {
				break;
			}
		}
	 	nodes[father] = h;
	}
}

huffman_t *create_huffman (long freqs[]) 
{
	huffman_t *heap[BYTES*2];
	int len = 0, i;
	
	for (i = 0; i < BYTES; ++i) {
		if (freqs[i]) {
			huffman_t *node = (huffman_t*) malloc (sizeof(huffman_t)); 
			node->right 	= node->left = NULL; 
			node->symbol 	= i; 
			node->freq 		= freqs[i]; 
			heap[len++] 	= node;
		}
	}

	while (len > 1) {
		heapsort (heap, len); 

		huffman_t *h = (huffman_t*) malloc (sizeof(huffman_t)); 
		h->symbol 	= 256;
		h->right 	= heap[--len]; 
		h->left 	= heap[--len]; 
		h->freq 	= h->left->freq + h->right->freq; 

		heap[len++] = h;
	}

	return heap[0]; 
} 

void create_table (huffman_t *huff, char **table, char *prefix) 
{
	if (!huff->left && !huff->right) {
		table[huff->symbol] = prefix;
	} else {
		if (huff->left) create_table (huff->left, table, concat (prefix, '0')); 
		if (huff->right) create_table (huff->right, table, concat (prefix, '1')); 
	}
} 

void write_bits (FILE *out, char *code) 
{
	unsigned int bits = 0;
	int bitcount = 0;

	while (*code) {
		bits = bits << 1 | *code;
		bitcount++;

		if (bitcount == BITS_PER_CHAR) {
			fputc(bits, out);
			bits = 0;
			bitcount = 0;
		}
		code++;
	}
} 

void encode (FILE *in, FILE *out) 
{
	long freqs[BYTES]; 

	memset (freqs, 0, sizeof freqs); 

	int character;
	while ((character = fgetc(in))!=EOF) { 
		freqs[character]++; 
	} 

	huffman_t *h;
	h = create_huffman (freqs); 

	char *table[BYTES];
	char *prefix = "0";
	create_table (h, table, prefix); 

	rewind (in);

	while ((character = fgetc(in))!=EOF) { 
		write_bits (out, table[character]);
	} 
} 

void decode (FILE *in, FILE *out) 
{

}