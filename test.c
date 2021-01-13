#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <unistd.h>

#define MAXLINE 1000

int
paayRemoveDuplicates(char *pa[], int n)
{ 
    if (n == 0 || n == 1)
        return n;
 
    char *uniq[n];
	char *current;

	int k=0;
	int i, j;
	for(i=0; i<n; i++) {
	   if( pa[i] == NULL ) {
			continue;
		}
		current = pa[i];
		uniq[k] = current;
		k++;
		for(j=i+1; j<n; j++) {
			if( current == pa[j] ) {
				pa[j] = NULL;
			}
		}
	}

	for (j = i = 0; i < n; i++) {
		if (i > k-1) {
			pa[i] = NULL;
		} else {
			pa[i] = uniq[j++];
		}
	}
 
    return j;
} 

void
printArr(char **pa)
{
	int i = 0;
	while (pa[i]){
		printf("%s | ", pa[i++]);
		fflush(stdout);
	}

	printf("\n");
	fflush(stdout);
}

int
main(int argc, char *argv[])
{
	char *parr[100] = { "asdf", "buff", "buccace", "bunnana", "asdf", "buff", "lisby", "asdf" };

	printArr(parr);

	paayRemoveDuplicates(parr, 8);

	printArr(parr);

	return 0;
}
