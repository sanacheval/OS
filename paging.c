#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include "pcb.h"
#include "kernel.h"
#include "shell.h"
#include "shellmemory.h"
#include "interpreter.h"
#include "ready_queue.h"
#include <dirent.h>
#include <sys/stat.h>

int separate_lines(char *ui, char *line1, char *line2) {
    int a = 0;
    int b = 0;
    int c = 0;
    //check if the line has a ';'
    while (a<strlen(ui)) {
        if (ui[a]==';'){
            //it has a ';'
            //break into two lines
            while (b<a){
                //first line before the ;
                line1[b]=ui[b];
                b++;
            }
            a++;
            a++;
            while (a<strlen(ui)) {
                //second line after ';'
                line2[c]=ui[a];
                c++;
                a++;
            }
            return 1;
        }
        a++;
    }
    return 0;
}

//store one page
int store_page(FILE *filename, int *start, int *frame1){
    char l[1000];
    char *line1 = (char*)malloc(1000);
    char *line2 = (char*)malloc(1000);
    char *line3 = (char*)malloc(1000);
    int separated;
    //take the first line
    fgets(l, 999, filename);
    //check if the line has a ';'
    separated = separate_lines(l, line1, line2);
    if (separated){
        //two lines in the first l
        //take the next line
        fgets(line3, 999, filename);
    }
    else {
        line1 = strdup(l);
        //only one line in the first line
        if (feof(filename)) {
            line2 = strdup("none");
            line3 = strdup("none");
        }
        else {
            fgets(l, 999, filename);
            //check if there are two lines in the second line
            separated =  separate_lines(l, line2, line3);
            if (!separated){
                line2 = strdup(l);
                if (feof(filename)) {
                        line3 = strdup("none");
                }
                else { 
                    fgets(line3, 999, filename);
                }
            }
        }
    }
    //store first three lines
    *frame1 = add_frames(line1, line2, line3, start);
    free(line1);
    free(line2);
    free(line3);
    return 0;
}

//storing pages
int store_two_pages(FILE *filename, int *start, int *frame1, int *frame2){
    char l[1000];
    char *line1 = (char*)malloc(1000);
    char *line2 = (char*)malloc(1000);
    char *line3 = (char*)malloc(1000);
    char *line4 = (char*)malloc(1000);
    char *line5 = (char*)malloc(1000);
    char *line6 = (char*)malloc(1000);
    int *second = (int*)malloc(sizeof(int));
    int separated;
    //take the first line
    fgets(l, 999, filename);
    //check if the line has a ';'
    separated = separate_lines(l, line1, line2);
    if (separated){
        //two lines in the first l
        //take the next line
        fgets(line3, 999, filename);
    }
    else {
        line1 = strdup(l);
        //only one line in the first line
        if (feof(filename)) {
            line2 = strdup("none");
            line3 = strdup("none");
        }
        else {
            fgets(l, 999, filename);
            //check if there are two lines in the second line
            separated =  separate_lines(l, line2, line3);
            if (!separated){
                line2 = strdup(l);
                if (feof(filename)) {
                        line3 = strdup("none");
                }
                else { 
                    fgets(line3, 999, filename);
                }
            }
        }
    }
    //store first three lines
    *frame1 = add_frames(line1, line2, line3, start);
    if (feof(filename)){
        *frame2 = -1;
    }
    else {
        //move to the next line
        fgets(l, 999, filename);
        //check if two
        separated = separate_lines(l, line4, line5);
        if (separated){
            //two lines in the first l
            //take the next line
            fgets(line6, 999, filename);
        }
        else {
            line4 = strdup(l);
            if (feof(filename)) {
                line5 = strdup("none");
                line6 = strdup("none");
            }
            else {
                //only one line in the first l
                fgets(l, 999, filename);
                //check second line for two lines
                separated =  separate_lines(l, line5, line6);
                if (!separated){
                    line5 = strdup(l);
                    //only one line in second line
                    //store third line
                    if (feof(filename)) {
                        line6 = strdup("none");
                    }
                    else { 
                        fgets(line6, 999, filename);
                    }
                }
            }
        }
        //add next frames
        *frame2 = add_frames(line4, line5, line6, second);
    }
    free(line1);
    free(line2);
    free(line3);
    free(line4);
    free(line5);
    free(line6);
    free(second);
    return 0;

}

//adding a file to the backing store
int bckstr_file(char *filename) {
    FILE* fp;
    int error_code = 0;
    //open the file
    fp = fopen(filename, "rt");
    //check if the file doesnt exist
    if(fp == NULL){
        error_code = 11; // 11 is the error code for file does not exist
        return error_code;
    }
    //check if the file already exists, then add it with a different name
    //open the directory
    DIR* dr = opendir("backing_store");
    //check if it exists
    if (dr == NULL) {
        return 11; //backing store doesnt exist
    }
    char* command;
    int* start = (int*)malloc(sizeof(int));
    int* frame1 = (int*)malloc(sizeof(int));
    int* frame2 = (int*)malloc(sizeof(int));
    int pagetable[10];
    FILE* backing_file;
    int found = 0;
    int i = 0;
    struct dirent* entry;
    //read the backing store
    while ((entry = readdir(dr))!=NULL){
        if (strcmp(entry->d_name, filename)==0) {
            //the filename already exists in the backing store
            i++;
            found = 1;
        }
    }
    //if the filename exists create a new filename and store the file to that
    if (found) {
        char *new_filename;
        //create a new filename
        sprintf(new_filename, "%s_%d", filename, i);
        char *cmd = (char*) calloc(1, 9+strlen(new_filename)+strlen("backing_store")); 
        sprintf(cmd, "touch %s/%s", "backing_store", new_filename);
        //create a file with the new filename in the backing store
        error_code = system(cmd);
        free(cmd);
        int total = strlen(filename)+strlen("backing_store")+strlen(new_filename)+4;
        command = (char*) calloc(1, 9+total);
        strncat(command, "cp ", 9);
        strncat(command, filename, strlen(filename));
        strncat(command, " ", 2);
        strncat(command, "backing_store ", strlen("backing_store")+2);
        strncat(command, new_filename, strlen(new_filename));
        //copy the contents of the original file to the new file in backing store
        error_code = system(command);
        free(command);
        command = (char*) calloc(1, 2+strlen(new_filename)+strlen("backing_store/"));
        strncat(command, "backing_store/", strlen("backing_store/")+1);
        strncat(command, new_filename, strlen(new_filename)+1);
        //open the file in the backing store
        backing_file = fopen(command, "rt");
        free(command);
        if(error_code != 0){
            free(start);
            free(frame1);
            free(frame2);
            fclose(fp);
            closedir(dr);
            fclose(backing_file);
            return error_code;
        }
    }
    closedir(dr);

    //the filename doesnt already exist in the backing store 
    //add file to backing store
    if (!found) {
        int filelen = strlen(filename);
        int dirlen = strlen("backing_store");
        int namelen = filelen + dirlen+2;

        command = (char*) calloc(1, 9+namelen); 
        strncat(command, "cp ", 9);
        strncat(command, filename, filelen);
        strncat(command, " ", 2);
        strncat(command, "backing_store", dirlen);
        //copy the file to the backing store
        int errorCode = system(command);
        free(command);
        command = (char*) calloc(1, 2+strlen(filename)+strlen("backing_store/"));
        strncat(command, "backing_store/", strlen("backing_store/")+1);
        strncat(command, filename, strlen(filename)+1);
        //open the file in the backing store
        backing_file = fopen(command, "rt");
        free(command);
        if(error_code != 0){
            free(start);
            free(frame1);
            free(frame2);
            fclose(fp);
            fclose(backing_file);
            return error_code;
        }
    }
    fclose(fp);
    //add two frames to the frame store
    error_code = store_two_pages(backing_file, start, frame1, frame2);
    fclose(backing_file);
    //store the frame numbers in the pagetable
    pagetable[0]=*frame1;
    pagetable[1]=*frame2;
    for (int a=2; a<10; a++) pagetable[a]=-1;
    if(error_code != 0){
        fclose(backing_file);
        return error_code;
    }
    //create the pcb for the file
    QueueNode *node = malloc(sizeof(QueueNode));
    node->pcb = makePCB(*start,0,pagetable,filename);
    ready_queue_add_to_tail(node);
    free(start);
    free(frame1);
    free(frame2);
	return error_code;
}

//returns false if it worked and true otherwise
bool page_fault(QueueNode *node){
    //load next page of file from backing store into the frame
    int frame1;
    //store the pcb of the current node
    PCB *pcb = node->pcb;
    int *start = (int*)malloc(sizeof(int));
    //open the file from the backing store
    int total = strlen("backing_store/") + strlen(pcb->filename);
    char *command = (char*) calloc(1, 9+total);
    strncat(command, "backing_store/", strlen("backing_store/")+1);
    strncat(command, pcb->filename, strlen(pcb->filename)+1);
    //command is: backing_store/filename
    FILE *fp = fopen(command, "r");
    free(command);
    //check if the file exists
    if (fp==NULL) return true;
    char l[1000], line1[1000], line2[1000], line3[1000];
    int i;
    //look for the end of the pagetable
    for (i=1; i<10; i++){
        if (pcb->pagetable[i]==-1) {
            break;
        }
    }
    //i would be the frame number inside the file
    //this is the line we are on: i*3-1
    int counter=0;
    char *none = "none";
    while(1){
        //take out the line
        fgets(l, 999, fp);
        //check if reached the end of the file
        if(feof(fp)) { 
            fclose(fp);
            return true; }
        //check when counter reaches the line we are at
        if (counter==i*3-1){
            //check if we are at the end of the file
            if(feof(fp)) {
                fclose(fp);
                return true; }
            //find however many lines are left of the file
            fgets(line1, 999, fp);
            if(feof(fp)) {
                //there was only one line
                //add frames to frame store
                frame1 = add_frames(line1, none, none, start); 
                //check if the pagetable has the frame already and change it
                for (int d=0; d<10; d++) {
                    if (pcb->pagetable[d]==frame1){
                        pcb->pagetable[d]=12;
                    }
                }
                //update the pagetable
                pcb->pagetable[i]=frame1;
                //update the program counter
                pcb->PC = *start;
                break;}
            fgets(line2, 999, fp);
            if(feof(fp)) {
                frame1 = add_frames(line1, line2, none, start);
                for (int d=0; d<10; d++) {
                    if (pcb->pagetable[d]==frame1){
                        pcb->pagetable[d]=12;
                    }
                }
                pcb->pagetable[i]=frame1;
                pcb->PC = *start;
                break;}
            fgets(line3, 999, fp);
            frame1 = add_frames(line1, line2, line3, start);
            for (int d=0; d<10; d++) {
                if (pcb->pagetable[d]==frame1){
                    pcb->pagetable[d]=12;
                }
            }
            pcb->pagetable[i]=frame1;
            pcb->PC = *start;
            break;
        }
        counter++;
	}
    memset(line1, 0, sizeof(line1));
    memset(line2, 0, sizeof(line2));
    memset(line3, 0, sizeof(line3));
    memset(l, 0, sizeof(l));
    fclose(fp);
    return false;
};

bool execute(QueueNode* node, int quanta){
    char *line = NULL;
    PCB *pcb = node->pcb;
    //runs for however big quanta is
    for(int i=0; i<quanta; i++){
        //check if its at the end of a frame already
        if ((pcb->PC)%3==0){
            for (int a=0; a<10; a++) {
                if (pcb->pagetable[a]==pcb->PC/3){
                    //it was at the start of a frame
                    //run two lines and leave
                    //take value from line of memory
                    line = find_frame(pcb->PC);
                    //run line
                    if (strcmp(line, "none")!=0 && strcmp(line, "")!=0){
                        parseInput(line);
                        update_used(pcb->PC);
                    }
                    pcb->PC++;
                    //take value from line of memory
                    line = find_frame(pcb->PC);
                    //run line
                    if (strcmp(line, "none")!=0 && strcmp(line, "")!=0){
                        parseInput(line);
                        update_used(pcb->PC);
                    }
                    pcb->PC++;
                    return false;
                }
            }
            //not at the start, needs to find this frame
            int index = (pcb->PC-3)/3;
            int found = 0;
            for (int a=0; a<10; a++) {
                //find the frame number in the pagetable
                //check if the next frame is recorded
                if (pcb->pagetable[a]==index) {
                    if (pcb->pagetable[a+1]!=-1) {
                        found = 1;
                        //update the frame number in index to the next frame
                        index = pcb->pagetable[a+1];
                        break;
                    }
                }
            }
            if(found){
                //update PC to next frame
                pcb->PC = find_frame_no(index);
                //run two lines
                //take value from line of memory
                line = find_frame(pcb->PC);
                //run line
                if (strcmp(line, "none")!=0 && strcmp(line, "")!=0){
                    parseInput(line);
                    update_used(pcb->PC);
                }
                pcb->PC++;
                //take value from line of memory
                line = find_frame(pcb->PC);
                //run line
                if (strcmp(line, "none")!=0 && strcmp(line, "")!=0){
                    parseInput(line);
                    update_used(pcb->PC);
                }
                pcb->PC++;
                return false;
            }
            if(!found){
                //not stored in frame store
                //take it from the backing store file
                return page_fault(node);
            }
        }
        //check if the next line is at the end of a frame
        if((pcb->PC+1)%3==0 && pcb->PC!=0){
            //run the first line
            //take value from line of memory
            line = find_frame(pcb->PC);
            //run line
            if (strcmp(line, "none")!=0 && strcmp(line, "")!=0){
                parseInput(line);
                update_used(pcb->PC);
            }
            pcb->PC++;
            //find current frame number in frame store
            int index = (pcb->PC-3)/3;
            int found = 0;
            for (int a=0; a<10; a++) {
                //find the frame number in the pagetable
                //check if the next frame is recorded
                if (pcb->pagetable[a]==index) {
                    if (pcb->pagetable[a+1]!=-1) {
                        found = 1;
                        //update the frame number in index to the next frame
                        index = pcb->pagetable[a+1];
                        break;
                    }
                }
            }
            if(found){
                //update PC to next frame
                pcb->PC = find_frame_no(index);
                //take value from line of memory
                line = find_frame(pcb->PC);
                //run the next line
                if (strcmp(line, "none")!=0 && strcmp(line, "")!=0){
                    parseInput(line);
                    update_used(pcb->PC);
                }
                pcb->PC++;
                return false;
            }
            if(!found){
                //not stored in frame store
                //take it from the backing store file
                return page_fault(node);
            }
        }
        //otherwise run two lines (if not at the end of a frame)
        //take value from line of memory
        line = find_frame(pcb->PC);
        //run line
        if (strcmp(line, "none")!=0 && strcmp(line, "")!=0){
            parseInput(line);
            update_used(pcb->PC);
        }
        pcb->PC++;
        //take value from line of memory
        line = find_frame(pcb->PC);
        //run line
        if (strcmp(line, "none")!=0 && strcmp(line, "")!=0){
            parseInput(line);
            update_used(pcb->PC);
        }
        pcb->PC++;
        return false;
    }
    return false;
}

void *scheduler(void *arg){
    //quanta will be 2
    int quanta = ((int *) arg)[0];
    QueueNode *cur;
    while(true){
        //check if the head of the queue is empty
        if(is_ready_empty()){
            break;
        }
        //take out the head of the queue if not empty
        cur = ready_queue_pop_head();
        //execute the two lines
        if(!execute(cur, quanta)) {
            //add the process to the end 
            ready_queue_add_to_tail(cur);
        }
    }
    return 0;
}

//only does RR
int schedule_policy(char* policy, bool mt){
    if(strcmp(policy, "FCFS")!=0 && strcmp(policy, "SJF")!=0 && 
        strcmp(policy, "RR")!=0 && strcmp(policy, "AGING")!=0 && strcmp(policy, "RR30")!=0){
            return 15; // no correct policy was used
    }
    int arg[1];
    if(strcmp("RR",policy)==0){
        arg[0] = 2;
        scheduler((void *) arg);
        return 0;
    }
}
