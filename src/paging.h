#include "ready_queue.h"
int store_two_pages(FILE *filename, int *start, int *end, int *pagetable[]);
int bckstr_file(char *filename);
bool execute(QueueNode *node, int quanta);
void *scheduler(void *arg);
int schedule_policy(char* policy, bool mt);