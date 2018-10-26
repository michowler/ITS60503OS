//most stable version
//1. added in unlink
//2. change order of passing
//3. 2 up in parent, 1 named in parent, 1 up in child 1
//4. parent to child 1 - read message, write up[1]
//5. child 1 to child 2 - read up1[0], write up2[1]
//6. child 2 to child 3 - read up2[0], write pipe(write)
//7. child 3 to parent - pipe (read), up4[1]
//8. parent - read up4[0]
//if still cannot, try changing pipe to only one end

/* 
Michelle Ler Hsin Yee 0333120
Michelle Ler Hsin Yee 0333120
Michelle Ler Hsin Yee 0333120
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <math.h>

#define messageSize 128

int main(){

	char buffer[26]; 
 	int millisec;
    struct tm* tm_info;
    struct timeval tv;

    gettimeofday(&tv, NULL);

    millisec = lrint(tv.tv_usec/1000.0); //round to nearest millisec
    if (millisec>=1000) { //allow for rounding up to nearest second
   		 millisec -=1000;
    	tv.tv_sec++;
  	}
  	tm_info = localtime(&tv.tv_sec);

  	strftime(buffer, 26, "%Y %m %d %H:%M:%S", tm_info); //format for timestamp

  	//create child process variables
	pid_t child1, child2, child3;

	FILE *messageFile;
	FILE *childlog1, *childlog2, *childlog3, *parentlog;

	int childNum;
	char messageLine[128];
	char message[messageSize];

	int up1[2], up2[2], up4[2]; //un-named pipes //pipefd
	int pipe_w, pipe_r, namedPipe; //named pipes with r,w ends
	
	char *mypipe = "mypipe";		
	unlink(mypipe);
    mkfifo(mypipe, 0666);/* create the FIFO (named pipe) */

  	printf("PARENT ID: %d\n" , getppid());

	if(pipe(up1) == -1) {
		perror("Unamed Pipe 1 has failed");
		exit(0);
	}

	if(pipe(up4) == -1) {
		perror("Unamed Pipe 3 has failed");
		exit(0);
	}	

	// FIFOs are a bit strange; open() as a writer 
	//will block until there's a reader, and vice versa. Worse, just like a real pipe, 
	//when the writer closes its end the read end will return EOF forever; you have to close 
	//and reopen (blocking for the next reader). Or you open(fifo, O_RDWR) and then you need 
	//some way to know when the writer is done such as having it use only a single line or 
	//having an in-band EOF packet.

	messageFile = fopen("Messages.txt", "r");
	close(up4[1]);
	while(!feof(messageFile)) {
		fgets(messageLine, messageSize, messageFile);
		parentlog = fopen("ParentLogFile.txt" , "w");
		
		sscanf(messageLine, "%d\t%[^\t\n]s", &childNum, message);

		if (childNum < 1){					
			printf("From Parent - ID %d - %s.%03d\t%s\tKEEP\n", getppid(), buffer, millisec, message);
			fprintf(parentlog, "%s.%03d\t%s\tKEEP\n", buffer, millisec, message);
			fflush(parentlog);
		}
		else {
			printf("From Parent - ID %d - %s.%03d\t%s\tFORWARD\n", getppid(), buffer, millisec, message);
			fprintf(parentlog, "%s.%03d\t%s\tFORWARD\n", buffer, millisec, message);
			fflush(parentlog);
			write(up1[1], messageLine, messageSize);
		}
		
		fclose(parentlog);		
		close(up1[1]);
	}
	fclose(messageFile);
	
	child1 = fork(); //create a new process by duplicating the existing process.

	if(child1 == -1){
		perror("Fork child 1 has failed");
		exit(0);
	} 
	else if (child1 == 0){		
			
		if(pipe(up2) == -1) {
			perror("Unamed Pipe 2 has failed");
			exit(0);	
		}

		child2 = fork(); 
		
		if (child2 == -1){
			perror("Fork child 2 has failed");
			exit(0);
		}

		else if (child2 == 0){
			printf("hi 6");
			// close(pipe_r);
			childlog2 = fopen("Child2LogFile.txt" , "w");
			namedPipe = open(mypipe, O_WRONLY); /*write only.*/ 
			while ((read(up2[0], messageLine , messageSize)) > 0){
				printf("hi 7");
				sscanf(messageLine, "%d\t%[^\t\n]s" , &childNum , message);

				if (childNum == 2){					
					printf("From Child 2 - ID %d - %s.%03d\t%s\tKEEP\n", getppid(), buffer, millisec, message);
					fprintf(childlog2, "%s.%03d\t%s\tKEEP\n", buffer, millisec, message);
					//fflush(childlog2);
				}
				else{					
					printf("From Child 2 - ID %d - %s.%03d\t%s\tFORWARD\n", getppid(), buffer, millisec, message);
					fprintf(childlog2, "%s.%03d\t%s\tFORWARD\n", buffer, millisec, message);
					//fflush(childlog2);
					write(namedPipe, messageLine, messageSize);
				}
			}			
			printf("hi 8");
			fclose(childlog2);
			close(namedPipe);

		} else {

			close(up1[1]);
			printf("hi 1");
			childlog1 = fopen("Child1LogFile.txt" , "w");
			printf("hi 2");
			while((read(up1[0] , messageLine , messageSize)) > 0) {
				printf("hi 3");

				sscanf(messageLine, "%d\t%[^\t\n]s", &childNum , message);
				printf("hi 4");
				
				if (childNum == 1) {					
					printf("From Child 1 - ID %d - %s.%03d\t%s\tKEEP\n", getppid(), buffer, millisec, message);
					fprintf(childlog1, "%s.%03d\t%s\tKEEP\n", buffer, millisec, message);
					fflush(childlog1);
				}
				else{
					printf("From Child 1 - ID %d - %s.%03d\t%s\tFORWARD\n", getppid(), buffer, millisec, message);
					fprintf(childlog1, "%s.%03d\t%s\tFORWARD\n", buffer, millisec, message);
					fflush(childlog1);
					write(up2[1], messageLine, messageSize);	
				}
			}				
			printf("hi 5");
			fclose(childlog1);	
			close(up2[1]);	
		}
	} else {
	
		if(pipe(up4) == -1) {
			perror("Pipe has failed");
			exit(0);
		}
		
		child3 = fork(); /*create a new process by duplicating the existing process.*/ 
		if (child3 == -1){
			perror("Fork child 3 has failed");
			exit(0);
		}
		else if (child3 == 0){

			// close(pipe_w);
			namedPipe = open(mypipe, O_RDONLY);
			childlog3 = fopen("Child3LogFile.txt" , "w");
			
			while ((read(namedPipe, messageLine , messageSize)) > 0){
				sscanf(messageLine, "%d\t%[^\t\n]s" , &childNum , message);

				if (childNum == 3){					
					printf("From Child 3 - ID %d - %s.%03d\t%s\tKEEP\n", getppid(), buffer, millisec, message);
					fprintf(childlog3, "%s.%03d\t%s\tKEEP\n", buffer, millisec, message);
					fflush(childlog3);
				}
				else{					
					printf("From Child 3 - ID %d - %s.%03d\t%s\tFORWARD\n", getppid(), buffer, millisec, message);
					fprintf(childlog3, "%s.%03d\t%s\tFORWARD\n", buffer, millisec, message);
					fflush(childlog3);
					write(up4[1], messageLine, messageSize);
					
				}
			}
			fclose(childlog3);						
			close(up4[1]);
		}

		else{				
			wait(NULL);
			close(up4[1]);
			parentlog = fopen("ParentLogFile.txt" , "w");
			while ((read(up4[0], messageLine, messageSize)) > 0){
				sscanf(messageLine, "%d\t%[^\t\n]s", &childNum, message);								
				printf("From Parent - ID %d - %s.%03d\t%s\tKEEP\n", getppid(), buffer, millisec, message);
				fprintf(parentlog, "%s.%03d\t%s\tKEEP\n", buffer, millisec, message);
				fflush(parentlog);						
			}
			fclose(parentlog);
		}	   	    
	}
	return 0;
}


