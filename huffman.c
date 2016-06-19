#include <stdio.h>
#include <stdlib.h>
#include <string.h>
 
#define BYTES 256

struct huffman 
{
	int symbol; 
	int freq; 
	struct huffman *left;
	struct huffman *right;
};
typedef struct huffman huffman_t;

static char *concat (char *prefix, char c) 
{
	char *result = (char*) malloc (strlen(prefix) + sizeof(char));
	sprintf (result, "%s%c", prefix, c);
	return result;
}

void heap_sort (huffman_t *nodes[], int len) 
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

	for (i = 0; i< len; ++i) { 
		// printf("'%c' (%d)\n", heap[i]->symbol, heap[i]->freq);
	} 

	while (len > 1) {
		heap_sort (heap, len); 

		huffman_t *h = (huffman_t*) malloc (sizeof(huffman_t)); 
		h->symbol 	= 256;
		h->right 	= heap[--len]; 
		h->left 	= heap[--len]; 
		h->freq 	= h->left->freq + h->right->freq; 

		heap[len++] = h;
	}

	return heap[0]; 
} 

void create_table (huffman_t *huff, char **table, char *prefix) {
	if (!huff->left && !huff->right) {
		table[huff->symbol] = prefix;
	} else {
		if (huff->left) create_table (huff->left, table, concat (prefix, '0')); 
		if (huff->right) create_table (huff->right, table, concat (prefix, '1')); 
	}
}

int main ( int argc, char* argv[] ) { 

	char const *input, *output; 

	input 	= argv[1]; 
	output 	= argv[2]; 

	FILE *fp;
	fp = fopen (input, "r"); 

	long freqs[BYTES]; 

	memset (freqs, 0, sizeof freqs); 

	int character;
	while ((character = fgetc(fp))!=EOF) { 
		freqs[character]++; 
	} 

	huffman_t *h;
	h = create_huffman (freqs); 

	char *table[BYTES];
	char *prefix = "0";
	create_table (h, table, prefix); 

	rewind (fp);

	FILE *out;
	out = fopen(output, "wb");

	while ((character = fgetc(fp))!=EOF) { 
		// printf("%s", table[character]);
		fputs(table[character], out);
	} 

	fclose(fp);
	fclose(out);

	return 0; 
}