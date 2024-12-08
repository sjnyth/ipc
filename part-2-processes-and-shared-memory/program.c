#define _SVID_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

void ChildProcess(int []);
void ParentProcess(int []);

int main(int argc, char *argv[]) {
    int ShmID, *ShmPTR, status;
    pid_t pid;

    srand(time(NULL));

    ShmID = shmget(IPC_PRIVATE, 2 * sizeof(int), IPC_CREAT | 0666);
    if (ShmID < 0) {
        printf("*** shmget error (server) ***\n");
        exit(1);
    }
    printf("Process has received a shared memory of two integers...\n");

    ShmPTR = (int *)shmat(ShmID, NULL, 0);
    if ((long)ShmPTR == -1) {
        printf("*** shmat error (server) ***\n");
        exit(1);
    }
    printf("Process has attached the shared memory...\n");

    ShmPTR[0] = 0; // Bank Account
    ShmPTR[1] = 0; // Turn

    printf("Original Bank Account = %d\n", ShmPTR[0]);

    pid = fork();
    if (pid < 0) {
        printf("*** fork error (server) ***\n");
        exit(1);
    }

    if (pid == 0) { // Child process
        for (int i = 0; i < 25; i++) {
            ChildProcess(ShmPTR);
        }
        exit(0);
    } else { // Parent process
        for (int i = 0; i < 25; i++) {
            ParentProcess(ShmPTR);
        }
        wait(&status);

        printf("Parent has detected the completion of its child...\n");
        shmdt((void *)ShmPTR);
        printf("Parent has detached its shared memory...\n");
        shmctl(ShmID, IPC_RMID, NULL);
        printf("Parent has removed its shared memory...\n");
        printf("Parent exits...\n");
    }
    return 0;
}

void ParentProcess(int SharedMem[]) {
    sleep(rand() % 6);
    while (SharedMem[1] != 0); // Wait for turn
    int account = SharedMem[0];
    if (account <= 100) {
        int balance = rand() % 101;
        if (balance % 2 == 0) {
            account += balance;
            printf("Dear old Dad: Deposits $%d / Balance = $%d\n", balance, account);
        } else {
            printf("Dear old Dad: Doesn't have any money to give\n");
        }
    } else {
        printf("Dear old Dad: Thinks Student has enough Cash ($%d)\n", account);
    }
    SharedMem[0] = account;
    SharedMem[1] = 1; 
}

void ChildProcess(int SharedMem[]) {
    sleep(rand() % 6);
    while (SharedMem[1] != 1); 
    int account = SharedMem[0];
    int balance = rand() % 51;
    printf("Poor Student needs $%d\n", balance);
    if (balance <= account) {
        account -= balance;
        printf("Poor Student: Withdraws $%d / Balance = $%d\n", balance, account);
    } else {
        printf("Poor Student: Not Enough Cash ($%d)\n", account);
    }
    SharedMem[0] = account;
    SharedMem[1] = 0; 
}
