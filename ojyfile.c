#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <fcntl.h> 
#include <sys/stat.h> 

#define MSGSIZE 128

/* The index of the "read" end of the pipe */
#define READ 0
/* The index of the "write" end of the pipe */
#define WRITE 1

char * getCurrentTimeStamp();
void writeFile(char *, char * ,int);

int main(void){
	int child1,child2,child3; //childs 
	int unnamedPipe[2]; //parent to child 1
	int uPipe2[2]; // ch1 to ch2 
	int uPipe3[2]; //child to parent
	FILE *fp; //file pointer to read from msg file
	char str[MSGSIZE];
	char message[MSGSIZE];
	int childNum;
	char receivedMsg[MSGSIZE];

	// named pipe initialization
	int namedPipe;
	char * myfifo = "./myfifo";
	unlink(myfifo); //if exist DELETE it, so its empty 
	mkfifo(myfifo, 0666); 



	printf("PARENT PID(MY) %d \n", (int)getpid()); //determine parent PID for debug

	printf("%s \n",getCurrentTimeStamp()); //timestamp debug REMOVE IN RTM 


	//create 2 unmamed pipe
	if(pipe(unnamedPipe)==-1){
		perror("Pipe Error!");
		return 1; //exit if error
	}

	if(pipe(uPipe3)==-1){
		perror("PIPE3 Error!");
		return 1; //exit if error
	}
	
	child1=fork();

	if(child1==0){ //child process

		if(pipe(uPipe2)==-1){ //create new pipe! AHAHHAHAHAH
			perror("Pipe 2 Error! ");
			return 1; //exit if error
		}
		child2=fork(); //make babies 

		printf("My Parent is %d , my pid is %d\n", (int)getppid(), (int)getpid());


		close(unnamedPipe[WRITE]); //MUST CLOSE before READ the Pipe, else the child wont DIE , fork might not runing in seq, might run parallel
		while( read(unnamedPipe[READ], message, MSGSIZE) > 0 ){
			//read(unnamedPipe[READ], message, MSGSIZE);

			sscanf(message, "%d\t%[^\n]" ,&childNum, receivedMsg);
			if(childNum==1){
				writeFile(receivedMsg, "child1Log.txt", 0);
			}else{
				writeFile(receivedMsg, "child1Log.txt", 1);
				write(uPipe2[WRITE],message,MSGSIZE);
				//dont close prematuraly 
				
			}
		}
		close(uPipe2[WRITE]);
		close(unnamedPipe[READ]);


		

		if(child2==0){ 
			//child 2 will use a named pipe to pipe to child 3 , will explain when im sober

			printf("My Parent is %d , my pid is %d\n", (int)getppid(), (int)getpid());

			close(uPipe2[WRITE]); //Just be safe 
			while( read(uPipe2[READ], message, MSGSIZE) > 0 ){
				sscanf(message, "%d\t%[^\n]" ,&childNum, receivedMsg);
				if(childNum==2){
					writeFile(receivedMsg, "child2Log.txt", 0);
				}else{
					writeFile(receivedMsg, "child2Log.txt", 1);
					//open the name pipe in WRiteONLY!
					namedPipe = open(myfifo,O_WRONLY);

					write(namedPipe,message,MSGSIZE);
					
				}
			}
			close(namedPipe); //close after writing
			close(uPipe2[READ]);
			

		}

		wait(&child2);

	}else{ //parent

			fp = fopen("Messages.txt","r");

			if(fp==NULL){
				printf("File Not Found!");
			}else{

				while (fgets(str, MSGSIZE, fp) != NULL){
					char writeOutMsg[MSGSIZE];
					//read the MsgFile to the Pipe 
					strtok(str, "\n"); //remove trailing newline
					sscanf(str, "%d\t%[^\n]" ,&childNum, receivedMsg);
					// printf("Parent:  Wrote '%s' to pipe!\n", str);					
					if(childNum==0){
							writeFile(receivedMsg, "parentLog.txt", 0);
					}else{
						writeFile(receivedMsg, "parentLog.txt", 1);
						write(unnamedPipe[WRITE],str, MSGSIZE );
						
					}
				} //end while loop
				close(unnamedPipe[WRITE]); //close after writing

				fclose(fp);
			}




			child3=fork();
			if(child3==0){
				printf("My Parent is %d , my pid is %d\n", (int)getppid(), (int)getpid());	
				//open the PIPE in READONLY
				close(namedPipe); //make sure its closed before opening
				namedPipe = open(myfifo,O_RDONLY);

				while( read(namedPipe, message, MSGSIZE) > 0 ){
					sscanf(message, "%d\t%[^\n]" ,&childNum, receivedMsg);
					if(childNum==3){
							writeFile(receivedMsg, "child3Log.txt", 0);
					}else{
						writeFile(receivedMsg, "child3Log.txt", 1);
						write(uPipe3[WRITE],message, MSGSIZE);
					}
				}
				close(uPipe3[WRITE]);
				close(namedPipe); //close after reading
			}else{
				//Parent again 

				close(uPipe3[WRITE]); //just to be safe again 
				while( read(uPipe3[READ], message, MSGSIZE) > 0 ){
					//All is yours
					sscanf(message, "%d\t%[^\n]" ,&childNum, receivedMsg);
					writeFile(receivedMsg, "parentLog.txt", 0);
				}
				close(uPipe3[READ]);
		

			}
			// wait(NULL);
			// wait(&child1);
			// wait(&child3);
			
	}


	return 0;
}


char *getCurrentTimeStamp(){
	//ref: https://stackoverflow.com/questions/2408976/struct-timeval-to-printable-format
	//modified for assignment
	//dont touch this, can work liao, work for few hours just to get timestamp
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

void writeFile(char *msg, char *filename, int operations){
//Operations 0 = Keep, 1 = Forward
	char logFile[80];
	mkdir("log",0777);
	strcpy(logFile,"log/");
	strcat(logFile, filename);

	FILE *f = fopen(logFile, "a");
	if (f == NULL)
	{
	    printf("Error opening file!\n");
	    exit(1);
	}
	char *text = "ERROR";
	if(operations==0){
		text = "KEEP";
	}else{
		text = "FORWARD";
	}
	
	/* print some text */
	fprintf(f, "%s \t %s %s\n", getCurrentTimeStamp(), msg, text);
	fclose(f);


}