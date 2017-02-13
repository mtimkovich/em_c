#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "linenoise.h"

#define LBSIZE 4096

struct node_t {
    char* line;
    struct node_t* prev;
    struct node_t* next;
};

typedef struct node_t node;

struct list_t {
    node* first;
    node* last;
    int length;
};

typedef struct list_t list;

FILE* fp;
list buffer;

void print_range(int start, int end)
{
    if (start > end || end > buffer.length || start <= 0)
        return;

    size_t len;
    int line_num = 1;
    node* cur = buffer.first;

    while (line_num <= end) {
        if (line_num >= start) {
            printf("%d\t%s\n", line_num, cur->line);
        }
        
        cur = cur->next;
        line_num++;
    }
}

void print_line(int num)
{
    print_range(num, num);
}

/* read file into a doubly linked list of lines */
void read_file(char* filename)
{
    node* root = NULL;
    node* prev;
    size_t len;
    buffer.length = 0;

    fp = fopen(filename, "r");

    rewind(fp);

    for (;;) {
        char* line = malloc(sizeof(char) * LBSIZE);
        int r = getline(&line, &len, fp);

        if (r == -1)
            break;

        node* cur = malloc(sizeof(node));
        line[strlen(line)-1] = 0;
        cur->line = line;

        if (root == NULL) {
            root = cur;
        } else {
            prev->next = cur;
            cur->prev = prev;
        }

        prev = cur;
        buffer.last = cur;
        buffer.length++;
    }

    buffer.first = root;
}

int main()
{
    char* filename = "/Users/mtimkovich/tmp/rps.py";
    char* line;

    read_file(filename);

    while ((line = linenoise(":")) != NULL) {
        if (strcmp("q", line) == 0) {
            return 0;
        }

        printf("%s\n", line);
        free(line);
    }

    fclose(fp);

    return 0;
}
