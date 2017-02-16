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
    bool modified;
};

typedef struct list_t list;

enum error_t { ADDR, CMD, IFILE };

list buffer;
int current_line;
char* error_msg;

void error(enum error_t type)
{
    if (type == ADDR)
        error_msg = "invalid address";
    else if (type == CMD)
        error_msg = "unknown command";
    else if (type == IFILE)
        error_msg = "cannot open input file";

    printf("?\n");
}

void print_range(int start, int end, bool show_num)
{
    if (buffer.first == NULL || start > end || end > buffer.length) {
        error(ADDR);
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

    current_line = end;
}

void delete_node(node* nd)
{
    node* prev = nd->prev;
    node* next = nd->next;

    if (prev != NULL)
        prev->next = next;

    if (next != NULL)
        next->prev = prev;

    if (nd == buffer.first) {
        buffer.first = next;
    } else if (nd == buffer.last) {
        buffer.last = prev;
    }

    buffer.length--;

    if (current_line > buffer.length)
        current_line = buffer.length;

    free(nd->line);
    free(nd);
}

void delete_range(int start, int end)
{
    node* cur = buffer.first;
    int line_num = 1;

    if (buffer.first == NULL || start == 0 || end > buffer.length) {
        error(ADDR);
        return;
    }

    while (line_num <= end) {
        node* next = cur->next;
        if (line_num >= start)
            delete_node(cur);

        cur = next;
        line_num++;
    }

    buffer.modified = true;
}

/* read file into a doubly linked list of lines */
void read_file(char* filename)
{
    node* root = NULL;
    node* prev;
    int total = 0;
    buffer.length = 0;
    buffer.modified = false;

    FILE* fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("%s: No such file or directory\n", filename);
        error(IFILE);
        return;
    }

    rewind(fp);

    for (;;) {
        char* line = NULL;
        size_t len = 0;
        int r = getline(&line, &len, fp);

        if (r == -1)
            break;

        total += r;

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

    printf("%d\n", total);
}

char parse(const char* line, int* start, int* end)
{
    char command = 0;
    int matches;

    if (isdigit(line[0])) {
        matches = sscanf(line, "%d %c", start, &command);

        if (matches == 1 && *start != -1) {
            command = 'p';
        } else if (matches == 2) {
            if (command == ',') {
                matches = sscanf(line, "%d , %d %c", start, end, &command);

                if (matches != 3) {
                    return 0;
                }
            }
        }

    // TODO: Allow '$' and '.' in commands and ranges
    } else if (line[0] == '$') {
        *start = buffer.length;
        command = 'p';
    } else if (line[0] == '.') {
        command = 'p';
    } else {
        command = line[0];
    }

    return command;
}

int main(int argc, char* argv[])
{
    char* filename = NULL;
    char* line;
    error_msg = "";
    buffer.first = NULL;

    if (argc > 1) {
        filename = argv[1];
        read_file(filename);
    }

    while ((line = linenoise("")) != NULL) {
        int start = -1;
        int end = -1;

        char command = parse(line, &start, &end);

        if (command == 0) {
            if (start > buffer.length ||
                    start == 0 ||
                    (end != -1 && start > end)) {
                error(ADDR);
            } else {
                error(CMD);
            }

            free(line);
            continue;
        }

        if (start == -1 && end == -1)
            start = current_line;

        if (start != -1 && end == -1)
            end = start;

        switch (command) {
            case 'q':
                return 0;
            case 'e':
                read_file(line+2);
                break;
            case 'n':
                print_range(start, end, true);
                break;
            case 'p':
                print_range(start, end, false);
                break;
            case 'd':
                delete_range(start, end);
                break;
            case 'h':
                if (strlen(error_msg) > 0)
                    printf("%s\n", error_msg);
                break;
            default:
                error(CMD);
        }

        free(line);
    }

    return 0;
}
