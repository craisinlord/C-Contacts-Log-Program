#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "contacts.h"

// This is the (somewhat famous) djb2 hash
unsigned hash(const char *str) {
    unsigned hash_val = 5381;
    int i = 0;
    while (str[i] != '\0') {
        hash_val = ((hash_val << 5) + hash_val) + str[i];
        i++;
    }
    return hash_val % NUM_BUCKETS;
}

contacts_log_t *create_contacts_log(const char *log_name) {
    contacts_log_t *new_log = malloc(sizeof(contacts_log_t));
    if (new_log == NULL) {
        return NULL;
    }

    strcpy(new_log->log_name, log_name);
    for (int i = 0; i < NUM_BUCKETS; i++) {
        new_log->buckets[i] = NULL;
    }
    new_log->size = 0;

    return new_log;
}

const char *get_contacts_log_name(const contacts_log_t *log) {
    const char *name = log->log_name;
    if(name == NULL) {
        return NULL;
    }
    else {
        return name;
    }
}

int add_contact(contacts_log_t *log, const char *name, unsigned long phone_number, unsigned zip_code) {
    if (phone_number < 1000000000 || phone_number > 9999999999) {
        return 1;
    }
    
    if (zip_code < 10000 || zip_code > 99999) {
        return 1;
    }
    //Determines buket index using hash function
    unsigned bucket_index = hash(name);

    //create new contact node for the matching bucket
    node_t *new_node = malloc(sizeof(node_t));
    if (!new_node) {
        printf("Error: Memory allocation for new contact failed.\n");
        return 2;
    }
    strncpy(new_node->name, name, MAX_NAME_LEN - 1);
    new_node->name[MAX_NAME_LEN - 1] = '\0'; // Ensure null termination
    new_node->phone_number = phone_number;
    new_node->zip_code = zip_code;
    new_node->next = NULL;

    // Insert the new node into the correct bucket's linked list
    node_t *current = log->buckets[bucket_index];
    if (current == NULL) {
        // If the bucket is empty, make the new node the head of the list
        log->buckets[bucket_index] = new_node;
    } else {
        // Go to the end of the list and add the new node
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = new_node;
    }
    log->size += 1;
    return 0;
}

long find_phone_number(const contacts_log_t *log, const char *name) {
    unsigned bucket_index = hash(name);
    node_t *current = log->buckets[bucket_index];
    while (current != NULL) {
        if (strcmp(current->name, name) == 0) {
            return current->phone_number;
        }
        current = current->next;
    }
    return -1;
}

void print_contacts_log(const contacts_log_t *log) {
    if (log == NULL) {
        printf("Contacts log is empty.\n");
        return;
    }
    for (unsigned i = 0; i < NUM_BUCKETS; i++) {
        node_t *current = log->buckets[i];
        while (current != NULL) {
            printf("Name: %s, Phone Number: %lu, Zip Code: %u\n", 
                   current->name, 
                   current->phone_number, 
                   current->zip_code);
            current = current->next;
        }
    }
}

void free_contacts_log(contacts_log_t *log) {
    if (log == NULL) {
        return;
    }
    for (unsigned i = 0; i < NUM_BUCKETS; i++) {
        node_t *current = log->buckets[i];  
        while (current != NULL) {
            node_t *temp = current;          
            current = current->next;          
            free(temp);                      
        }
        log->buckets[i] = NULL;
    }
    free(log);
}

int write_contacts_log_to_text(const contacts_log_t *log) {
    char file_name[MAX_NAME_LEN + 4]; //adding length of .txt extension
    strcpy(file_name, log->log_name);
    strcat(file_name, ".txt");
    FILE *f = fopen(file_name, "w");
    if (f == NULL) {
        return -1;
    }
    fprintf(f, "%u\n", log->size);
    for (int i = 0; i < NUM_BUCKETS; i++) {
        node_t *current = log->buckets[i];
        while (current != NULL) {
            fprintf(f, "%s %lu %u\n", current->name, current->phone_number, current->zip_code);
            current = current->next;
        }
    }
    fclose(f);
    return 0;
}

contacts_log_t *read_contacts_log_from_text(const char *file_name) {
    FILE *f = fopen(file_name, "r");
    if (f == NULL) {
        return NULL;
    }
    contacts_log_t *log = malloc(sizeof(contacts_log_t));
    if (log == NULL) {
        fclose(f);
        return NULL;
    }

    //Copy all but last 4 characters for file extension
    strncpy(log->log_name, file_name, strlen(file_name) - 4); 

    //Ensure null-termination
    log->log_name[strlen(file_name) - 4] = '\0'; 

    fscanf(f, "%u", &log->size);


    for (int i = 0; i < NUM_BUCKETS; i++) {
        log->buckets[i] = NULL;
    }
    char name[MAX_NAME_LEN];
    unsigned long phone_number;
    unsigned zip_code;

    int size = log->size;
    for (int i = 0; i < size; i++) {
        fscanf(f, "%s %lu %u", name, &phone_number, &zip_code);
        add_contact(log, name, phone_number, zip_code);;
    }
    fclose(f);
    return log;
}

int write_contacts_log_to_binary(const contacts_log_t *log) {
    char file_name[MAX_NAME_LEN + 4]; //adding length of .bin extension
    strcpy(file_name, log->log_name);
    strcat(file_name, ".bin");
    FILE *f = fopen(file_name, "wb");
    if (f == NULL) {
        return -1;
    }
    fwrite(&log->size, sizeof(unsigned), 1, f);
    
    for (int i = 0; i < NUM_BUCKETS; i++) {
        node_t *current = log->buckets[i];
        while (current != NULL) {
            unsigned len = (unsigned)strlen(current->name);
            fwrite(&len, sizeof(unsigned), 1, f);
            fwrite(current->name, sizeof(char), len, f);
            fwrite(&current->phone_number, sizeof(unsigned long), 1, f);
            fwrite(&current->zip_code, sizeof(unsigned), 1, f);
            current = current->next;
        }
    }
    fclose(f);
    return 0;
}

contacts_log_t *read_contacts_log_from_binary(const char *file_name) {
    FILE *f = fopen(file_name, "rb");
    if (f == NULL) {
        return NULL;
    }
    contacts_log_t *log = malloc(sizeof(contacts_log_t));
    if (log == NULL) {
        fclose(f);
        return NULL;
    }

    //Copy all but last 4 characters for file extension
    strncpy(log->log_name, file_name, strlen(file_name) - 4); 

    //Ensure null-termination
    log->log_name[strlen(file_name) - 4] = '\0'; 

    //Initialize buckets to NULL
    for (int i = 0; i < NUM_BUCKETS; i++) {
        log->buckets[i] = NULL;
    }

    fread(&log->size, sizeof(unsigned), 1, f);
    unsigned size = log->size;
    for (unsigned i = 0; i < size; i++) {
        node_t *new_node = malloc(sizeof(node_t));
        //If node is null frees log and closes file
        if (new_node == NULL) {
            free_contacts_log(log);
            fclose(f);
            return NULL;
        }
        //Initialize node to 0
        memset(new_node, 0, sizeof(node_t));

        //Read the length of the name
        unsigned name_length;
        fread(&name_length, sizeof(unsigned), 1, f);

        //Ensure name_length doesn't exceed the max name length
        if (name_length > MAX_NAME_LEN - 1) { // -1 to reserve space for null terminator
            printf("Invalid name length at entry %u: %u\n", i, name_length);
            free(new_node);
            free_contacts_log(log);
            fclose(f);
            return NULL;
        }

        fread(new_node->name, sizeof(char), name_length, f);
        new_node->name[MAX_NAME_LEN] = '\0'; // Null-terminate the name string

        //Read phone number and zip code
        fread(&new_node->phone_number, sizeof(unsigned long), 1, f);
        fread(&new_node->zip_code, sizeof(unsigned), 1, f);
        
        new_node->next = NULL;

        //Insert the new node into the correct bucket
        unsigned bucket_index = hash(new_node->name);
        if (log->buckets[bucket_index] == NULL) {
            //If the bucket is empty, insert the node directly
            log->buckets[bucket_index] = new_node;
        } else {
            //Otherwise, append it to the front of the linked list
            node_t *current = log->buckets[bucket_index];
            while (current->next != NULL) {
                current = current->next;
            }
            current->next = new_node;
        }
    }


    fclose(f);
    return log;
}