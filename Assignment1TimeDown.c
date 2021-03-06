#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <math.h>
#include <errno.h>
#define messageSize 128

/* 
	GROUP MEMBERS :

	1. Michelle Ler Hsin Yee 0333120
	2. Chai Jin Li 0322643
	3. Chin Tuan Loong 0323317 

	-Assignment 1- 
	-closing pipes/files are important
	-timestamp formatting = tedious
	-don't let any process die before it ends

*/

/* curTime() - returns the string of the timestamp for logfile with the desired format */
char *curTime();
char *curTimes();

/* below got 2 timestamp fnc, if you want to use either timestamp functions below, just switch the s from the name curTimes() */
char *curTimes(){ 
	struct timeval tv;
	time_t nowtime;
	struct tm *nowtm;
	char tmbuf[64], buf[64];

	gettimeofday(&tv, NULL);
	nowtime = tv.tv_sec;
	nowtm = localtime(&nowtime);
	strftime(tmbuf, sizeof tmbuf, "%Y %m %d %H:%M:%S", nowtm);
	snprintf(buf, sizeof buf, "%s.%d", tmbuf, (int)(tv.tv_usec/100));
	
	char *finalTimestamp = buf;

	return finalTimestamp;
}

char *curTime(){
	// char *buffer;	
	char tmpbuffer[26], buf[64];
 	int millisec;
    struct tm* tm_info;
    struct timeval tv;

    gettimeofday(&tv, NULL);
    millisec = lrint(tv.tv_usec/1000.0); /* Round to nearest millisec */
    if (millisec>=1000) { /* Allow for rounding up to nearest second */
   		 millisec -=1000;
    	tv.tv_sec++;
  	}
  	tm_info = localtime(&tv.tv_sec);

  	strftime(tmpbuffer, 26, "%Y %m %d %H:%M:%S", tm_info);
  	snprintf(buf, sizeof buf, "%s.%03d", tmpbuffer, (int)(tv.tv_usec/100));
  	
  	char *buffer = buf;
  	return buffer;
}

int main(){	
  	/* child process variables */
	pid_t child1, child2, child3;

	FILE *messageFile;
	FILE *childlog1, *childlog2, *childlog3, *parentlog;

	int childNum;
	char messageLine[128];
	char message[messageSize];

	/* unamed pipes, up */
	int up1[2], up2[2], up4[2];

	/* named pipe */
	int namedPipe = 0;	
	char *mypipe = "mypipe";		
	unlink(mypipe);
	mkfifo(mypipe, 0666); 
	
	remove("ParentLogFile.txt");
  	printf("PARENT ID - %d %s\n", getppid(), curTime());

	if(pipe(up1) == -1) {
		perror("Pipe 1 has failed");
		exit(0);
	}

	if(pipe(up4) == -1) {
		perror("Pipe 3 has failed");
		exit(0);
	}		

	messageFile = fopen("Messages.txt", "r");
	parentlog = fopen("ParentLogFile.txt" , "w");	
	while(!feof(messageFile)) {
		fgets(messageLine, messageSize, messageFile);				
		sscanf(messageLine, "%d\t%[^\t\n]s", &childNum, message);

		if (childNum < 1){					
			printf("Parent - ID %d - %s\t%s\tKEEP\n", getppid(), curTime(), message);
			fprintf(parentlog, "%s\t%s\tKEEP\n", curTime(), message);			
		}
		else {
			printf("Parent - ID %d - %s\t%s\tFORWARD\n", getppid(),curTime(), message);
			fprintf(parentlog, "%s\t%s\tFORWARD\n", curTime(), message);			
			write(up1[1], messageLine, messageSize);
		}							
	}
	close(up1[1]);
	fclose(parentlog);	
	fclose(messageFile);
	
	child1 = fork(); 

	if(child1 == -1){
		perror("Fork child 1 has failed");
		exit(0);
	} 
	else if (child1 == 0){					
		if(pipe(up2) == -1) {
			perror("Pipe 2 has failed");
			exit(0);	
		}

		child2 = fork(); 	

		if (child2 == -1){
			perror("Fork child 2 has failed");
			exit(0);
		}
		else if (child2 == 0){			
			close(up2[1]);
			childlog2 = fopen("Child2LogFile.txt" , "w");
			namedPipe = open(mypipe, O_WRONLY); /*write only.*/ 
			while ((read(up2[0], messageLine , messageSize)) > 0){				
				sscanf(messageLine, "%d\t%[^\t\n]s" , &childNum , message);

				if (childNum == 2){					
					printf("Child 2 - ID %d - %s\t%s\tKEEP\n", getppid(), curTime(), message);
					fprintf(childlog2, "%s\t%s\tKEEP\n", curTime(),message);					
				}
				else{					
					printf("Child 2 - ID %d - %s\t%s\tFORWARD\n", getppid(), curTime(), message);
					fprintf(childlog2, "%s\t%s\tFORWARD\n", curTime(), message);					
					write(namedPipe, messageLine, messageSize);
				}
			}						
			fclose(childlog2);
			close(namedPipe);
		} else {
			close(up1[1]);			
			childlog1 = fopen("Child1LogFile.txt" , "w");			
			while((read(up1[0] , messageLine , messageSize)) > 0) {
				sscanf(messageLine, "%d\t%[^\t\n]s", &childNum , message);				
				
				if (childNum == 1) {					
					printf("Child 1 - ID %d - %s\t%s\tKEEP\n", getppid(), curTime(),message);
					fprintf(childlog1, "%s\t%s\tKEEP\n", curTime(), message);
				}
				else{
					printf("Child 1 - ID %d - %s\t%s\tFORWARD\n", getppid(), curTime(), message);
					fprintf(childlog1, "%s\t%s\tFORWARD\n", curTime(), message);					
					write(up2[1], messageLine, messageSize);	
				}
			}							
			fclose(childlog1);	
			close(up2[1]);	
		}
		wait(&child2);
	} else {
		
		child3 = fork(); 

		if (child3 == -1){
			perror("Fork child 3 has failed");
			exit(0);
		}
		else if (child3 == 0){			
			namedPipe = open(mypipe, O_RDONLY);
			childlog3 = fopen("Child3LogFile.txt" , "w");
			
			while ((read(namedPipe, messageLine , messageSize)) > 0){
				sscanf(messageLine, "%d\t%[^\t\n]s" , &childNum , message);

				if (childNum == 3){					
					printf("Child 3 - ID %d - %s\t%s\tKEEP\n", getppid(), curTime() ,message);
					fprintf(childlog3, "%s\t%s\tKEEP\n", curTime(),message);					
				}
				else{					
					printf("Child 3 - ID %d - %s\t%s\tFORWARD\n", getppid(), curTime(),  message);
					fprintf(childlog3, "%s\t%s\tFORWARD\n", curTime(),message);					
					write(up4[1], messageLine, messageSize);					
				}
			}
			fclose(childlog3);						
			close(up4[1]);			
		}
		else{							
			close(up4[1]);
			parentlog = fopen("ParentLogFile.txt" , "a");
			while ((read(up4[0], messageLine, messageSize)) > 0){
				sscanf(messageLine, "%d\t%[^\t\n]s", &childNum, message);								
				printf("Parent - ID %d - %s\t%s\tKEEP\n", getppid(), curTime(),message);
				fprintf(parentlog, "%s\t%s\tKEEP\n", curTime(), message);									
			}
			fclose(parentlog);
		}	   	    
	}	
	close(namedPipe);
	return 0;
}


	/*FIFOs are a bit strange; open() as a writer 
	will block until there's a reader, and vice versa. Worse, just like a real pipe, 
	when the writer closes its end the read end will return EOF forever; you have to close 
	and reopen (blocking for the next reader). Or you open(fifo, O_RDWR) and then you need 
	some way to know when the writer is done such as having it use only a single line or 
	having an in-band EOF packet.*/

