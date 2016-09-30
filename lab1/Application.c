#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

void record_maker(char* array, int size);
void compare(char* array, FILE* fp, int rand2);

int main() {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	printf("Application start time (seconds): %d\n", (int)tv.tv_sec);
 	printf("Application start time (microseconds): %d\n", (int)tv.tv_usec);
	
	FILE *fp2 = fopen("apptimes2.csv", "a");
	fprintf(fp2, "%d, %d\n", (int)tv.tv_sec, (int)tv.tv_usec);
	fclose(fp2);
	 
	char array1[120];
	char array2[120];
	char array3[120];
	char array4[120];
	char array5[120];
	char array6[120];
	char array7[120];
	char array8[120];
	char array9[120];
	char array10[120];
	
	srand(time(NULL));
	int iterate;
	for ( iterate = 1; iterate <= 5; iterate++) {

		//fills in 10 arrays each with 120 random characters
		record_maker(array1, 120);
		record_maker(array2, 120);
		record_maker(array3, 120);
		record_maker(array4, 120);
		record_maker(array5, 120);
		record_maker(array6, 120);
		record_maker(array7, 120);
		record_maker(array8, 120);
		record_maker(array9, 120);
		record_maker(array10, 120);

		//writes the 10 records into a file	
		FILE* fp = fopen("records.txt", "w");
		fwrite(array1, sizeof(char), sizeof(array1), fp);
		fwrite(array2, sizeof(char), sizeof(array2), fp);
		fwrite(array3, sizeof(char), sizeof(array3), fp);
		fwrite(array4, sizeof(char), sizeof(array4), fp);
		fwrite(array5, sizeof(char), sizeof(array5), fp);
		fwrite(array6, sizeof(char), sizeof(array6), fp);
		fwrite(array7, sizeof(char), sizeof(array7), fp);
		fwrite(array8, sizeof(char), sizeof(array8), fp);
		fwrite(array9, sizeof(char), sizeof(array9), fp);
		fwrite(array10, sizeof(char), sizeof(array10), fp);
		fclose(fp);
		
		/*Picks a number from 1-10. Depending on which number is chosen, the corresponding
		array will be chosen to be compared to the characters in the file "records.txt"*/
		int rand2 = rand() % 9 +1;
		char* arrayToCom;
		if (rand2 == 1) {
			arrayToCom = array1;
		} else if (rand2 == 2) {
			arrayToCom = array2;
		} else if (rand2 == 3) {
			arrayToCom = array3;
		} else if (rand2 == 4) {
                        arrayToCom = array4;
		} else if (rand2 == 5) {
                        arrayToCom = array5;
		} else if (rand2 == 6) {
                        arrayToCom = array6;
		} else if (rand2 == 7) {
                        arrayToCom = array7;
		} else if (rand2 == 8) {
                        arrayToCom = array8;
		} else if (rand2 == 9) {
                        arrayToCom = array9;
		} else if (rand2 == 10) {
                        arrayToCom = array10;
		}
				
		compare(arrayToCom, fp, rand2);
		sleep(1);
		printf("Iteration %d..prints %c\n", iterate, array1[0]);
		remove("records.txt");	
	}
	
	printf("Done\n");
}

//fills in an array with random characters from the alphabet
void record_maker(char* array, int size) {
	char alphabet[] = "abcdefghijklmnopqrstuvwxyz";
	int i;
	
	int rnd = rand() % 26;
	for (i = 0; i < size; i++) {
		array[i] = alphabet[rnd];
		rnd = rand() % 26;
	}
}

//compares the characters of an array to the characters at the position where the array was written in the file to see if the characters are the same. 
void compare(char* array, FILE* fp, int rand2) {
	fopen("records.txt", "r");
	fseek(fp, ((rand2-1)*120), SEEK_SET);
	int i;
	int c = fgetc(fp);
	for (i =0; i <120; i++) {
		if (array[i] == c) {			
		
		} else {
			break;
		}
		c = fgetc(fp);
	}
}
