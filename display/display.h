// Function declarations
#include <stdlib.h>
void init_display(int fd);
void write_command(int fd, unsigned char cmd);
void write_data(int fd, unsigned char *data, int size);
void clear_display(int fd);
void print_text(int fd, int page, int col, const char *text);