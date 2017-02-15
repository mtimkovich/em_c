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

void delete_line(int num)
{
    node* cur = buffer.first;
    int line_num = 1;

    while (line_num < num) {
        cur = cur->next;
        line_num++;
    }

    node* prev = cur->prev;
    node* next = cur->next;

    if (prev != NULL)
        prev->next = next;

    if (next != NULL)
        next->prev = prev;

    if (cur == buffer.first) {
        buffer.first = next;
    } else if (cur == buffer.last) {
        buffer.last = prev;
    }

    buffer.length--;

    if (current_line > buffer.length)
        current_line = buffer.length;

    free(cur->line);
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
            root->prev = NULL;
        } else {
            prev->next = cur;
            cur->prev = prev;
        }

        prev = cur;
        buffer.last = cur;
        buffer.length++;
    }

    buffer.first = root;
    buffer.last->next = NULL;
    current_line = buffer.length;
}

int main()
{
    char* filename = "/Users/mtimkovich/tmp/rps.py";
    char* line;

    read_file(filename);

    while ((line = linenoise("")) != NULL) {
        int start = -1;
        char command = 0;

        if (isdigit(line[0])) {
            sscanf(line, "%d", &start);
            command = 'p';

            if (start > buffer.length || (start <= 0 && start != -1)) {
                error();
                continue;
            } else {
                current_line = start;
            }
        } else if (line[0] == '$') {
            current_line = buffer.length;
            command = 'p';
        } else if (line[0] == '.') {
            command = 'p';
        }

        if (command == 0) {
            command = line[0];
        }

        switch (command) {
            case 'q':
                return 0;
            case 'n':
                print_line(current_line, true);
                break;
            case 'p':
                print_line(current_line, false);
                break;
            case 'd':
                delete_line(current_line);
                break;
            default:
                error();
        }

        free(line);
    }

    fclose(fp);

    return 0;
}
