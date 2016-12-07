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
int where_root = 12801; //10 blocks for Root directory 12802
int where_data = 17921; //rest of the blocks 17923

void metadata();
void write_entry(int offset, int free, char filename[], char ext[], int attr, int hour, int min, int day, int month, int year, int open, int start, int size);
void init_fat(int offset);
int locate_first_empty_fat();
int jcreate(char path_string[]);
int jopen(char path_string[]);
int jclose(char path_string[]);
char* get_last_pathname(char path_string[]);
int edit_entry(int offset, char what[]);
int search(char path_string[]);
int search_parent_dir(char path_string[]);
char* get_entry_name(int offset);
int get_entry_byte(int offset);
int go_first_empty(int offset);
int occupied(int offset);

char* error = "error";

int main() {
	//Initialize meta data's first line: name of the file system.
	fp = fopen("output.dat", "w+");
	char *str = "John Lim's File System";
	fwrite(str, sizeof(char), 22, fp);

	metadata();
	
	char* word = get_last_pathname("/root/dir1/file.txt");
	printf("word: %s\n", word);
	jcreate("/root/file.txt");
	jcreate("/root/dir1");
	jcreate("/root/dir1/file3.txt");
	jcreate("/root/dir2");
	jcreate("/root/dir2/file2.txt");
	jcreate("/root/dir1/file4.txt");
	jopen("/root/dir1/file4.txt");
	
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

//switches the open argument to 1 for allowance of reading and writing
int jopen(char path_string[]) {
	int offset = search_parent_dir(path_string);
	if (edit_entry(offset, "open") == 1) {
		return 1;
	}
	return -1;
}

int jclose(char path_string[]) {
	int offset = search_parent_dir(path_string);
	if (edit_entry(offset, "open") == 0) {
		return 1;
	}
	return -1;
}

int edit_entry(int offset, char what[]) {
	fseek(fp, offset, SEEK_SET);
	
	time_t now;
	struct tm *now_tm;	
	now = time(NULL);
	now_tm = localtime(&now);
	int hour = now_tm->tm_hour;
	int min = now_tm->tm_min;
	int day = now_tm->tm_mday;
	int month = (now_tm->tm_mon) + 1;
	int year = ((now_tm->tm_year) - 100);
	
	if (strcmp(what, "filename") == 0) {
		char filename[8];
		printf("Enter filename: ");
		scanf("%s", filename);
		fseek(fp, offset+1, SEEK_SET);
		fwrite(filename, sizeof(char), 8, fp);
	} else if (strcmp(what, "ext") == 0) {
		char ext[3];
		printf("Enter ext: ");
		scanf("%s", ext);
		fseek(fp, offset+9, SEEK_SET);	
		fwrite(ext, sizeof(char), 3, fp);
	} else if (strcmp(what, "attr") == 0) {
		int attr;
		printf("Enter attr: ");
		scanf("%d", &attr);
		fseek(fp, offset+12, SEEK_SET);
		fwrite(&attr, sizeof(char), 1, fp);
	} else if (strcmp(what, "hour") == 0) {
		fseek(fp, offset+13, SEEK_SET);
		fwrite(&hour, 2, 1, fp);
	} else if (strcmp(what, "min") == 0) {
		fseek(fp, offset+15, SEEK_SET);
		fwrite(&min, 2, 1, fp);
	} else if (strcmp(what, "day") == 0) {
		fseek(fp, offset+17, SEEK_SET);
		fwrite(&day, 2, 1, fp);
	} else if (strcmp(what, "month") == 0) {
		fseek(fp, offset+19, SEEK_SET);
		fwrite(&month, 1, 1, fp);
	} else if (strcmp(what, "year") == 0) {
		fseek(fp, offset+20, SEEK_SET);
		fwrite(&year, 2, 1, fp);
	} else if (strcmp(what, "open") == 0) {
		int open;
		printf("Enter 0 for close, 1 for open: ");
		scanf("%d", &open);
		if (open == 1) {
			fseek(fp, offset+22, SEEK_SET);
			fwrite(&open, 1, 1, fp);
			return 1;
		} else if (open == 0) {
			fseek(fp, offset+22, SEEK_SET);
			fwrite(&open, 1, 1, fp);
			return 0;	
		}
	} else if (strcmp(what, "start") == 0) {
		int start;
		printf("Enter start: ");
		scanf("%d", &start);
		fseek(fp, offset+26, SEEK_SET);
		fwrite(&start, 2, 1, fp);
	} else if (strcmp(what, "size") == 0) {
		int size;
		printf("Enter size: ");
		scanf("%d", &size);
		fseek(fp, offset+28, SEEK_SET);
		fwrite(&size, 4, 1, fp);
	} else {
		return -1;
	}
	return 1;
}

//given a path ex: /root/dir1/file.txt
//returns the offset for dir1
//ex: /root returns -1
//ex: /root/dir returns 12801
//ex: /root/dir2/file.txt returns starting byte of dir2
int search_parent_dir(char path_string[]) {
	int offset = 12801;

	if (strcmp(path_string, "/root") == 0) {
		return -1;
	}

	int i;
	int slash_count = 0;
	for (i = 0; i < strlen(path_string); i++) {
		if (path_string[i] == '/') {
			slash_count++;
		}
	}
	slash_count--;
	
	if (slash_count == 1) {
		return offset;
	}

	char var[strlen(path_string)];
	strcpy(var, path_string);	

	char* word;
	char* s = "/";
	word = strtok(var, s);
	word = strtok(NULL, s);
	int j = 1;
	int count = 0;
	while (j < slash_count) {
		count = 0;
		printf("word: %s\n", word);
		while (count < 512) { //search directory for file/dir
			if (strcmp(get_entry_name(offset), word) == 0) {
				break;
			}
			offset+=32;		
			count+=32;
		}
		if (count == 512) {
			return -2;
		}
		offset = get_entry_byte(offset);
		word=strtok(NULL,s);
		j++;
	}
	return offset;
}

//creates file or directory from given path
//ex: jcreate("/root/dir1/file.txt") 
int jcreate(char path_string[]) {
	int offset = search_parent_dir(path_string);
	printf("offset: %d\n", offset);
	int first_empty = go_first_empty(offset);

	char filename[8];
	char* last_path = get_last_pathname(path_string);
	printf("last_path: %s\n", last_path);

	char* s = ".";
	char* word;	
	word = strtok(last_path, s);
	strcpy(filename, word);
	printf("filename: %s\n", filename);	

	word = strtok(NULL, s);
	char ext[3];	
	int attr;
	if (word != NULL) {
		strcpy(ext, word);
		attr = 1;
	} else {
		attr = 2;
	}
	printf("ext: %s\n", ext);

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
	int first_fat = locate_first_empty_fat();
	int start = (((first_fat - 513)/3) * 512)+17921;
	
	printf("start: %d\n", start);
	
	init_fat(first_fat);	
	int size = 0;
	
	printf("about to write_entry\n");

	if (attr == 1) {
		write_entry(first_empty, 1, filename, ext, attr, hour, min, day, month, year, 0, start, size);
	} else if (attr == 2) {
		write_entry(first_empty, 1, filename, " ", attr, hour, min, day, month, year, 0, start, size);		
	}
	printf("success for %s\n\n", filename);
	return 1;
}

//looks at the last path name in the path string
//and returns the filename part of the string
//ex: if string is "/root/dir1/file.txt" will return "file"
char* get_last_pathname(char path_string[]) {
	int slash_count = 0;
	int i;
	for (i = 0; i < strlen(path_string); i++) {
		if (path_string[i] == '/') {
			slash_count++;
		}
	}
	slash_count--;

	char var[strlen(path_string)];
	strcpy(var, path_string);
	char* s = "/";
	char* word;	
	word = strtok(var, s);
	int j = 0;
	while (j < slash_count) {
		word=strtok(NULL, s);
		j++;
	}
	
	char* c = malloc(sizeof(char) * 13);
	strcpy(c, word);
	return c;
}

//returns byte of first empty fat entry
int locate_first_empty_fat() {
	int offset = 513;
	fseek(fp, offset, SEEK_SET);
	int empty = 0;
	while (offset < 12288) {
		fread(&empty, 3, 1, fp);
		if (empty == 0) {
			return offset;
		} else {
			offset+=3;
			fseek(fp,offset,SEEK_SET);
		}
	}
	return offset;
}

//initialize fat value to -1 (for jcreate)
void init_fat(int offset) {
	fseek(fp, offset, SEEK_SET);
	int value = -1;
	fwrite(&value, 3, 1, fp);
}

//writes a directory entry (for jcreate)
void write_entry(int offset, int free, char filename[], char ext[], int attr, int hour, int min, int day, int month, int year, int open, int start, int size) {
	fseek(fp, offset, SEEK_SET);
	fwrite(&free, 1, 1, fp);
	fseek(fp, offset+1, SEEK_SET);
	fwrite(filename, sizeof(char), strlen(filename), fp);
	fseek(fp, offset+9, SEEK_SET);
	fwrite(ext, sizeof(char), strlen(ext), fp);
	fseek(fp, offset+12, SEEK_SET);
	fwrite(&attr, sizeof(char), 1, fp);
	fseek(fp, offset+13, SEEK_SET);
	fwrite(&hour, 2, 1, fp);
	fseek(fp, offset+15, SEEK_SET);
	fwrite(&min, 2, 1, fp);
	fseek(fp, offset+17, SEEK_SET);
	fwrite(&day, 2, 1, fp);
	fseek(fp, offset+19, SEEK_SET);
	fwrite(&month, 1, 1, fp);
	fseek(fp, offset+20, SEEK_SET);
	fwrite(&year, 2, 1, fp);
	fseek(fp, offset+22, SEEK_SET);
	fwrite(&open, 1, 1, fp);
	fseek(fp, offset+26, SEEK_SET);
	fwrite(&start, 2, 1, fp);
	fseek(fp, offset+28, SEEK_SET);
	fwrite(&size, 4, 1, fp);	
}

//returns starting byte of last element in the given path
//ex: search("/root/dir1/dir2/dir3");, returns starting byte of dir3.
int search(char path_string[]) { 
	int offset = 12801;
	char* word;
	char var[strlen(path_string)];
	strcpy(var, path_string);
	char* s = "/";
	int count = 0;
	if (strcmp(path_string, "/root") == 0) {
		return offset;
	}
	word = strtok(var, s);
	word = strtok(NULL,s);
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

//returns starting byte of last element's entry in the given path
//ex: search_entry("/root/dir/file.txt");, searches for file.txt in dir
//and then returns the starting byte of file.txt's entry.
int search_entry(char path_string[]) {
	int offset = search_parent_dir(path_string);
	int filename[8];
	fread(filename, sizeof(char), 
}
//given an offset for a directory/file entry,
//returns the offset for the first empty entry found.
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

//given an offset for a directory/file entry,
//returns the 2nd - 9th byte in entry (filename).
char* get_entry_name(int offset) {
	int i = occupied(offset);
//	printf("occupied: %d\n", i);
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

//given an offset for a directory/file entry,
//returns the 26th and 27th byte in entry (start).
int get_entry_byte(int offset) {
	int i = occupied(offset);
	if (i = 1) {
		fseek(fp, offset+26, SEEK_SET);
		int c = 0;
		fread(&c, 2, 1, fp);
		return c;
	} else {
		return -1;	
	}
}

//given an offset for a directory/file entry,
//returns 0 if first byte in entry (free) is free.
//returns 1 if first byte in entry (free) is occupied/not free.
int occupied(int offset) {
	fseek(fp, offset, SEEK_SET);
	int i = 0;
	fread(&i, 1, 1, fp);
	return i;
}
