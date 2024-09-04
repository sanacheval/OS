#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "pcb.h"

int pid_counter = 1;

int generatePID(){
    return pid_counter++;
}

//In this implementation, Pid is the same as file ID 
PCB* makePCB(int start, int end, int pagetable[10], char *filename){
    PCB * newPCB = malloc(sizeof(PCB));
    newPCB->pid = generatePID();
    newPCB->PC = start;
    newPCB->start  = start;
    //end is now the end of the loaded frames
    newPCB->end = end;
    newPCB->job_length_score = 1+end-start;
    newPCB->priority = false;
    //adding the pagetable
    for (int i =0; i<10; i++) {
        newPCB->pagetable[i]= pagetable[i];
    }
    newPCB->filename = filename;
    return newPCB;
}