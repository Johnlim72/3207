#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

FILE* fp; //pointer to the filesystem.

int size_dir_entry = 32;
int size_fat_entry = 3; //size of fat entry: 3 bytes.
int size_block = 512; //size of each data block.
int num_blocks = 4096; //total number of blocks in filesystem.
int num_fats = 1; //number of FATs
int num_root = 1; //number of root directories
int max_size = 32768; //max file size
int where_fat = 513; //24 blocks for FAT
int where_root = 12801; //10 blocks for Root directory
int where_data = 17921; //rest of the blocks

void metadata();
void write_dir_entry(int offset, int free, char* filename, char* ext, int attr, int time, int date, int start, int size);
int jcreate_dir(char* path_string);
int search(char path_string[]);
char* get_entry_name(int offset);
int get_entry_byte(int offset);
int go_first_empty(int offset);

char* error = "error";

int main() {
	//Initialize meta data's first line: name of the file system.
	fp = fopen("output.dat", "w+");
	char *str = "John Lim's File System";
	fwrite(str, sizeof(char), 22, fp);

	metadata();
	write_dir_entry(12801, 1,  "dir1", "dir", 1, 65424, 12216, 17921, 4); 
	write_dir_entry(12833, 1,  "dir2", "dir", 1, 65424, 12216, 17953, 5); 
	write_dir_entry(17953, 1,  "file", "txt", 2, 65424, 12216, 17985, 6); 
	
	char path[] = "/root/dir2";
	int o = search(path);
	printf("%d\n", o);
	return 1;
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

int jcreate_dir(char* path_string) {
	int offset = search(path_string);
	int first_empty = go_first_empty(offset);

	char filename[8];
	printf("Enter filename (8 character limit): ");
	scanf("%s", filename);
	char ext[3];
	printf("Enter file extension (3 character limit): ");
	scanf("%s", ext);
	int attr;
	printf("Enter file attribute (1 for file, 2 for directory): ");
	scanf("%d", &attr);
	
	time_t now;
	struct tm *now_tm;	
	now = time(NULL);
	now_tm = localtime(&now);
	int hour = now_tm->tm_hour;
	int min = now_tm->tm_min;
	int day = now_tm->tm_mday;
	int month = (now_tm->tm_mon) + 1;
	int year = ((now_tm->tm_year) - 100);

	//look for first empty block
	return 1;
}

//writes a directory entry
void write_dir_entry(int offset, int free, char* filename, char* ext, int attr, int time, int date, int start, int size) {
	fseek(fp, offset, SEEK_SET);
	fwrite(&free, 1, 1, fp);
	fseek(fp, offset+1, SEEK_SET);
	fwrite(filename, sizeof(char), strlen(filename), fp);
	fseek(fp, offset+9, SEEK_SET);
	fwrite(ext, sizeof(char), strlen(ext), fp);
	fseek(fp, offset+12, SEEK_SET);
	fwrite(&attr, sizeof(char), 1, fp);
	fseek(fp, offset+22, SEEK_SET);
	fwrite(&time, 2, 1, fp);
	fseek(fp, offset+24, SEEK_SET);
	fwrite(&date, 2, 1, fp);
	fseek(fp, offset+26, SEEK_SET);
	fwrite(&start, 2, 1, fp);
	fseek(fp, offset+28, SEEK_SET);
	fwrite(&size, 4, 1, fp);	
}

//returns starting byte of last element in the given path
int search(char path_string[]) { 
	int offset = 12801;
	char* word;
	char* s = "/";
	int count = 0;
	word = strtok(path_string, s);
	word = strtok(NULL, s);
	while (word != NULL) {
		printf("word: %s\n", word);
		count = 0;
		while (count < 512) {
			if (strcmp(get_entry_name(offset), word) == 0) {
				printf("success\n");
				break;
			}
			offset+=32;		
			count+=32;
		}
		if (count == 512) {
			return -1;
		}
		offset = get_entry_byte(offset);
		word=strtok(NULL,s);
	}
	return offset;
}

//returns byte of first empty entry
int go_first_empty(int offset) {
	int i = occupied(offset);
	int count = 0;
	while (count < 512) {
		if (i == 1) {
			count+=32;
			offset+=32;
			i = occupied(offset);
		} else if (i == 0) {
			return offset;
		}
	}
	return -1;
}

//returns filename
char* get_entry_name(int offset) {
	int i = occupied(offset);
	printf("occupied: %d\n", i);
	if (i == 1) {
		fseek(fp, offset+1, SEEK_SET);
		char c[8];
		fread(c, sizeof(char), 8, fp);
		printf("entry_name: %s\n", c);
		return c;
	} else {
		return error;
	}
}

//returns starting byte 
int get_entry_byte(int offset) {
	int i = occupied(offset);
	if (i = 1) {
		fseek(fp, offset+26, SEEK_SET);
		int c;
		fread(&c, 2, 1, fp);
		return c;
	} else {
		return -1;	
	}
}

//returns 0 if free, 1 if occupied
int occupied(int offset) {
	fseek(fp, offset, SEEK_SET);
	int i = 0;
	fread(&i, 1, 1, fp);
	return i;
}
