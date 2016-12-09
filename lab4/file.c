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
int jdelete(char path_string[]);
int jwrite(char path_string[], char buffer[]);
char* jread(char path_string[], int size, char buffer[]);
int jclose(char path_string[]);
void delete_fat(int offset);
void format(int offset);
char* get_last_pathname(char path_string[]);
void edit_fat(int offset, int new_fat);
int update(char path_string[], int length);
int search_parent_dir(char path_string[]);
int search_entry(char path_string[]);
char* get_entry_name(int offset);
int get_start(int offset);
int go_first_empty(int offset);
int get_open(int offset);
int get_attr(int offset);
int get_size(int offset);
int occupied(int offset);

char* error = "error";

int main() {
	//Initialize meta data's first line: name of the file system.
	fp = fopen("output.dat", "r+");
	char *str = "John Lim's File System";
	fwrite(str, sizeof(char), 22, fp);

	metadata();
	
	jcreate("/root/file1.txt");
	jcreate("/root/dir1");
	jcreate("/root/dir2");
	jcreate("/root/dir1/file2.txt");
	jcreate("/root/dir2/file3.txt");
	jcreate("/root/dir1/file4.txt");
	jcreate("/root/dir1/file5.txt");
	jcreate("/root/dir1/file6.txt");
	jopen("/root/dir1/file4.txt");

	char* buffer2 = "hello my name is john lim i am a computer science major, monil bid is sitting next to me, next to him is jae kim, i am in operating systems class right now hello computer i am typing on a keyboard, jae is working on 2166, i just spent fucking 700 dollars today to go to electric forest in june everyday i spend my time dark runescape willow yew oak copper tin rune dragon green hide hai dang mac miller sup yo lmao ello guva shut the fuck up yo lmaoooooo ayyyyyyyyyyyyyyy firetruck watergun flamethrower japan korea crazy bitch shes so mean gorilla monkey giraffe af addison 2033 david dobor hi my name is john again lol im going to play basketball later and fucking merk peoples ankles lol like kyrie irving on the cavaliers fuck KD fuck the warriors lmaooooo ight yo i need to chill out lowkey im fucking smacked ight lets fucking get this quiz going ya heard lmao hoe shut the fuck up i got way too much on my mental SUP yo i just want this semester to be over like im so over it i need to just chilll and do nothing for a month i dont wanna do anything anymore this lab is crazy hard and long i cant believe im almost done with it lol end ";	
	jwrite("/root/dir1/file4.txt", buffer2); 

	char buffer3[10000];
		
	jread("/root/dir1/file4.txt", 600, buffer3);	
	
	printf("buffer3: %s", buffer3);
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
	int open = 1;
	int offset = search_entry(path_string);
	fseek(fp, offset+22, SEEK_SET);
	fwrite(&open, 1, 1, fp);
	return 1;
}

int jclose(char path_string[]) {
	int close = 0x00;
	int offset = search_entry(path_string);
	fseek(fp, offset+22, SEEK_SET);
	fwrite(&close, 1, 1, fp);
	return 1;
}

int jdelete(char path_string[]) {
	
}

char* jread(char path_string[], int size, char buffer[]) {
	int offset = search_entry(path_string);
	int file_size = get_size(offset);

	if (get_open(offset) == 0) {
		return error;
	}
	if (size >= 32768) {
		return error;
	}
	
	int curr_block = get_start(offset);
	int curr_fat = (((curr_block  - 17921)/512)*3) + 513;
	
	fseek(fp, curr_block, SEEK_SET);
	int i = 0;
	int end_block = 0;
	int next_fat;
	while (i < size && i != file_size) {
		buffer[i] = getc(fp);
		i++;
		end_block++;
		
		if (end_block == 512) {
			fseek(fp, curr_fat, SEEK_SET);
			fread(&next_fat, 3, 1, fp);
			fseek(fp, next_fat, SEEK_SET);
			curr_block = (((next_fat - 513)/3) * 512) + 17921;
			fseek(fp, curr_block, SEEK_SET);
			end_block = 0;
		}
	}
	return buffer;
}

int jwrite(char path_string[], char buffer[]) {
	int offset = search_entry(path_string); 
	
	if (get_open(offset) == 0) {
		return -1;
	}
	if (strlen(buffer) >= 32768) {
		return -1;
	}

	//go to the file's 1st fat byte
	int curr_block = get_start(offset);
	int curr_fat = (((curr_block - 17921)/512)*3) + 513;
	fseek(fp, curr_fat, SEEK_SET);

	//format the files.
	int next_fat = 0;
	fread(&next_fat, 3, 1, fp);
	int neg_one = 16777215;
	delete_fat(curr_fat);
	format(curr_block);

	while (next_fat != neg_one) {
		curr_fat = next_fat;
		fseek(fp, curr_fat, SEEK_SET);	
		fread(&next_fat, 3, 1, fp);
		curr_block = (((next_fat - 513)/3) * 512)+17921;
		format(curr_block);
		delete_fat(curr_fat);
	}

	int new_fat = locate_first_empty_fat();
	fseek(fp, new_fat, SEEK_SET);
	int first_block = (((new_fat - 513)/3) * 512) + 17921;
	fseek(fp, first_block, SEEK_SET);

	double new_block_count = 0;
	int end_block = 0;
	int i = 0;
	while (i < strlen(buffer)) {
		putc(buffer[i], fp);
		i++;
		end_block++;

		if (end_block == 512) {
			edit_fat(new_fat, 1); //temporary for next line's purpose.
			int next_new_fat = locate_first_empty_fat();
			edit_fat(new_fat, next_new_fat);
			edit_fat(next_new_fat, neg_one);
			new_fat = next_new_fat;
			int start_of_new_block = (((next_new_fat - 513)/3) * 512)+17921;
			fseek(fp, start_of_new_block, SEEK_SET);
			end_block = 0;		
			new_block_count++;
		}
	}
	
	if (new_block_count == 0) {
		edit_fat(new_fat, neg_one);
	}

	update(path_string, strlen(buffer));
	return 1;
}
 

//given a offset for a data block
void format(int offset) {
	fseek(fp, offset, SEEK_SET);
	int i = 0;
	char var = 0x00;
	while (i < 512) {
		fwrite(&var, 1, 1, fp);
		i++;
	}
}

void delete_fat(int offset) {
	fseek(fp, offset, SEEK_SET);
	char var = 0x00;
	fwrite(&var, 1, 1, fp);
	fwrite(&var, 1, 1, fp);
	fwrite(&var, 1, 1, fp);
}

void edit_fat(int offset, int new_fat) {
	int fat = new_fat;
	fseek(fp, offset, SEEK_SET);
	fwrite(&fat, 3, 1, fp);	
}

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
		offset = get_start(offset);
		word=strtok(NULL,s);
		j++;
	}
	return offset;
}

//creates file or directory from given path
//ex: jcreate("/root/dir1/file.txt") 
int jcreate(char path_string[]) {
	int offset = search_parent_dir(path_string);
	printf("offset for parent dir: %d\n", offset);
	int first_empty = go_first_empty(offset);

	char filename[8];
	char* last_path = get_last_pathname(path_string);
	printf("last_path name: %s\n", last_path);

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
		strcpy(ext, " ");
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
	
	printf("start of file/dir: %d\n", start);
	
	init_fat(first_fat);	
	int size = 0;

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
	return -1;
}

//initialize fat value to -1 (for jcreate)
void init_fat(int offset) {
	fseek(fp, offset, SEEK_SET);
	int value = -1;
	fwrite(&value, 3, 1, fp);
}

//writes a directory entry (for jcreate) can be file/dir
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

int update(char path_string[], int length) {
	int offset = 12801;

	char* last_path = get_last_pathname(path_string);
	last_path = strtok(last_path, ".");
	
	time_t now;
	struct tm *now_tm;	
	now = time(NULL);
	now_tm = localtime(&now);
	int hour = now_tm->tm_hour;
	int min = now_tm->tm_min;
	int day = now_tm->tm_mday;
	int month = (now_tm->tm_mon) + 1;
	int year = ((now_tm->tm_year)) - 100;

	int i;
	int slash_count = 0;
	for (i = 0; i < strlen(path_string); i++) {
		if (path_string[i] == '/') {
			slash_count++;
		}
	}
	char* word;
	char var[strlen(path_string)];
	strcpy(var, path_string);
	char* s = "/";
	int count = 0;
	word = strtok(var, s);
	word = strtok(NULL, s);
	int j = 2;
	while (j <= slash_count) {
		count = 0;
		while (count < 512) {
			if (j == slash_count) {
				if (strcmp(get_entry_name(offset), last_path) == 0) {
					break;
				}	
			} else {
				if (strcmp(get_entry_name(offset), word) == 0) {
					break;
				}
			}
			offset+=32;		
			count+=32;
		}
		if (count == 512) {
			return -1;
		}
		int current = offset;
		
		fseek(fp, offset, SEEK_SET);
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
	
		offset = get_start(current);
		word=strtok(NULL,s);
		j++;
	}
	int size = length;
	offset = search_entry(path_string);
	fseek(fp, offset+28, SEEK_SET);
	fwrite(&size, 4, 1, fp);

	return 1;
}

//returns starting byte of last element's entry in the given path
//ex: search_entry("/root/dir/file.txt");, searches for file.txt in dir
//and then returns the starting byte of file.txt's entry.
int search_entry(char path_string[]) {
	int offset = search_parent_dir(path_string);
	char* last_path = get_last_pathname(path_string);
		
	char filename[8];
	char* s = ".";
	char* word;	
	word = strtok(last_path, s);
	strcpy(filename, word);
	
	int count = 0;
	while (count < 512) {
		if (strcmp(get_entry_name(offset), filename) == 0) {
			break;
		}
		count+=32;
		offset+=32;
	}
	if (count == 512) {
		return -1;
	}
	return offset;
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
	if (i == 1) {
		fseek(fp, offset+1, SEEK_SET);
		char c[8] = " ";
		fread(c, sizeof(char), 8, fp);
		return c;
	} else {
		return error;
	}
}

//given an offset for a directory/file entry,
//returns the 26th and 27th byte in entry (start).
int get_start(int offset) {
	int i = occupied(offset);
	if (i == 1) {
		fseek(fp, offset+26, SEEK_SET);
		int c = 0;
		fread(&c, 2, 1, fp);
		return c;
	} else {
		return -1;	
	}
}

int get_open(int offset) {
	int i = occupied(offset);
	if (i == 1) {
		fseek(fp, offset+22, SEEK_SET);
		int c = 0;
		fread(&c, 1, 1, fp);
		return c;
	} else {
		return -1;
	}	
}

int get_size(int offset) {
	int i = occupied(offset);
	if (i == 1) {
		fseek(fp, offset+28, SEEK_SET);
		int c = 0;
		fread(&c, 4, 1, fp);
		return c;
	} else {
		return -1;
	}
}

int get_attr(int offset) {
	int i = occupied(offset);
	if (i == 1) {
		fseek(fp, offset+12, SEEK_SET);
		int c = 0;
		fread(&c, 1, 1, fp);
		return c;
	} else {
		return -1;
	}
}

int occupied(int offset) {
	fseek(fp, offset, SEEK_SET);
	int i = 0;
	fread(&i, 1, 1, fp);
	return i;
}
