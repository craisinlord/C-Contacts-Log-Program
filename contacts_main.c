#include <stdio.h>
#include <string.h>

#include "contacts.h"

#define MAX_CMD_LEN 128

/*
 * This is in general *very* similar to the list_main file seen in lab 2
 * One big difference is the notion of switching between contact logs in one
 * run of the program.
 * You have to create or load a contact log from a file before you can do things
 * like add, lookup, or write.
 * The code below has to check if contact log is NULL before the operations
 * occur. Also, the user has to explicitly clear the current contact log before
 * they can create or load in a new one.
 */
int main(int argc, char **argv) {
    contacts_log_t *log = NULL;

    // Handle command line argument for file loading
    if (argc > 1) {
        const char *file_name = argv[1];
        // printf("File name is %s\n", file_name);
        size_t len = strlen(file_name);

        if (len > 4 && strcmp(file_name + len - 4, ".txt") == 0) {
            log = read_contacts_log_from_text(file_name);
            if (log != NULL) {
                printf("Contacts log loaded from text file\n");
            } else {
                printf("Failed to read contacts log from text file\n");
            }
        } else if (len > 4 && strcmp(file_name + len - 4, ".bin") == 0) {
            log = read_contacts_log_from_binary(file_name);
            if (log != NULL) {
                printf("Contacts log loaded from binary file\n");
            } else {
                printf("Failed to read contacts log from binary file\n");
            }
        } else {
            printf("Error: Unknown contacts log file extension\n");
        }
    }

    printf("CSCI 2021 Contact Log System\n");
    printf("Commands:\n");
    printf("  create <name>:            creates a new log with specified name\n");
    printf("  log:                      shows the name of the active contacts log\n");
    printf("  add <name> <phone> <zip>: adds a new contact\n");
    printf("  lookup <name>:            searches for a phone number by contact name\n");
    printf("  clear:                    resets current contacts log\n");
    printf("  print:                    shows all contacts in active log\n");
    printf("  write_text:               saves all contacts to text file\n");
    printf("  read_text <file_name>:    loads contacts from text file\n");
    printf("  write_bin:                saves all contacts to binary file\n");
    printf("  read_bin <file_name>:     loads contacts from binary file\n");
    printf("  exit:                     exits the program\n");

    char cmd[MAX_CMD_LEN];
    while (1) {
        printf("contacts> ");
        if (scanf("%s", cmd) == EOF) {
            printf("\n");
            break;
        }
        if (strcmp("exit", cmd) == 0) {
            break;
        }

        else if (strcmp("create", cmd) == 0) {
            if (scanf("%s", cmd) == 1) { 
                if (log != NULL) {
                    printf("Error: You already have an contacts log.\n");
                    printf("You can remove it with the \'clear\' command\n");
                } else {
                    log = create_contacts_log(cmd);
                    if (log == NULL) {
                        printf("Contacts log creation failed\n");
                    }
                }
            } else {
                printf("Error: Invalid input format for 'create' command\n");
            }
        }

        else if (strcmp("log", cmd) == 0) {
            if (log == NULL) {
                printf("Error: You must create or load a contacts log first\n");
            } else {
                printf("%s\n", log->log_name);
            }
        }

        else if (strcmp("add", cmd) == 0) {
            char name[MAX_NAME_LEN] = "EMPTY";
            unsigned long phone = 0;
            unsigned zip = 0;
            scanf("%s %lu %u", name, &phone, &zip);
            if (log == NULL) {
                printf("Error: You must create or load a contacts log first\n");
            } else { 
                if (add_contact(log, name, phone, zip) == 1) {
                    printf("Error: Invalid phone number and/or zip code\n");
                }
            }
        }

        else if (strcmp("lookup", cmd) == 0) {
            char contact_name[MAX_NAME_LEN];
            scanf("%s", contact_name);
            if (log == NULL) {
                printf("Error: You must create or load a contacts log first\n");
            } else {
                long phone_number = find_phone_number(log, contact_name);
                if (phone_number == -1) {
                    printf("No phone number for '%s' found\n", contact_name);
                } else {
                    printf("%s: %ld\n", contact_name, phone_number);
                }
            }
        }

        else if (strcmp("clear", cmd) == 0) {
            if (log == NULL) {
                printf("Error: No contacts log to clear\n");
            } else {
                free_contacts_log(log);
                log = NULL;
            }
        }

        else if (strcmp("print", cmd) == 0) {
            if (log == NULL) {
                printf("Error: You must create or load a contacts log first\n");
            } else {
                // Extract just the base name without the extension
                const char *base_name = log->log_name; // Assuming log_name already doesn't include path

                // Find the last dot in the log_name to truncate the extension
                char *dot = strrchr(base_name, '.');
                if (dot != NULL) {
                    // Truncate at the dot
                    size_t length = dot - base_name;
                    char name_without_extension[MAX_NAME_LEN];
                    strncpy(name_without_extension, base_name, length);
                    name_without_extension[length] = '\0'; // Null-terminate

                    printf("All contacts in %s:\n", name_without_extension);
                } else {
                    printf("All contacts in %s:\n", base_name);
                }

                print_contacts_log(log);
            }
        }

        else if (strcmp("write_text", cmd) == 0) {
            if (log == NULL) {
                printf("Error: You must create or load a contacts log first\n");
            } else {
                if (write_contacts_log_to_text(log) == 0) {
                    printf("Contacts log successfully written to %s.txt\n", log->log_name);
                } else {
                    printf("Failed to write contacts log to text file\n");
                }
            }
        }

        else if (strcmp("read_text", cmd) == 0) {
            scanf("%s", cmd);
            if (log != NULL) {
                printf("Error: You must clear current contacts log first\n");
            } else {
                log = read_contacts_log_from_text(cmd);
                if (log != NULL) {
                    printf("Contacts log loaded from text file\n");
                } else {
                    printf("Failed to read contacts log from text file\n");
                }
            }
        }

        else if (strcmp("write_bin", cmd) == 0) {
            if (log == NULL) {
                printf("Error: You must create or load a contacts log first\n");
            } else {
                if (write_contacts_log_to_binary(log) == 0) {
                    printf("Contacts log successfully written to %s.bin\n", log->log_name);
                } else {
                    printf("Failed to write contacts log to binary file\n");
                }
            }
        }

        else if (strcmp("read_bin", cmd) == 0) {
            scanf("%s", cmd);
            if (log != NULL) {
                printf("Error: You must clear current contacts log first\n");
            } else {
                log = read_contacts_log_from_binary(cmd);
                if (log != NULL) {
                    printf("Contacts log loaded from binary file\n");
                } else {
                    printf("Failed to read contacts log from binary file\n");
                }
            }
            
        }
        else {
            printf("Unknown command %s\n", cmd);
        }
    }

    if (log != NULL) {
        free_contacts_log(log);
    }
    return 0;
}
