# ITS60503OS
Operating system assignment 1

## This are the instructions of the assignment.
Here is what our programme will do:
1. Parent forks a child process
2. Child 1 process will fork another child process - Child 2
3. The parent process will fork another child process - Child 3.
4. Piped messages will need to be used to ensure the correct order of events. After each process creation, the code will create an unnamed pipe from Parent process
to Child 1 process, and from Child 1 process to Child2 process.
5. The code will also create a named pipe from Child 2 process to Child 3 process.
6. Child 3 process will also have an unamed pipe back to the Parent process.
7. Pass the messages through the pipes from the parent - child 1 - child 2 - child 3 - back to the parent.
8. Each child process, as it receives a message will read the number and determine if it is the intended recipient of the message. It will then log the message as described above and then either keep the message or pass it to the next child process in line. 
(Note that a message to “Child 4” or any child number that is not available is to be passed through all three child processes and then back to the parent.)
9. A log entry will be: <timestamp<tab><full_message from or to pipe><tab><KEEP or FORWARD>.
     
## To use : Edit your text file for message passing

```
1    This message is for child 1 of the main process.
2    This message is for child 2 of the main process.
3    This message is to the child of the first child which is the
grand-child of the main processs.
0    This will be the main process itself.
4    This and any other number which does not belong to any child
processes belongs to parent (or main) process.
2    Process numbers need not come in sequence.
3    Numbers can be repeated.
```
