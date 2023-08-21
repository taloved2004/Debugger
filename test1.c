#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

void foo(){
	printf("hello\n");
}

int main() {
	int a;
	//scanf("%d", &a);
    pid_t childPid = fork();

    if (childPid < 0) {
        printf("Fork failed.\n");
        return 1;
    } else if (childPid == 0) {
        // Child process
        foo();
    } else {
        // Parent process
	printf("child_pid_from_program itself: %d\n",childPid);
        waitpid(childPid, NULL, 0); // Wait for the child to finish
	printf("son finished\n");
    }

    return 0;
}


//0x7fd807b42a50
