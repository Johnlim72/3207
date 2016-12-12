#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

FILE* fp; //Pointer to virtual disk.

int size_dir_entry = 32; //Size of one directory entry.
int size_fat_entry = 3; //Size of one FAT entry.
int size_block = 512; //Size of each data block.
int num_blocks = 4096; //Total number of blocks in virtual disk.
int num_fats = 1; //Number of FATs.
int num_root = 1; //Number of root directories.
int max_size = 32768; //Max file size in bytes.
int where_fat = 513; //Starting byte offset for the FAT. (24 total blocks).
int where_root = 12801; //Starting byte offset for the root directory. (10 total blocks).
int where_data = 17921; //Starting byte offset for the data blocks.
int disk_size = 2; //Size of the virtual disk in megabytes.

void metadata();
int jcreate(char path_string[]);
int jopen(char path_string[]);
int jclose(char path_string[]);
int jdelete(char path_string[]);
int jwrite(char path_string[], char buffer[]);
char* jread(char path_string[], int size, char buffer[]);

void write_entry(int offset, int free, char filename[], char ext[], int attr, int hour, int min, int day, int month, int year, int open, int start, int size);
int go_first_empty(int offset);
int look_for_same(char path_string[]);
int update(char path_string[], int start, int length);

void edit_fat(int offset, int new_fat);
int locate_first_empty_fat();
void init_fat(int offset);
void delete_fat(int offset);
void format(int offset);

int print_dir(char path_string[]);
int search_dir(char path_string[]);
int search_parent_dir(char path_string[]);
int search_entry(char path_string[]);

char* get_last_pathname(char path_string[]);
char* get_entry_name(int offset);
int get_start(int offset);
int get_open(int offset);
int get_attr(int offset);
int get_size(int offset);
int occupied(int offset);

char* error = "error";

int main() {
	//Open virtual disk; Label virtual disk "John Lim's File System.
	fp = fopen("output.dat", "r+");
	//Write metadata for filesystem.
	metadata();
	
	//******************TESTS******************
	jcreate("/root/file1.txt");
	jcreate("/root/dir1");
	jcreate("/root/dir2");
	jcreate("/root/dir1/file2.txt");
	jcreate("/root/dir2/file3.txt");
	jcreate("/root/dir1/file4.txt");
	jcreate("/root/dir1/file5.txt");
	jcreate("/root/dir1/file6.txt");
	jopen("/root/dir1/file4.txt");

	char* buffer1 = "f m t l e p b e u a q v b y y j a q j y n k g f i s p e z p u v h e m t o n j t p q y y y n u s e f a k o q v n b e m l x a p z u i s d w k u y z z l k x y m y e o v j s z o m n a n s n v s z m m o m a v w z f h b y c t k i q j a b z l w j s n l d k b z c q e z f n o x a h j y n k i v d h n z g r b k w x v k r s v v h f v i w y o e b x n x q q f v z c a x w x d w o n d a m a l v v l u b c c a v h q d c u x i g k b q v d s c g s h a y g f z b t a y e n e b d x i x m j h f y n l k r s w u x o r o h o s u a m r b w r z h q c r u j l c m p l c o z a b g i i o l t g m c d u l j f g o z l j s j x p o r m u p c w o w y k t z y a s p j u h a c e h v k z n b i b i c e y i d o w y a e y r s w y c n v a w h g d d i s r m v n v i z d s c u s m t c r r w m f b q m s y z q h c p h b o f g l i l p v i m g e p p p s s r a u c u h r j z z x o j v f u z r s m a c p q l g y b a e n o j s c f z f r m q w y u u s h c p i d m t r r y u w i u k w u l n d v a z e r p a b j r m y e u z m f s r b b d h e g o q b t b j e s u h q q i h i g a b a w w l h q u w m m a r j d j t k t c t k b z t i y o g r w s f r r x r x s f h q b j u o o g m l u c g i m r w j w y c l p o o f z s g m o r e m j w v h u s r m b t o y i t l t r r v k m d v x d w p s k u o x h w b b c f x u z e w s j h c x m b l u c g h b e b t m e t m z r x c o b a y e i p m v t b q o f m a k c s v t w h r c i f r b u o w r e v z q j j t w d p l j e o u x q u k m a r n a n l u g o r q r o y l n w n m k y a a k r j p e e f s b x r v r q w m w f k a t m v r o z p d m u z p h s c f u y b n k q i m n i f m v x c r f u j b n j i v t c o m q o r c i d e z c x m z c f p r s y n p h f k e k c b e q f x h k g p r h h i y e w i m h v o p n g v t d g b r w z r r c b b c s k e r n g d v x r h j s s m g z f k o j d j m r r u h a y v u y a d z s x g b t c k x u j p o u g u e a y i t b j q q v c g o d d d d b x q z v l v n y w k n j x q f s m q l v d o a r a o o r k i e c j g r a s b x h z g a q t p q t r g a n z r k r w m b c b x w p b k s s p t z ";	

	jwrite("/root/dir1/file4.txt", buffer1); 

	char buffer3[10000];
		
	jread("/root/dir1/file4.txt", 600, buffer3);	
	printf("buffer3: %s\n", buffer3);
	
	jdelete("/root/dir1/file4.txt");
	jcreate("/root/dir1/file7.txt");
	
	int offset = print_dir("/root/dir1");
	
	return 1;
}

//Writes the metadata for the filesystem.
void metadata() {
	char *str = "John Lim's File System";
	fwrite(str, sizeof(char), 22, fp);
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
	fseek(fp, 104, SEEK_SET);
	fwrite(&disk_size, sizeof(int), 1, fp);
}

//Creates the file/directory (given the full path including the file/directory).
int jcreate(char path_string[]) {
	//Get the byte offset of the parent directory of the file/directory to be created.
	int offset = search_parent_dir(path_string);
		
	//Check the directory if the file/subdirectory is already created.
	int same = look_for_same(path_string);
	if (same == 1) {
		return -1;
	}
	
	//Get the byte offset of the first empty entry in the directory.
	//Find the byte offsets for the first empty FAT entry and data block.
	//Initialize FAT entry to -1.
	
	int first_empty = go_first_empty(offset);
	if (first_empty == -1 ) {
		return -1;
	} 
	int first_fat = locate_first_empty_fat();
	int start = (((first_fat - 513)/3) * 512)+17921;	
	init_fat(first_fat);
	

	//Initialize the size to 0.
	int size = 0;
	
	//Get the filename (filename length can't be greater than 8).
	char filename[8];
	char* last_path = get_last_pathname(path_string);
	char* s = ".";
	char* word;	
	word = strtok(last_path, s);
	strcpy(filename, word);	
	if (strlen(filename) > 8) {
		return -1;
	}

	//Get the extension (extension length can't be greater than 3).
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
	if (strlen(ext) > 3) {
		return -1;
	}

	//Get the time and date.
	time_t now;
	struct tm *now_tm;	
	now = time(NULL);
	now_tm = localtime(&now);
	int hour = now_tm->tm_hour;
	int min = now_tm->tm_min;
	int day = now_tm->tm_mday;
	int month = (now_tm->tm_mon) + 1;
	int year = ((now_tm->tm_year) - 100);

	//Write the directory entry for the file/directory.
	write_entry(first_empty, 1, filename, ext, attr, hour, min, day, month, year, 0, start, size);		
	
	return 1;
}

//Opens the file (Given the full path including file).
int jopen(char path_string[]) {
	int open = 1;
	int offset = search_entry(path_string);
	fseek(fp, offset+22, SEEK_SET);
	fwrite(&open, 1, 1, fp);
	return 1;
}

//Closes the file (Given the full path including file).
int jclose(char path_string[]) {
	int close = 0x00;
	int offset = search_entry(path_string);
	fseek(fp, offset+22, SEEK_SET);
	fwrite(&close, 1, 1, fp);
	return 1;
}

//Deletes the file (Given the full path including file).
int jdelete(char path_string[]) {
	//Get the byte offset for the file's FAT entry and data block.
	int offset = search_entry(path_string); 
	int curr_block = get_start(offset);
	int curr_fat = (((curr_block - 17921)/512)*3) + 513;
	fseek(fp, curr_fat, SEEK_SET);

	//Delete each FAT entry and format each data block after the first FAT entry and first data block.
	int next_fat = 0;
	fread(&next_fat, 3, 1, fp);
	int neg_one = 16777215;
	while (next_fat != neg_one) {
		curr_fat = next_fat;
		fseek(fp, curr_fat, SEEK_SET);	
		fread(&next_fat, 3, 1, fp);
		curr_block = (((curr_fat - 513)/3) * 512)+17921;
		format(curr_block);
		delete_fat(curr_fat);
	}

	//Delete the first FAT entry and first data block.
	offset= search_entry(path_string);
	curr_block = get_start(offset);
	curr_fat = (((curr_block - 17921)/512)*3) + 513;
	delete_fat(curr_fat);
	format(curr_block);

	//Delete the directory entry for the file
	offset = search_entry(path_string);
	fseek(fp, offset, SEEK_SET);
	int i = 0;
	int zero = 0x00;
	while (i < 32) {
		fwrite(&zero, 1, 1, fp);
		i++;
	}
	return 1;
}

//Reads a file (Given the full path name, amount of bytes to be read, and the array for buffer).
char* jread(char path_string[], int size, char buffer[]) {
	//Get the starting byte offset of the file.
	int offset = search_entry(path_string);
	//Get the size of the file.
	int file_size = get_size(offset);	
	//Check if file is closed.
	if (get_open(offset) == 0) {
		return error;
	}
	//Check if size is greater than max file size.
	if (size >= 32768) {
		return error;
	}
	//Get starting byte offset of the file's starting data block and FAT entry.
	int curr_block = get_start(offset);
	int curr_fat = (((curr_block  - 17921)/512)*3) + 513;
	fseek(fp, curr_block, SEEK_SET);

	//Reads the file character by character.
	int i = 0;
	int end_block = 0;
	int next_fat;
	while (i < size && i != file_size) {
		buffer[i] = getc(fp);
		i++;
		end_block++;
		//Go to next data block if file size greater than 512 bytes.
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

//Write a file (Given the full path including file name and array of elements to be written).
//Overwrites file if data is already written in file and then writes new elements.
int jwrite(char path_string[], char buffer[]) {
	//Get the offset of the file's directory entry.
	int offset = search_entry(path_string); 
	//Check if file is closed.
	if (get_open(offset) == 0) {
		return -1;
	}
	//Check if file is greater than max file size.
	if (strlen(buffer) >= 32768) {
		return -1;
	}

	//Delete the file, in order to be overwritten.
	int curr_block = get_start(offset);
	int curr_fat = (((curr_block - 17921)/512)*3) + 513;
	fseek(fp, curr_fat, SEEK_SET);
	int next_fat = 0;
	fread(&next_fat, 3, 1, fp);
	int neg_one = 16777215;
	delete_fat(curr_fat);
	format(curr_block);
	while (next_fat != neg_one) {
		curr_fat = next_fat;
		fseek(fp, curr_fat, SEEK_SET);	
		fread(&next_fat, 3, 1, fp);
		curr_block = (((curr_fat - 513)/3) * 512)+17921;
		format(curr_block);
		delete_fat(curr_fat);
	}

	//Find first empty FAT entry and data block for file to be written in.
	int new_fat = locate_first_empty_fat();
	fseek(fp, new_fat, SEEK_SET);
	int first_block = (((new_fat - 513)/3) * 512) + 17921;
	fseek(fp, first_block, SEEK_SET);
	//Write the file
	double new_block_count = 0;
	int end_block = 0;
	int i = 0;
	while (i < strlen(buffer)) {
		putc(buffer[i], fp);
		i++;
		end_block++;
		//Find new data block if current data block reaches max data block size.
		if (end_block == 512) {
			edit_fat(new_fat, 1); //Temporary for next line's purpose.
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
	//Update time, data, and file size.
	update(path_string, first_block, strlen(buffer));
	return 1;
}

//Formats data block (Given starting byte offset for data block).
void format(int offset) {
	fseek(fp, offset, SEEK_SET);
	int i = 0;
	char var = 0x00;
	while (i < 512) {
		fwrite(&var, 1, 1, fp);
		i++;
	}
}

//Deletes FAT entry (Given starting byte offset for FAT entry).
void delete_fat(int offset) {
	fseek(fp, offset, SEEK_SET);
	char var = 0x00;
	fwrite(&var, 1, 1, fp);
	fwrite(&var, 1, 1, fp);
	fwrite(&var, 1, 1, fp);
}

//Gives FAT entry a new value.
void edit_fat(int offset, int new_fat) {
	int fat = new_fat;
	fseek(fp, offset, SEEK_SET);
	fwrite(&fat, 3, 1, fp);	
}

//Finds starting byte offset of directory (Given the full path name including directory name).
int search_dir(char path_string[]) {
	int offset = 12801;
	if (strcmp(path_string, "/root") == 0) {
		return offset;
	}

	int i;
	int slash_count = 0;
	for (i = 0; i < strlen(path_string); i++) {
		if (path_string[i] == '/') {
			slash_count++;
		}
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
		//Search current directory for the subdirecoty wanted to be found.
		while (count < 512) { 
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

//Prints all subdirectories and files within a directory.
int print_dir(char path_string[]) {
	int offset = search_dir(path_string);

	int j = 1;
	char* word;
	while (j <= 16) {
		word = get_entry_name(offset);
		if (strcmp(error, word) != 0) {
			printf("%s ", word);
		}
		int start = get_start(offset);
		if (start != -1) {
			printf("%d\n", start); 
		}
		offset+=32;
		j++;
	}
	return 1;
}

//Find starting byte offset of the file/directory's parent directory.
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

//Checks the directory if there is a file/subdirectory with the same name.
int look_for_same(char path_string[]) {
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
			return 1;
		}
		count+=32;
		offset+=32;
	}
	if (count == 512) {
		return -1;
	}
	return -1;
}

//Gets the last name in the path.
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

//Finds the first empty FAT entry.
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

//Initializes the FAT entry to -1. (When creating a file).
void init_fat(int offset) {
	fseek(fp, offset, SEEK_SET);
	int value = -1;
	fwrite(&value, 3, 1, fp);
}

//Writes a directory entry (Given the starting byte offset for the entry).
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

//Updates time, data, starting byte and size for a file and it's parent directories (when done writing a file).
int update(char path_string[], int start, int length) {
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
	int starting = start;
	offset = search_entry(path_string);
	fseek(fp, offset+26, SEEK_SET);
	fwrite(&starting, 2, 1, fp);
	fseek(fp, offset+28, SEEK_SET);
	fwrite(&size, 4, 1, fp);

	return 1;
}

//Searches for the starting byte offset of a file/directory's entry.
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

//Gets the starting byte offset of first empty directory entry (given the starting byte offset of the directory).
int go_first_empty(int offset) {
	int i = occupied(offset);
	int count = 0;
	int max;
	if (offset == 12801) {
		max = 5120;
	} else {
		max = 512;
	}
	while (count < max) {
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

//Gets the name of a file/directory (given the starting byte offset of the file/directory's entry).
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

//Gets the starting byte offset of the file/directory's data block (given the starting byte offset of the file/directory's entry).
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

//Gets the byte that tells if the file is open/closed (given the starting byte offset of the file/directory's entry).
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

//Gets the bytes that respresent the size of a file (given the starting byte offset of the file/directory's entry).
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

//Gets the byte that respresnet the attribute of a file (given the starting byte offset of the file/directory's entry).
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

//Checks if the entry is being used or empty (given the starting byte offset of the file/directory's entry).
int occupied(int offset) {
	fseek(fp, offset, SEEK_SET);
	int i = 0;
	fread(&i, 1, 1, fp);
	return i;
}
