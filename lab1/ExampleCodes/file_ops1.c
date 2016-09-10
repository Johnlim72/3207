#include <stdio.h>
int main() {
	char char_array[10] = {'a','b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j' };
	int i;
	for (i = 0; i <10; ++i) {
		FILE* fp = fopen("file_ops_test.txt", "a");
		fwrite(char_array, sizeof(char), sizeof(char_array), fp);
		fclose(fp);
	}
	printf("Done with 10 iterations.\n");
}
