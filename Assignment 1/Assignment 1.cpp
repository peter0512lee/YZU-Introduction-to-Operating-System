#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/file.h>
#include <sys/mman.h>
#include <sys/wait.h>

using namespace std;


void error_and_die(const char *msg) {
	perror(msg);
	exit(EXIT_FAILURE);
}

void forkChildren (int nChildren, int *ptr) {

    int i;
    pid_t pid;	

	for(int i = 0; i < nChildren; i++) {
	
        pid = fork();
        
		if(pid == 0) {			    		

		    ptr[i] = getpid();

		    return;
		}
        
	}
	
}

void Print(int *ptr, char c[]) {

    int PID = getpid();
    
    for(int i = 0; i < 11; i++) {
        
        if (PID + i == ptr[i]) {
            
            cout << c[i];
            
        }
    
    }

}


int main() {

	int r;
	const char *memname = "sample";
	const size_t region_size = sysconf(_SC_PAGE_SIZE);

	int fd = shm_open(memname, O_CREAT| O_TRUNC| O_RDWR, 0666);

	if(fd == -1)
		error_and_die("shm_open");

	r = ftruncate(fd, region_size);

	if(r != 0)
		error_and_die("ftruncate");

	int *ptr = (int*)mmap(0, region_size, PROT_READ| PROT_WRITE, MAP_SHARED, fd, 0);
	if(ptr == MAP_FAILED)
		error_and_die("mmap");
	close(fd);	

	char c[] = "Hello World";
	
	forkChildren(11, ptr);
	
	Print(ptr, c);

	return 0;
}
