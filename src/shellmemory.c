#include<stdlib.h>
#include<string.h>
#include<stdio.h>
#include<stdbool.h>

#define SHELL_MEM_LENGTH 1000

int up_used = 0;
//TO DO: Partition the shell memory to have a space for
//frames and a space for variables
void printShellMemory();
struct memory_struct{
	int var;
	char *value;
	int used;
};

struct memory_struct frames[FRAME_STORE_SIZE];
struct memory_struct variables[VAR_STORE_SIZE];

// Helper functions
int match(char *model, char *var) {
	int i, len=strlen(var), matchCount=0;
	for(i=0;i<len;i++)
		if (*(model+i) == *(var+i)) matchCount++;
	if (matchCount == len)
		return 1;
	else
		return 0;
}

char *extract(char *model) {
	char token='=';    // look for this to find value
	char value[1000];  // stores the extract value
	int i,j, len=strlen(model);
	for(i=0;i<len && *(model+i)!=token;i++); // loop till we get there
	// extract the value
	for(i=i+1,j=0;i<len;i++,j++) value[j]=*(model+i);
	value[j]='\0';
	return strdup(value);
}

int using(){
	return up_used++;
}

// Shell memory functions
int update_used(int index){
	frames[index].used = using();
}

void mem_init(){
	int i;
	for (i=0; i<FRAME_STORE_SIZE; i++){		
		frames[i].var = -1;
		frames[i].value = "none";
		frames[i].used = -1;
	}
	for (i=0; i<VAR_STORE_SIZE; i++){		
		variables[i].var = -1;
		variables[i].value = "none";
		variables[i].used = -1;
	}
}

int pg_fault(char *value_1, char *value_2, char *value_3, int* pStart){
	//find the lowest value of used for the page fault
	int index = 0;
	//find the last used frame
	for (int i=0; i<FRAME_STORE_SIZE; i++) {
		if (frames[index].used==-1){
			index++;
			continue;
		}
		if ((frames[i].used<frames[index].used) && (frames[i].used!=-1)) {
			index = frames[i].var*3;
		}
	}
	//print the frame being evicted
	printf("%s\n\n", "Page fault! Victim page contents:");
	printf("%s", frames[index].value);
	printf("%s", frames[index+1].value);
	printf("%s\n", frames[index+2].value);
	printf("%s\n", "End of victim page contents.");
	//store frame number
	int frame_no = frames[index].var;
	*pStart = index;
	//replace the frames
	frames[index].var=frame_no;
	frames[index].value = strdup(value_1);
	frames[index].used = -1;
	frames[index+1].var = frame_no;
	frames[index+1].value = strdup(value_2);
	frames[index+1].used = -1;
	frames[index+2].var = frame_no;
	frames[index+2].value = strdup(value_3);
	frames[index+2].used = -1;
	return frame_no;
}

//DONE: create a function that adds frames and returns the frame number added at
int add_frames(char *value_1, char *value_2, char *value_3, int* pStart, int* pEnd) {
	int i;
	int frame_no=0;
	for (i=0; i<FRAME_STORE_SIZE; i++) {
		if (i%3==0 && i!=0) frame_no++;
		if (frames[i].var==-1){
			//empty space found
			*pStart = (int)i;
			frames[i].var = frame_no;
			frames[i].value = strdup(value_1);
			frames[i+1].var = frame_no;
			frames[i+1].value = strdup(value_2);
			frames[i+2].var = frame_no;
			frames[i+2].value = strdup(value_3);
			//TO DO: STORE THE VARIABLES SOMEHOW
			return frame_no;
		}
	}

	//TO DO: find the least recently used frame and evict it
	//TO DO: print in terminal with the page
	//TO DO: add frames to this spot
	return pg_fault(value_1, value_2, value_3, pStart);
}

//DONE: create a function that takes a frame no and returns a pointer to the first line
char* find_frame(int index) {
	return frames[index].value;
}

//DONE: takes a frame number and returns the index it starts at
int find_frame_no(int frame_no){
	int i;
	for (i=0; i<FRAME_STORE_SIZE; i++){
		if (frames[i].var == frame_no){
			return i;
		}
	}
}



// Set key value pair
void mem_set_value(char *v_in, char *value_in) {
    int i;
	int var_in = atoi(v_in);
    for (i=0; i<1000; i++){
        if (frames[i].var==var_in){
            frames[i].value = strdup(value_in);
            return;
        } 
    }

    //Value does not exist, need to find a free spot.
    for (i=0; i<1000; i++){
        if (frames[i].var==-1){
            frames[i].var = var_in;
            frames[i].value = strdup(value_in);
            return;
        } 
    }

    return;

}

//get value based on input key
char *mem_get_value(char *v_in) {
	int i;
	int var_in = atoi(v_in);
	for (i=0; i<1000; i++){
		if (frames[i].var == var_in){
			return strdup(frames[i].value);
		} 
	}
	return NULL;

}



void printShellMemory(){
	int count_empty = 0;
	for (int i = 0; i < FRAME_STORE_SIZE; i++){
		if(frames[i].var == -1){
			count_empty++;
		}
		else{
			printf("\nline %d: key: %d\t\tvalue: %s\n", i, frames[i].var, frames[i].value);
		}
    }
	printf("\n\t%d lines in total, %d lines in use, %d lines free\n\n", SHELL_MEM_LENGTH, SHELL_MEM_LENGTH-count_empty, count_empty);
}

/*
 * Function:  addFileToMem 
 * 	Added in A2
 * --------------------
 * Load the source code of the file fp into the shell memory:
 * 		Loading format - var stores fileID, value stores a line
 *		Note that the first 100 lines are for set command, the rests are for run and exec command
 *
 *  pStart: This function will store the first line of the loaded file 
 * 			in shell memory in here
 *	pEnd: This function will store the last line of the loaded file 
 			in shell memory in here
 *  fileID: Input that need to provide when calling the function, 
 			stores the ID of the file
 * 
 * returns: error code, 21: no space left*/
 
/*int load_file(FILE* fp, int* pStart, int* pEnd, char* filename)
{
	char *line;
    size_t i;
    int error_code = 0;
	bool hasSpaceLeft = false;
	bool flag = true;
	i=101;
	size_t candidate;
	while(flag){
		flag = false;
		for (i; i < FRAME_STORE_SIZE; i++){
			if(frames[i].var == -1){
				*pStart = (int)i;
				hasSpaceLeft = true;
				break;
			}
		}
		candidate = i;
		for(i; i < FRAME_STORE_SIZE; i++){
			if(frames[i].var != -1){
				flag = true;
				break;
			}
		}
	}
	i = candidate;
	//shell memory is full
	if(hasSpaceLeft == 0){
		error_code = 21;
		return error_code;
	}
    
    for (size_t j = i; j < SHELL_MEM_LENGTH; j++){
        if(feof(fp))
        {
            *pEnd = (int)j-1;
            break;
        }else{
			line = calloc(1, SHELL_MEM_LENGTH);
            fgets(line, 999, fp);
			frames[j].var = 0;
            frames[j].value = strndup(line, strlen(line));
			free(line);
        }
    }

	//no space left to load the entire file into shell memory
	if(!feof(fp)){
		error_code = 21;
		//clean up the file in memory
		for(int j = 1; i <= SHELL_MEM_LENGTH; i ++){
			frames[j].var = -1;
			frames[j].value = "none";
    	}
		return error_code;
	}
	//printShellMemory();
    return error_code;
}
*/


char * mem_get_value_at_line(int index){
	if(index<0 || index > FRAME_STORE_SIZE) return NULL; 
	return frames[index].value;
}

void mem_free_lines_between(int start, int end){
	for (int i=start; i<=end && i<FRAME_STORE_SIZE; i++){
		if(frames[i].value != NULL){
			free(frames[i].value);
		}	
		frames[i].var = -1;
		frames[i].value = "none";
	}
}
