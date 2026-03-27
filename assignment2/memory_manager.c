/*
    Memory Manager Simulation
    OS Principles Assignment 2 - simulates dynamic memory allocation using first fit, next fit, best fit, and worst fit strategies.
    Memory is represented as a doubly linked list of segments (either allocated or free). Total memory is 256 MB.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MEM_SIZE 256
#define NAME_LEN 32

//a segment is either free or taken by a process
typedef enum { FREE, USED } Status;

typedef struct Block {
    int start;
    int size;
    Status status;
    char name[NAME_LEN];   //only meaningful if status == USED
    struct Block *prev;
    struct Block *next;
} Block;

Block *head = NULL;
Block *next_fit_last = NULL;  //next fit needs to remember where it left off


/* helpers */
Block *new_block(int start, int size, Status s, const char *name) {
    Block *b = malloc(sizeof(Block));
    b->start = start;
    b->size = size;
    b->status = s;
    strncpy(b->name, name ? name : "", NAME_LEN - 1);
    b->name[NAME_LEN - 1] = '\0';
    b->prev = b->next = NULL;
    return b;
}

//inserts 'node' right after 'prev_node' in the list
void insert_after(Block *prev_node, Block *node) {
    if(prev_node==NULL){
        //inserting at the very front
        node->next = head;
        node->prev = NULL;
        if (head) head->prev = node;
        head = node;
    }else{
        node->next = prev_node->next;
        node->prev = prev_node;
        if(prev_node->next) prev_node->next->prev = node;
        prev_node->next = node;
    }
}

void delete_block(Block *b) {
    if(b->prev) b->prev->next = b->next;
    else head=b->next;

    if(b->next) b->next->prev = b->prev;

    //to make sure next_fit doesn't point to a deleted block
    if(next_fit_last == b)
        next_fit_last = b->next ? b->next : head;

    free(b);
}

void reset_memory() {
    //free the whole list before starting a new strategy run
    Block *cur = head;
    while(cur){
        Block *tmp = cur->next;
        free(cur);
        cur = tmp;
    }
    head = new_block(0, MEM_SIZE, FREE, "");
    next_fit_last = head;
}


/* print current state */
void log_state() {
    printf("    %-8s %-8s %s\n", "Start", "Size", "Owner");
    printf("    ----------------------------\n");
    Block *cur=head;
    while(cur){
        if(cur->status == FREE)
            printf("    %-8d %-8d [free]\n", cur->start, cur->size);
        else
            printf("    %-8d %-8d %s\n", cur->start, cur->size, cur->name);
        cur=cur->next;
    }
    printf("\n");
}


/* split a hole to fit a process */

//takes a free block, marks the first 'size' MB as used, and creates a new free block for the leftover space (if any)
void do_alloc(Block *hole, int size, const char *name) {
    int leftover=hole->size - size;

    hole->status=USED;
    hole->size=size;
    
    strncpy(hole->name, name, NAME_LEN - 1);
    hole->name[NAME_LEN - 1]='\0';

    if(leftover > 0){
        Block *rest=new_block(hole->start + size, leftover, FREE, "");
        insert_after(hole, rest);
    }
}

/* merge free neighbours after a dealloc */
void merge_adjacent(Block *b) {

    //check right neighbour first
    if(b->next && b->next->status==FREE) {
        b->size += b->next->size;
        delete_block(b->next);
    }

    //then check left neighbour
    if(b->prev && b->prev->status==FREE) {
        b->prev->size += b->size;
        delete_block(b);
    }
}


/* ------------------------------ */
/* the four allocation algorithms */

int first_fit(const char *name, int size) {
    Block *b=head;

    while(b){
        if(b->status==FREE && b->size>=size) {
            do_alloc(b, size, name);
            return 1;
        }
        b=b->next;
    }
    return 0;  //no hole big enough
}

int next_fit(const char *name, int size) {
    if(!next_fit_last) next_fit_last = head;

    Block *start = next_fit_last;
    Block *b = start;

    //scan from where we left off, wrap around if needed
    do {
        if(b->status==FREE && b->size>=size) {
            next_fit_last = b->next ? b->next : head;
            do_alloc(b, size, name);
            return 1;
        }
        b=b->next ? b->next : head;
    } while(b!=start);

    return 0;
}


int best_fit(const char *name, int size) {
    Block *best=NULL;
    Block *b=head;

    while(b){
        if(b->status==FREE && b->size>=size) {
            if(best==NULL || b->size<best->size)
                best=b;
        }
        b=b->next;
    }

    if(!best) return 0;
    do_alloc(best, size, name);
    return 1;
}



int worst_fit(const char *name, int size) {
    Block *worst=NULL;
    Block *b=head;

    while(b){
        if(b->status==FREE && b->size>=size) {
            if(worst==NULL || b->size>worst->size)
                worst=b;
        }
        b=b->next;
    }

    if (!worst) return 0;
    do_alloc(worst, size, name);
    return 1;



}


/* de-allocation */
int terminate(const char *name) {

    Block *b=head;
    while(b){
        if (b->status==USED && strcmp(b->name, name)==0) {
            b->status=FREE;
            memset(b->name, 0, NAME_LEN);
            merge_adjacent(b);
            return 1;
        }
        b=b->next;
    }
    return 0;  //process not found
}


/* workload runner */
typedef struct {
    int is_alloc;
    char name[NAME_LEN];
    int size;
} Workload;


void run(Workload *ops, int count, const char *alg, int (*alloc)(const char *, int)) {
    printf("==============================\n");
    printf("  %s\n", alg);
    printf("==============================\n\n");

    reset_memory();

    for (int i = 0; i < count; i++) {

        Workload *op = &ops[i];
        if (op->is_alloc) {
            printf("  Allocate %d MB -> %s\n", op->size, op->name);
            if (!alloc(op->name, op->size))
                printf("  [!] Not enough memory for %s (%d MB)\n", op->name, op->size);
        } else {
            printf("  Terminate %s\n", op->name);
            if (!terminate(op->name))
                printf("  [!] %s wasn't found in memory\n", op->name);
        }
        log_state();
    }
}


/*main */
int main() {

    Workload workload[] = {
        {1,"p1",40},
        {1,"p2",20},
        {1,"p3",30},
        {1,"p4",10},
        {1,"p5",50},
        {0,"p2", 0},   //leaves a 20 MB hole
        {0,"p4", 0},   //leaves a 10 MB hole
        {1,"p6",15},
        {1,"p7", 8},
        {1,"p8",25},
        {0,"p3", 0},   //freeing P3 should merge with P4's old hole
        {1,"p9",35},
        {0,"p1", 0},
        {0,"p5", 0},
        {0,"p6", 0},
        {0,"p7", 0},
        {0,"p8", 0},
        {0,"p9", 0},
        
    };
    int n = sizeof(workload)/sizeof(workload[0]);

    run(workload, n, "First Fit", first_fit);
    run(workload, n, "Next Fit", next_fit);
    run(workload, n, "Best Fit", best_fit);
    run(workload, n, "Worst Fit", worst_fit);

    //clean up
    Block *cur = head;
    while(cur){
        Block *tmp=cur->next;
        free(cur);
        cur=tmp;
    }


}
