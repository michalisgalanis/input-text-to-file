#include <stdio.h>                  // printf() support
#include <stdlib.h>                 // exit() support
#include <unistd.h>                 // pid_t type, getpid() support
#include <sys/sem.h>	            // semaphores support
#include <sys/shm.h>                // shared memory support
#include <sys/wait.h>               // wait() support

// General Values
#define BUFFER_SIZE 10

#define TERM_CHAR '.'
#define NEW_LINE '\n'

// Structures for Shared Memory
typedef struct shm_segment {        /* Used by Server and Client to communicated using shared memory */
    char buffer[BUFFER_SIZE];       // which player is playing (process ID)
} Segment;
Segment *message;


union semun {                       /* semaphore value, for semctl().     */
    int val;
    struct semid_ds *buf;
    ushort * array;
} sem_info;    


// Functions

/**
 * Prints information and exits the program
 */
void perror_exit(const char *message){
    perror(message);
    exit(EXIT_FAILURE);
}