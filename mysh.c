/* Copyright (C) 2022 Hong Xu */
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/wait.h>

#define MAX_LEN 1000
#define LIMIT_LEN 512

typedef struct {
    char* name;
    char** strs;
    int num_strs;
    void* next;
    void* prev;
} Alias;

typedef struct {
    int length;
    void* head;
    void* tail;
} AliasList;

/**
 * @brief print the all the alias
 * 
 * @param list the list of alias
 */
void print_list(AliasList* list) {
    Alias* cur = list->head;
    while (cur != NULL) {
       fprintf(stdout, "%s", cur->name);
       fflush(stdout);
       int num = cur->num_strs;
       for (int i = 0; i < num; i++) {
           fprintf(stdout, " %s", cur->strs[i]);
           fflush(stdout);
       }
       fprintf(stdout, "\n");
       fflush(stdout);
       cur = cur->next;
    }
}

/**
 * @brief free the memory allocated for list
 * 
 * @param list the list of alias
 */
void free_list(AliasList* list) {
    Alias* cur = list->tail;
    while (cur != NULL) {
        int num = cur->num_strs;
        for (int i = 0; i < num; i++) {
            free(cur->strs[i]);
            cur->strs[i] = NULL;
        }
        free(cur->strs);
        cur->strs = NULL;
        free(cur->name);
        cur->name = NULL;
        if (cur->prev == NULL) {
            cur->next = NULL;
            cur->prev = NULL;
            free(cur);
            cur = NULL;
            list->head = NULL;
            break;
        }
        Alias* temp = cur;
        cur = cur->prev;
        temp->prev = NULL;
        free(cur->next);
        cur->next = NULL;
    }
}

/**
 * @brief Get the new arguments associated with alias name
 * 
 * @param num_strs number of new arguments (including command)
 * @param mylist the list of alias
 * @param index the index of the alias, whose arguments need
 *              to be returned
 * @return char**  the arguments of a alias
 */
char** get_strs(int*num_strs, AliasList* mylist, int index) {
    Alias* cur = mylist->head;
    for (int i = 0; i < index; i++) {
        cur = cur->next;
    }
    int count = cur->num_strs;
    *num_strs = count;
    char** new_strs = malloc(LIMIT_LEN * sizeof(char*));
    for (int i = 0; i < count; i++) {
        int str_len = strlen(cur->strs[i]);
        new_strs[i] = malloc((str_len + 1) * sizeof(char));
        // strcpy(new_strs[i], cur->strs[i]);
        snprintf(new_strs[i], (str_len + 1), "%s", cur->strs[i]);
    }
    new_strs[count] = NULL;  // null pointer for terminate
    return new_strs;
}

/**
 * @brief print all the arguments associated with a alias name
 * 
 * @param mylist the list of alias
 * @param index the index of alias name
 */
void print_strs(AliasList* mylist, int index) {
    Alias* cur = mylist->head;
    for (int i = 0; i < index; i++) {
        cur = cur->next;
    }
    int count = cur->num_strs;
    fprintf(stdout, "%s", cur->name);
    fflush(stdout);
    for (int i = 0; i < count; i++) {
        fprintf(stdout, " %s", cur->strs[i]);
        fflush(stdout);
    }
    fprintf(stdout, "\n");
    fflush(stdout);
}

/**
 * @brief get the index of the alias name
 * 
 * @param mylist the list of alias
 * @param name the alias name
 * @return int the index
 */
int get_index(AliasList* mylist, char* name) {
    int index = 0;
    //  char** new_strs = NULL;
    Alias* cur = mylist->head;
    while (cur != NULL) {
       int compare = strcmp(name, cur->name);
       if (compare == 0) {
           return index;
       }
       cur = cur->next;
       index++;
    }
    return -1;
}

/**
 * @brief delete an alias
 * 
 * @param mylist the list of alias
 * @param index the index of an alias to be deleted
 */
void delete_node(AliasList* mylist, int index) {
    Alias* cur = mylist->head;
    for (int i = 0; i < index; i++) {
        cur = cur->next;
    }

    // free node fields
    int num = cur->num_strs;
    for (int i = 0; i < num; i++) {
        free(cur->strs[i]);
        cur->strs[i] = NULL;
    }
    free(cur->strs);
    cur->strs = NULL;
    free(cur->name);
    cur->name = NULL;

    // set list
    if (mylist->length == 1) {  // cur is the only onde
        free(cur);
        cur = NULL;
        mylist->head = NULL;
        mylist->tail = NULL;
    } else if (index == 0) {  // cur is head
        Alias* temp = cur->next;
        temp->prev = NULL;
        mylist->head = temp;
        cur->next = NULL;
        free(cur);
        cur = NULL;
    } else if (index == mylist->length - 1) {
        // cur is tail
        Alias* temp = cur->prev;
        temp->next = NULL;
        mylist->tail = temp;
        cur->prev = NULL;
        free(cur);
        cur = NULL;
    } else {
        // cur is at the middle
        Alias* left = cur->prev;
        Alias* right = cur->next;
        left->next = right;
        right->prev = left;
        cur->next = NULL;
        cur->prev = NULL;
        free(cur);
        cur = NULL;
    }
    mylist->length--;
    return;
}

/**
 * @brief replace values of a alias name
 * 
 * @param mylist the list of alias
 * @param index the index of alias name
 * @param args input arguments to replace
 */
void replace_strs(AliasList* mylist, int index, char** args) {
    Alias* cur = mylist->head;
    for (int i = 0; i < index; i++) {
        cur = cur->next;
    }

    // free previous strings
    int num = cur->num_strs;
    for (int i = 0; i < num; i++) {
        free(cur->strs[i]);
        cur->strs[i] = NULL;
    }
    free(cur->strs);
    cur->strs = NULL;

    // allocate current strings
    int len = 0;
    while (args[len] != NULL) {
        len++;
    }
    int num_str = len - 2;
    cur->num_strs = num_str;
    cur->strs = malloc(LIMIT_LEN * sizeof(char*));
    for (int i = 0; i < num_str; i++) {
        int str_len = strlen(args[i+2]);
        char* new_str = malloc((str_len + 1) * sizeof(char));
        // strcpy(new_str, args[i+2]);
        snprintf(new_str, (str_len + 1), "%s", args[i+2]);
        cur->strs[i] = new_str;
        new_str = NULL;
    }
}

/**
 * @brief add new alias to the list
 * 
 * @param myList the list of alias
 * @param args input arguement of the alias
 */
void add_node(AliasList* myList, char** args) {
    int len = 0;
    while (args[len] != NULL) {
        len++;
    }
    int num_str = len - 2;  // excluding command and <alias_name>
    // building a new node
    Alias* new_node = malloc(sizeof(Alias));
    new_node->num_strs = num_str;
    new_node->name = malloc((strlen(args[1]) + 1) * sizeof(char));
    // strcpy(new_node->name, args[1]);
    snprintf(new_node->name, (strlen(args[1]) + 1), "%s", args[1]);
    new_node->strs = malloc(LIMIT_LEN * sizeof(char*));
    for (int i = 0; i < num_str; i++) {
        int str_len = strlen(args[i+2]);
        char* new_str = malloc((str_len + 1) * sizeof(char));
        // strcpy(new_str, args[i+2]);
        snprintf(new_str, (str_len + 1), "%s", args[i+2]);
        new_node->strs[i] = new_str;
        new_str = NULL;
    }

    // set up list
    new_node->prev = myList->tail;
    new_node->next = NULL;
    if (myList->length == 0) {
        myList->tail = new_node;
        myList->head = new_node;
        myList->length = 1;
        new_node = NULL;
        return;
    }
    ((Alias*)(myList->tail))->next = new_node;
    myList->tail = new_node;
    new_node = NULL;
    myList->length++;
    return;
}

/**
 * @brief check the format of a command line
 * 
 * @param src command line W/O '\n'
 * @return int 2 if empty or only whitespaces
 *             1 if valid redirection
 *             0 if valid non-redirection
 *            -1 if misformatted redirection
 */
int format_command(char *src) {
    int num = 0;
    int len = strlen(src);
    if (len == 0) {  // empty command
        return 2;
    }
    int index = -1;
    int first_nonspace = -1;
    int last_nonspace = -1;
    int first_nonspace_after_sign = -1;
    int count_hole = 0;
    for (int i = 0; i < len; i++) {
        if (src[i] == '>') {
            index = i;
            num++;
        }
    }
    for (int i = 0; i < len; i++) {
        if (!isspace(src[i])) {
            first_nonspace = i;
            break;
        }
    }
    for (int i = len-1; i >= 0; i--) {
        if (!isspace(src[i])) {
            last_nonspace = i;
            break;
        }
    }
    if (first_nonspace == -1) return 2;  // command only contains whitespaces
    if (num == 0) {  // nonempty (excluding whitespaces) without '>'
        return 0;
    } else if (num > 1) {
        return -1;  // multiple '>'
    }

    // starting or ending with '>'
    if (first_nonspace == index || last_nonspace == index) {
        return -1;
    }

    // multiple files
    for (int i = index + 1; i <= last_nonspace; i++) {
        if (!isspace(src[i])) {
            first_nonspace_after_sign = i;
            break;
        }
    }
    for (int i = first_nonspace_after_sign; i <= last_nonspace; i++) {
        if (isspace(src[i])) {
            count_hole++;
            break;
        }
    }
    if (count_hole > 0) {
        return -1;
    }

    // valid format
    return 1;
}

/**
 * @brief parse the command line
 * 
 * @param args parsed command line arguments
 * @param out_file output file for redirection
 * @param command original command line
 * @param mode 1 if redirection
 */
void parse_command(char** args, char* out_file, char* command, int* mode) {
    const char *delimiters = " \t\n>";  // ignore space, tab, newline, '>'
    int i = 0;

    // split tokens
    char* token;
    char* rest = command;
    while ((token = strtok_r(rest, delimiters, &rest))) {
        args[i] = token;
        i++;
    }
    args[i] = NULL;

    // if mode is redirection, get the output file name
    if (1 == *mode) {
        memcpy(out_file, args[i-1], strlen(args[i-1]));
        args[i-1] = NULL;
    }
}

/**
 * @brief driver of the program
 * 
 * @param argc 
 * @param argv 
 * @return int 
 */
int main(int argc, char *argv[]) {
    int is_batch;  // 1 if batch mode, 0 if not
    int saved_stdout = dup(1);  // save the stdout
    FILE* batch_file;

    // Check command line arguments for mysh
    if (argc > 2) {  // incorrect number of arguments for shell, exit
        fprintf(stderr, "Usage: mysh [batch-file]\n");
        fflush(stderr);
        exit(1);
    } else if (argc == 1) {  // in interactive mode
        is_batch = 0;
    } else {  // in batch mode
        is_batch = 1;
        // open batch file
        batch_file = fopen(argv[1], "r");
        if (batch_file == NULL) {  // Error: file not exist or cannot be opened
            fprintf(stderr, "Error: Cannot open file %s.\n", argv[1]);
            fflush(stderr);  // not sure whether it is necessary since
                             // stderr is not buffered by default
            exit(1);
        }

        // redirect the stdin to the batch file, won't be changed until
        // the shell terminates
        dup2(fileno(batch_file), 0);
        close(fileno(batch_file));   // release it to be reuseable by others
        fclose(batch_file);
    }

    // initialize alias list before go into the loop
    AliasList* mylist = malloc(sizeof(AliasList));
    mylist->head = NULL;
    mylist->tail = NULL;
    mylist->length = 0;

    while (1) {
        int is_redirection;  // indicator for redirection
        if (!is_batch) {  // interactive mode
            fprintf(stdout, "mysh> ");
            fflush(stdout);
        }

        // check input from user or batch file
        char command_line[MAX_LEN];
        if (fgets(command_line, MAX_LEN, stdin) == NULL) {
            free_list(mylist);
            free(mylist);
            mylist = NULL;
            if (feof(stdin)) {  // reach EOF
                exit(0);
            } else {  // error for input stream
                perror("Error: something wrong with the input\n");
                exit(1);
            }
        }

        int is_too_long;
        if (strlen(command_line) > LIMIT_LEN) {
            if (command_line[LIMIT_LEN] != '\n') {  // too long
                is_too_long = 1;
                if (strchr(command_line, '\n') == NULL) {
                    // may exceed the MAX_LEN or it's the last line
                    // flush the stdin until reach '\n' or EOF
                    do {
                        char c = fgetc(stdin);
                        if (feof(stdin) || c == '\n') {break;}
                    } while (1);
                }
            } else {  // length is LIMIT_LEN + 1, ended with '\n'
                is_too_long = 0;
            }
        } else {
            is_too_long = 0;
        }
        // dealing with long command, echo command if necessary (batch mode)
        command_line[strcspn(command_line, "\n")] = 0;
        if (is_too_long) {
            fprintf(stderr, "Error: over %i characters\n", LIMIT_LEN);
            fflush(stderr);
            if (is_batch) {
                // echo first 512 characters
                char temp[MAX_LEN];
                memcpy(temp, &command_line[0], LIMIT_LEN);
                temp[LIMIT_LEN] = '\0';
                fprintf(stdout, "%s\n", temp);
                fflush(stdout);
            }
            continue;
        } else {
            if (is_batch) {
                fprintf(stdout, "%s\n", command_line);
                fflush(stdout);
            }
        }

        // set indicator of redirection
        is_redirection = format_command(command_line);
        if (is_redirection == -1) {
            // misformatted redirection
            fprintf(stderr, "Redirection misformatted.\n");
            fflush(stderr);
            continue;
        } else if (is_redirection == 2) {
            // empty (excluding whitespaces) command
            continue;
        }

        // parse command line arguments
        char** args = malloc(MAX_LEN * sizeof(char *));
        char out_file[MAX_LEN] = "";
        parse_command(args, out_file, command_line, &is_redirection);
        if (is_redirection == 1) {
            FILE* redirect_file = fopen(out_file, "w");
            if (redirect_file == NULL) {  // Error: file cannot be opened
                fprintf(stderr, "Cannot write to file %s.\n", out_file);
                fflush(stderr);
                continue;
            }
            dup2(fileno(redirect_file), 1);  // set stdout to the opened file
            close(fileno(redirect_file));
            fclose(redirect_file);
        }

        // check built-in commands
        int is_alias = 0;
        int is_unalias = 0;
        if (strcmp(args[0], "exit") == 0) {
            free(args);
            args = NULL;
            free_list(mylist);
            free(mylist);
            mylist = NULL;
            exit(0);
        } else if (strcmp(args[0], "alias") == 0) {
            is_alias = 1;
        } else if (strcmp(args[0], "unalias") == 0) {
            is_unalias = 1;
        }

        // alias and unalias before fork()
        int len = 0;
        while (args[len] != NULL) {
            len++;
        }
        if ((is_alias || is_unalias) && len >= 2) {
            if (strcmp(args[1], "alias") == 0 ||
                strcmp(args[1], "unalias") == 0 ||
                strcmp(args[1], "exit") == 0) {
                    fprintf(stderr, "alias: Too dangerous to alias that.\n");
                    fflush(stderr);
                    free(args);
                    args = NULL;
                    continue;
                }
        }

        if (is_alias) {
            if (len == 1) {  // print alias
                print_list(mylist);
            } else if (len == 2) {  // print replacement strs
                int index = get_index(mylist, args[1]);
                if (index != -1) {
                    print_strs(mylist, index);
                }
            } else {  // replace or add strs
                int index = get_index(mylist, args[1]);
                if (index != -1) {
                    replace_strs(mylist, index, args);
                } else {
                    add_node(mylist, args);
                }
            }
            free(args);
            args = NULL;
            continue;
        } else if (is_unalias) {
            if (len != 2) {
                fprintf(stderr, "unalias: Incorrect number of arguments.\n");
                fflush(stderr);
            } else {
                int index = get_index(mylist, args[1]);
                if (index != -1) {
                    delete_node(mylist, index);
                }
            }
            free(args);
            args = NULL;
            continue;
        }

        int index = get_index(mylist, args[0]);
        int num_strs = 0;
        char** new_args;
        if (index != -1) {  // command is alias name
            new_args = get_strs(&num_strs, mylist, index);
        }
        // fork a child and execute it
        pid_t retval = fork();
        if (retval == 0) {
            if (index != -1) {
                execv(new_args[0], new_args);
                fprintf(stderr, "%s: Command not found.\n", new_args[0]);
                fflush(stderr);
            } else {
                execv(args[0], args);
                fprintf(stderr, "%s: Command not found.\n", args[0]);
                fflush(stderr);
            }
            _exit(1);
        } else {
            wait(NULL);
            // recover stdout to console
            dup2(saved_stdout, 1);

            if (index != -1) {
                for (int i = 0; i < num_strs; i++) {
                    free(new_args[i]);
                }
                free(new_args);
                new_args = NULL;
            }
            free(args);
            args = NULL;
        }
    }
    free_list(mylist);
    free(mylist);
    mylist = NULL;
}
