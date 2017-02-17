#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include "linenoise.h"

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

enum error_t {
    ADDR,
    CMD,
    IFILE,
    NO_FILE,
    MOD
};

list buffer;
int current_line;
char* error_msg;
bool asked;

void error(enum error_t type)
{
    if (type == ADDR)
        error_msg = "invalid address";
    else if (type == CMD)
        error_msg = "unknown command";
    else if (type == IFILE)
        error_msg = "cannot open input file";
    else if (type == NO_FILE)
        error_msg = "no current filename";
    else if (type == MOD)
        error_msg = "warning: file modified";

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
    asked = false;
}

void write_buffer(char* filename)
{
    node* cur = buffer.first;
    int total = 0;

    if (filename == NULL) {
        error(NO_FILE);
        return;
    }

    FILE* fp = fopen(filename, "w");
    if (fp == NULL) {
        printf("%s: No such file or directory\n", filename);
        error(IFILE);
        return;
    }

    while (cur != NULL) {
        int r = fprintf(fp, "%s\n", cur->line);
        total += r;
        cur = cur->next;
    }

    fclose(fp);
    printf("%d\n", total);
    buffer.modified = false;
}

/* read file into a doubly linked list of lines */
void read_file(char* filename)
{
    node* root = NULL;
    node* prev;
    int total = 0;

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
    buffer.modified = false;

    printf("%d\n", total);

    fclose(fp);
}

bool is_str_digit(char* str)
{
    for (int i = 0; i < strlen(str); i++) {
        if (!isdigit(str[i]))
            return false;
    }

    return true;
}

int parse_macro(char c)
{
    switch (c) {
        case '.':
            return current_line;
        case '$':
            return buffer.length;
        case '+':
            return current_line + 1;
        case '-':
            return current_line - 1;
        case ',':
            return 1;
        default:
            return -1;
    }
}

void parse_line_num(char* line, int* start)
{
    if (is_str_digit(line))
        sscanf(line, "%d", start);
    else
        *start = parse_macro(line[0]);
}

// This is super messy. I may clean it up later
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
                char start_str[11];
                char end_str[11];
                matches = sscanf(line, "%10[0-9.$-+] , %10[0-9.$-+] %c[a-z]", start_str, end_str, &command);

                if (matches != 3) {
                    return 0;
                }

                parse_line_num(start_str, start);
                parse_line_num(end_str, end);
            }
        }

    } else if (parse_macro(line[0]) != -1) {
        if (strlen(line) == 1)
            command = 'p';
        else
            command = line[1];
        *start = parse_macro(line[0]);

        if (line[0] == ',')
            *end = buffer.length;
    } else {
        command = line[0];
    }

    return command;
}

int insert_into_buffer(list* lst, int num)
{
    if (lst == NULL)
        return 0;

    node* before = buffer.first;
    int line_num = 1;

    while (line_num++ < num)
        before = before->next;

    node* after = before->next;

    if (num != 0) {
        if (before != NULL)
            before->next = lst->first;
        lst->first->prev = before;

        if (after != NULL)
            after->prev = lst->last;
        else
            buffer.last = lst->last;
        lst->last->next = after;
    } else {
        buffer.first = lst->first;
        lst->last->next = before;
    }

    buffer.length += lst->length;
    buffer.modified = true;
    int wrote = lst->length;

    free(lst);

    return wrote;
}

void init_list(list* lst)
{
    lst->first = NULL;
    lst->last = NULL;
    lst->length = 0;
    lst->modified = false;
}

list* text_input()
{
    list* input_buffer = malloc(sizeof(list));
    init_list(input_buffer);

    node* root = NULL;
    node* prev;
    char* line;

    while ((line = linenoise("")) != NULL) {
        if (strcmp(".", line) == 0) {
            free(line);
            break;
        }

        node* cur = malloc(sizeof(node));
        cur->line = line;

        if (root == NULL) {
            root = cur;
            root->prev = NULL;
        } else {
            prev->next = cur;
            cur->prev = prev;
        }

        prev = cur;
        input_buffer->last = cur;
        input_buffer->length++;
    }

    if (root == NULL) {
        free(input_buffer);
        return NULL;
    }

    input_buffer->first = root;
    input_buffer->last->next = NULL;

    return input_buffer;
}

int main(int argc, char* argv[])
{
    char* filename = NULL;
    char* line;
    list* input;
    error_msg = "";
    asked = false;
    int w;

    init_list(&buffer);

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
                if (buffer.modified && !asked) {
                    error(MOD);
                    asked = true;
                } else {
                    return 0;
                }
                break;
            case 'Q':
                return 0;
            case 'e':
                if (strlen(line) > 2)
                    filename = line + 2;
                else
                    error(NO_FILE);

                read_file(filename);
                break;
            case 'w':
                if (strlen(line) > 2)
                    filename = line + 2;
                write_buffer(filename);
                break;
            case 'a':
                input = text_input();
                w = insert_into_buffer(input, end);
                current_line += w;
                break;
            case 'i':
                input = text_input();
                w = insert_into_buffer(input, start-1);
                current_line = start-1 + w;
                break;
            case 'c':
                delete_range(start, end);
                input = text_input();
                w = insert_into_buffer(input, start-1);
                current_line = start-1 + w;
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
