#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>

int root_start_pos = 12801;
int space_array[3900]; //see which blocks are free, 1 for free, 0 for used
int space_count = 0;
int fat[8*512];
int fat_count = 0;

void metadata(FILE *fp);
void write_entry(FILE *fp, int root_pos, char* filename, char* ext, int attr, int time, int date, int state, int size);
void jcreate(FILE *fp);

int main() {
	FILE* fp = fopen("output.dat", "w+");
	char *str = "John Lim's File System";
	fwrite(str, sizeof(char), 22, fp);
	
	int i;
	for(i = 0; i < 3900; i++) {
		space_array[i] = 0;	
	}

	metadata(fp);
	write_entry(fp, 12801, "filename", "txt", 1, 65424, 1001010, 17921, 340); 
	
	return 0;
}

void metadata(FILE *fp) {
	int size_block = 512; //size of each block
	int num_fats = 1; //number of FATs
	int num_root = 1; //number of root directories
	int total_blocks = 3906; //total number of blocks in filesystem
	int max_size = 32768; //max file size
	int where_fat = 513; //24 blocks for FAT
	int where_root = 12801; //10 blocks for Root directory
	int where_data = 17921; //rest of the blocks

	fseek(fp, 32, SEEK_SET);
	fwrite(&size_block, sizeof(int), 1, fp);	
	fseek(fp, 40, SEEK_SET);
	fwrite(&num_fats, sizeof(int), 1, fp);
	fseek(fp, 48, SEEK_SET);
	fwrite(&total_blocks, sizeof(int), 1, fp);
	fseek(fp, 56, SEEK_SET);
	fwrite(&max_size, sizeof(int), 1, fp);
	fseek(fp, 64, SEEK_SET);
	fwrite(&num_root, sizeof(int), 1, fp);
	fseek(fp, 72, SEEK_SET);
	fwrite(&where_fat, sizeof(int), 1, fp);
	fseek(fp, 80, SEEK_SET);
	fwrite(&where_root, sizeof(int), 1, fp);
	fseek(fp, 88, SEEK_SET);
	fwrite(&where_data, sizeof(int), 1, fp);
}

void write_entry(FILE *fp, int dir_pos, char* filename, char* ext, int attr, int time, int date, int start, int size) {
	fseek(fp, dir_pos, SEEK_SET);
	fwrite(filename, sizeof(char), 8, fp);
	fseek(fp, dir_pos+8, SEEK_SET);
	fwrite(ext, sizeof(char), 3, fp);
	fseek(fp, dir_pos+11, SEEK_SET);
	fwrite(&attr, sizeof(char), 1, fp);
	fseek(fp, dir_pos+22, SEEK_SET);
	fwrite(&time, sizeof(int), 2, fp);
	fseek(fp, dir_pos+24, SEEK_SET);
	fwrite(&date, sizeof(int), 2, fp);
	fseek(fp, dir_pos+26, SEEK_SET);
	fwrite(&start, sizeof(int), 2, fp);
	fseek(fp, dir_pos+28, SEEK_SET);
	fwrite(&size, sizeof(int), 4, fp);
	
	space_array[space_count] = 1;
	space_count++;
}

void jcreate(FILE *fp) {

}

void jopen(FILE *fp) {
	
}

//search directory
//format function<F2>

