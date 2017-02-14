#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

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
int current_line;

void error()
{
    printf("?\n");
}

void print_range(int start, int end, bool show_num)
{
    if (start > end) {
        error();
        return;
    }

    size_t len;
    int line_num = 1;
    node* cur = buffer.first;

    while (line_num <= end) {
        if (line_num >= start) {
            if (show_num)
                printf("%d\t%s\n", line_num, cur->line);
            else
                printf("%s\n", cur->line);
        }
        
        cur = cur->next;
        line_num++;
    }
}

void print_line(int num, bool show_num)
{
    print_range(num, num, show_num);
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
    current_line = buffer.length;
}

int main()
{
    char* filename = "/Users/mtimkovich/tmp/rps.py";
    char* line;

    read_file(filename);

    while ((line = linenoise("")) != NULL) {
        int start;
        char command = 0;

        if (isdigit(line[0])) {
            sscanf(line, "%d", &start);
            command = 'p';
        } else if (line[0] == '$') {
            current_line = buffer.length;
            command = 'p';
        } else if (line[0] == '.') {
            command = 'p';
        }

        if (command == 0) {
            command = line[0];
        }


        if (start > buffer.length || start <= 0) {
            error();
            continue;
        }

        current_line = start;

        switch (command) {
            case 'q':
                return 0;
            case 'n':
                print_line(current_line, true);
                break;
            case 'p':
                print_line(current_line, false);
                break;
            default:
                error();
        }

        free(line);
    }

    fclose(fp);

    return 0;
}
