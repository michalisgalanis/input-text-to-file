#include "util.h"

void serverChild(key_t sem_id, Segment *message);
void serverParent(key_t sem_id, Segment *message, char* output_filename);
void sem_lock(int sem_id);
void sem_unlock(int sem_id);

int main(int argc, char *argv[]){ //input params: input_file
    // G0. Checking parameters
    if (argc != 2) perror_exit("usage: output!");


    // G1. Create shared memory segment and attach to it
    message = (Segment *)malloc(sizeof(Segment));

    key_t shm_key = ftok(argv[0], 1); // create shared memory identifier (key)
    if (shm_key == -1) perror_exit("Error creating key!");

    int shm_id = shmget(shm_key, sizeof(Segment), 0666 | IPC_CREAT); // create shared memory segment
    if (shm_id == -1) perror_exit("Erorr creating shared memory segment!");

    Segment *segment = shmat(shm_id, NULL, 0); // attach to shared memory segment created
    if (segment == (void *) -1) perror_exit("Error attaching to shared memory segment!");
    message = segment;

    for (int i = 0; i < BUFFER_SIZE; i++) message->buffer[i] = 0;

    // G2. Create semaphores
    key_t sem_key = ftok(argv[0], 1); // create semaphore identifier (key)
    if (sem_key == -1) perror_exit("Error creating key!");

    int sem_id = semget(sem_key, 1, 0666 | IPC_CREAT); // create semaphore set with 1 semaphore
    if (sem_id == -1) perror_exit("Error creating semaphore set!");

    sem_info.val = 1;
    int rc = semctl(sem_id, 0, SETVAL, sem_info);
    if (rc == -1) perror_exit("Error initializing semaphore set");
    
    // G3. Create child reader process
    pid_t pid;
    switch (pid = fork()){
        case (-1):
            perror_exit("Error creating child!");
        case (0):
            serverChild(sem_id, message);
            exit(0);
            break;
        default:
            serverParent(sem_id, message, argv[1]);
            break;
    }

    // P7. Cleanup
    while(wait(NULL) >= 0); //Waiting for children to terminate
    printf("[Parent %d] : Child Exited. Killing Parent\n", getpid());
    
    int shmdt_status = shmdt(segment); // detach from shared memory segment
    if (shmdt_status == -1) perror_exit("Error detaching from shared memory segment!");

    return 0;
}

// Process functions

/**
 *  Child Process Function. Reads repeatedly 10 characters from stdin
 *  and writes to shared memory segment. Communication with parent is
 *  one through the use of semaphores.
 */
void serverChild(key_t sem_id, Segment *message){
    while (1){
        // C4. Read 10 chars from stdin and write to shared memory
        sem_lock(sem_id); // start of critical section
        system ("/bin/stty raw"); // make terminal send all keystrokes to stdin, SYSTEM DEPENDENT!

        printf("\n[Child %d] : Enter characters: ", getpid());
        for (int i = 0; i < BUFFER_SIZE; i++){
            char ch = getchar();
            if (ch == TERM_CHAR) {
                message->buffer[i] = 0;
                system ("/bin/stty cooked"); // make terminal normal again
                printf("\n[Child %d] : '.' terminating character detected. Exiting!\n", getpid());
                sem_unlock(sem_id);
                return;
            }
            message->buffer[i] = ch;
        }

        system ("/bin/stty cooked"); // make terminal normal again
        sem_unlock(sem_id); // end of critical section
    }
}

/**
 *  Parent Process Function. Reads repeatedly 10 characters from shared
 *  memory and writes to output file. Communication with child is done 
 *  through the use of semaphores.
 */
void serverParent(key_t sem_id, Segment *message, char* output_filename){
    // Setup output file
    FILE *fp = fopen(output_filename, "w");

    sleep(0.1); // let the child start first
    while (1){
        // P4. Read 10 chars from shared memory and write to output.txt
        sem_lock(sem_id); // start of critical section

        printf("\n");
        for (int i = 0; i < BUFFER_SIZE; i++){
            char ch = message->buffer[i];
            if (ch == 0){
                printf("\n[Parent %d] : '.' terminating character detected. Exiting!\n", getpid());
                sem_unlock(sem_id);
                fclose(fp);
                return;
            }
            printf("[Parent %d] : Character %d: %c (%d)\n", getpid(), (i+1), ch, ch);
            fputc(ch, fp);
        }

        sem_unlock(sem_id); // end of critical section
        sleep(0.1);
    }
}


// Semaphore functions

/**
 * Locks the semaphore with specific id for exclusive access to a resource (Decrease value by 1)
 */
void sem_lock(int sem_id) {
    struct sembuf sem_op = {0, -1, 0};
    int status = semop(sem_id, &sem_op, 1);
    if (status == -1) perror_exit("Error locking semaphore!");
}

/**
 * Unlocks the semaphore with specific id (Increase value by 1)
 */
void sem_unlock(int sem_id) {
    struct sembuf sem_op = {0, 1, 0};
    int status = semop(sem_id, &sem_op, 1);
    if (status == -1) perror_exit("Error unlocking semaphore!");
}