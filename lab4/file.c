#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>

FILE* fp; //pointer to the filesystem.

int size_fat_entry = 3; //size of fat entry: 3 bytes.
int size_block = 512; //size of each data block.
int num_blocks = 4096; //total number of blocks in filesystem.
int num_fats = 1; //number of FATs
int num_root = 1; //number of root directories
int max_size = 32768; //max file size
int where_fat = 513; //24 blocks for FAT
int where_root = 12801; //10 blocks for Root directory
int where_data = 17921; //rest of the blocks

// if num_blocks value changes, change size of these arrays to match.
int space_array[4096]; //see which blocks are free, 1 for free, 0 for used.
int fat[4096]; //4096 total data blocks (not all are going to be used)

void metadata();
void write_entry(int root_pos, char* filename, char* ext, int attr, int time, int date, int state, int size);
int locate_empty_fat();
int write_fat_entry();
int jcreate();

char* error = "error";

int main() {
	//Initialize meta data's first line: name of the file system.
	fp = fopen("output.dat", "w+");
	char *str = "John Lim's File System";
	fwrite(str, sizeof(char), 22, fp);
	
	//Initialize every entry in space_array to 0, except for first 3 blocks.
	//Initialize every entry in fat to 0, except for first 3 blocks.
	int i;
	for (i = 0; i < 3; i++) { //first 3 blocks are for metadata, FAT, and root directory. They are data blocks that can't be used by user.
		space_array[i] = 1;
		fat[i] = 1;
	}
	for(i = 3; i < num_blocks; i++) {
		space_array[i] = 0;	
		fat[i] = 0;
	}

	metadata();
	write_entry(12801, "filename", "txt", 1, 65424, 12216, 17921, 340); 
	
	return 0;
}

void metadata() {
	fseek(fp, 32, SEEK_SET);
	fwrite(&size_block, sizeof(int), 1, fp);	
	fseek(fp, 40, SEEK_SET);
	fwrite(&num_fats, sizeof(int), 1, fp);
	fseek(fp, 48, SEEK_SET);
	fwrite(&num_blocks, sizeof(int), 1, fp);
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
	fseek(fp, 96, SEEK_SET);
	fwrite(&size_fat_entry, sizeof(int), 1, fp);
}

void write_entry(int dir_pos, char* filename, char* ext, int attr, int time, int date, int start, int size) {
	fseek(fp, dir_pos, SEEK_SET);
	fwrite(filename, sizeof(char), 8, fp);
	fseek(fp, 8, SEEK_CUR);
	fwrite(ext, sizeof(char), 3, fp);
	fseek(fp, 11, SEEK_CUR);
	fwrite(&attr, sizeof(char), 1, fp);
	fseek(fp, 22, SEEK_SET);
	fwrite(&time, sizeof(int), 2, fp);
	fseek(fp, 24, SEEK_SET);
	fwrite(&date, sizeof(int), 2, fp);
	fseek(fp, 26, SEEK_SET);
	fwrite(&start, sizeof(int), 2, fp);
	fseek(fp, 28, SEEK_SET);
	fwrite(&size, sizeof(int), 4, fp);
	
}

int locate_empty_fat() {
	int i = 2; //skip first two blocks for checking, but include block 3 to avoid errors
	int empty_space; //block number of first empty space.
	while (1) {
		if (space_array[i] == 0) {
			empty_space == i;
			break;
		} else if (i == 4095) {
			return -1;
		} else if (space_array[i] == 1) {
			i++;
		}
	}

	if (fat[empty_space] == 0) {
		return empty_space;
	} else {
		return -1;
	}
	return -1;
}

int write_fat_entry() {
	int empty_space = locate_empty_fat();
	int offset = 618 + ((empty_space-3) * size_fat_entry);
	fseek(fp, offset, SEEK_SET);

//	fwrite(&fat_entry, sizeof(int), 3, fp);

	//have to know if there are mult blocks for file
	
}

int jcreate() {
	fseek(fp, where_root, SEEK_SET);
	
}

int search_dir(char* path_string) {
	//make sure path size is valid
	//put each section the path_string into path_array
	//make sure first section in path is /root
	//for each section in path, go to the directory in data section
	//are they searching for file or directory?

	if (sizeof(path) <= 5) {
		return -1;
	}	

	if (path_string[0] != "/") {
		return -1;
	}

	char* word; //temporary path name
	char* path_array[]; //array for all names in path
	char *c; //temporary filename in given directory
	int pointer; //stores pointer to dir/file
	int offset = 12801;
	int i = 1;
	while (i < (sizeof(path_string) -1)) {
		while (path_string[i] != "/") {
			word[i] = path_string[i];
			i++;
		}
		if (path_string[i] == "/") {
			i++;
		}
		path_array[path_array_count] = word;
		path_array_count++;
	}

	int j = 0;
	c = get_entry_name(offset);
	while(strcmp(path_array[j], c) != 0) {
		offset+=32;
		c = get_entry_name(offset);
	}
	if (strcmp(path_array[j], c) == 0 ) {
		fseek(fp, 26, SEEK_CUR);
		fread(pointer, sizeof(int), 2, fp);
	}

}

char* get_entry_name(int offset) {
	fseek(fp, offset, SEEK_SET);
	char c[8];
	fread(c, sizeof(char), 8, fp);
	if (c[0] == NULL) {
		return error;
	}
	return c;
}

//search directory /fp/root/dir1/file.txt
//format function
//check allocation space
