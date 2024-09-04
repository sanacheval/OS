#ifndef SHELLMEMORY_H
#define SHELLMEMORY_H
void mem_init();
int update_used(int index);
int add_frames(char *value_1, char *value_2, char *value_3, int* pStart);
char* find_frame(int index);
int find_frame_no(int frame_no);
char *mem_get_value(char *var);
void mem_set_value(char *var, char *value);
int load_file(FILE* fp, int* pStart, int* pEnd, char* fileID);
char * mem_get_value_at_line(int index);
void mem_free_lines_between(int start, int end);
void printShellMemory();
#endif
