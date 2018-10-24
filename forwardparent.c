//most stable version

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

	pid_t child1;
	pid_t child2;
	pid_t child3;

	FILE *messageFile;
	FILE *childlog1, *childlog2, *childlog3, *parentlog;

	char messageLine[128];

	int up1[2], up2[2], up4[2]; //un-named pipes //pipefd
	int pipe_w, pipe_r; //named pipes with r,w ends

	int childNum;
	char message[messageSize];

	time_t LTime;

	remove("mypipe");

	char buffer[26];
 	int millisec;
    struct tm* tm_info;
    struct timeval tv;

    gettimeofday(&tv, NULL);

    millisec = lrint(tv.tv_usec/1000.0); // Round to nearest millisec
    if (millisec>=1000) { // Allow for rounding up to nearest second
   		 millisec -=1000;
    	tv.tv_sec++;
  	}
  	tm_info = localtime(&tv.tv_sec);

  	strftime(buffer, 26, "%Y %m %d %H:%M:%S", tm_info);

  	//printf("PARENT ID: %d\n" , getppid());

	if(pipe(up1) == -1) {
		perror("Pipe has failed");
		exit(0);
	}

	child1 = fork(); //create a new process by duplicating the existing process.

	if(child1 == -1){
		perror("Fork child 1 has failed");
		exit(0);
	} 
	else if (child1 == 0){
			
		if(pipe(up2) == -1) {
			perror("Pipe has failed");
			exit(0);	
		}

		child2 = fork(); 
		
		if (child2 == -1){
			perror("Fork child 2 has failed");
			exit(0);
		}
		else if (child2 == 0){
			close(pipe_r);
			childlog2 = fopen("Child2LogFile.txt" , "w");
			pipe_w = open("mypipe", O_WRONLY); /*write only.*/ 
			while ((read(up2[0], messageLine , messageSize)) > 0){
				
				sscanf(messageLine, "%d\t%[^\t\n]s" , &childNum , message);

				if (childNum == 2){					
					printf("From Child 2 - ID %d - %s.%03d\t%s\tKEEP\n", getppid(), buffer, millisec, message);
					fprintf(childlog2, "%s.%03d\t%s\tKEEP\n", buffer, millisec, message);
					fflush(childlog2);
				}
				else{					
					printf("From Child 2 - ID %d - %s.%03d\t%s\tFORWARD\n", getppid(), buffer, millisec, message);
					fprintf(childlog2, "%s.%03d\t%s\tFORWARD\n", buffer, millisec, message);
					fflush(childlog2);
					write(pipe_w, messageLine, messageSize);
				}
			}			
			fclose(childlog2);
		} else {
			
			childlog1 = fopen("Child1LogFile.txt" , "w");
			while((read(up1[0] , messageLine , messageSize)) > 0) {

				sscanf(messageLine, "%d\t%[^\t\n]s", &childNum , message);
				
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
			fclose(childlog1);			
		}
	} else {

		messageFile = fopen("Messages.txt", "r");
		
		while(!feof(messageFile)) {
			parentlog = fopen("ParentLogFile.txt" , "w");
			while ((read(up4[0], messageLine, messageSize)) > 0){
				sscanf(messageLine, "%d\t%[^\t\n]s", &childNum, message);

				if (childNum < 1 || childNum > 3){					
					printf("From Parent - ID %d - %s.%03d\t%s\tKEEP\n", getppid(), buffer, millisec, message);
					fprintf(parentlog, "%s.%03d\t%s\tKEEP\n", buffer, millisec, message);
					fflush(parentlog);
				}
				else {
					printf("From Parent - %s.%03d\t%s\tFORWARD\n", buffer, millisec, message);
					fprintf(parentlog, "%s.%03d\t%s\tFORWARD\n", buffer, millisec, message);
					fflush(parentlog);
					write(up1[1], messageLine, messageSize);
				}
			}
			fclose(parentlog);

			
			
			fgets(messageLine, messageSize, messageFile);
			//write(up1[1], messageLine, messageSize);			
			fclose(parentlog);
		}

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

			if(mkfifo("mypipe", 0666) == -1) {
				perror("Named pipe has failed");
				exit(0);
			}
			close(pipe_w);
			pipe_r = open("mypipe", O_RDONLY);
			childlog3 = fopen("Child3LogFile.txt" , "w");
			
			while ((read(pipe_r, messageLine , messageSize)) > 0){
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
			fclose(messageFile);
		}

		else{					
			parentlog = fopen("ParentLogFile.txt" , "w");
			while ((read(up4[0], messageLine, messageSize)) > 0){
				sscanf(messageLine, "%d\t%[^\t\n]s", &childNum, message);

				if (childNum < 1 || childNum > 3){					
					printf("From Parent - ID %d - %s.%03d\t%s\tKEEP\n", getppid(), buffer, millisec, message);
					fprintf(parentlog, "%s.%03d\t%s\tKEEP\n", buffer, millisec, message);
					fflush(parentlog);
				}
				// else {
				// 	printf("From Parent - %s.%03d\t%s\tFORWARD\n", buffer, millisec, message);
				// 	fprintf(parentlog, "%s.%03d\t%s\tFORWARD\n", buffer, millisec, message);
				// 	fflush(parentlog);
				// 	write(up1[1], messageLine, messageSize);
				// }
			}
			fclose(parentlog);
		}	   	    
	}
	return 0;
}


