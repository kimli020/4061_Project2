#include "utils.h"

char *getChunkData(int mapperID) {
    
    /*open Message Queue*/ 
    key_t key = ftok("project",4285922);          //the key can be whatever, but it has to be the same as the one used to open the msg queue in  sendChunkData(). TA recommends the 2nd argument be student id#
    struct msgBuffer msg;                        //declare an instance of msgBuffer to represent the 1024-bit chunk
    int mid = msgget(key, PERM| IPC_CREAT);      //User, groups and other have R/W. Create queue if it doesn't already exist
 
     //error handling:
    if(mid == -1){
        return -1; 
    }

    //receive data from the master who was supposed to send a specific mapperID/
    msgrcv(mid, &msg, sizeof(msg.msgText), mapperID, 0); 

}

// sends chunks of size 1024 to the mappers in RR fashion
void sendChunkData(char *inputFile, int nMappers) {
       
  /*open Message Queue*/  
  key_t key = ftok("project", 4285922);        //has to be the same key as the one in getChuckdata()
  int msgid = msgget(key, PERM | IPC_CREAT);


  /*open file*/
  FILE* file = fopen (inputFile, "r");
  char chunk[chunkSize];                           //defined in "utils.h"

  //error handling:
  if(file == NULL){
    printf("This file could not be opened");
    exit(ERROR);
  }

 

 
  char tempString[chunkSize];                      //temporary string storage
  memset(wholeString, '\0', chunkSize);             //To make sure it is always null-terminated
  int charCount = 0;                                //use this to see if a word is shared between the end of one chunk and the beginning of the next
  int currentChunkSize = 0;
  int send1;
  mapperID = 0;
  
  /*construct 1024 byte chunks and send them to a mapper*/
  while ((int checkString = fgetc (file)) != EOF){                      //if the pointer is not at end-of-file...
    charCount++;                                                       //a character has been read, so increment the character count...
    
    if(checkString != ' '){                                             //if the the character is not a space, i am in the middle of a word, so add it to the temp string storage.
      strcat(tempString,checkString);   
      
    }else{                                                              //at this point we have read a space...
      currentChunkSize += (charCount-1);                                //the "-1" is to account for the space
      
      if(currentChunkSize <= chunkSize){                                    //i have read a space, so i am at the end of a word, and I have read less than or equal the chunk size
        strcat(chunk,tempString);                                           //concatenate the temporary string i have been building to the chunk
        strcat(chunk,checkString);                                           //add the whitespace character
        
        if(currentChunkSize == chunkSize){                                  //once i have reached 1024 bytes, send the chunk
          send1 = msgsnd(msgid, (void *) &chunk, mapperID)
          
          if(mapperID < nMappers){
            mapperID++;
          }
          
          /*Reset everything after a chunk as been sent*/
          charCount = 0;
          currentChunkSize = 0;
          memset(chunk, '\0', chunkSize);
        }
        
        memset(tempString, '\0', chunkSize);
        
      }else if(currentChunkSize > chunkSize){                               //DO NOT concatenate the next word to the chunk if it would put it over 1024 bytes
        
        send1 = msgsnd(msgid, (void *) &chunk, mapperID)
        
        if(mapperID < nMappers){
          mapperID++;
        }
        
       /*Reset everything after a chunk as been sent*/
        currentChunkSize = 0;
        memset(chunk, '\0', chunkSize);)

        currentChunkSize += (charCount-1);                 //at this point, I want to go back to line 29...not sure how to implement this
      }
    }
  }

  /*Send to Mapper*/
  int send2;
  message msg;
  for (int i=0; i< nMappers; i++){
    msg.mtype = i+1;
    memset(msg.mtext, '\0', MSGSIZE);
    sprintf(msg.mtext, "END");
    send2 = msgsnd(msgid, (void *)&msg, MSGSIZE, i);
  }

  //is this for the ACK??
  for (int i =0; i<nMappers; i++){
    wait(msgid);
  }

  /*Close everything*/
  msgctl(msgid, IPC_RMID, NULL);
  fclose(file);
  
    
}

// hash function to divide the list of word.txt files across reducers
//http://www.cse.yorku.ca/~oz/hash.html
int hashFunction(char* key, int reducers){
	unsigned long hash = 0;
    int c;

    while ((c = *key++)!='\0')
        hash = c + (hash << 6) + (hash << 16) - hash;

    return (hash % reducers);
}

int getInterData(char *key, int reducerID) {
}

void shuffle(int nMappers, int nReducers) {
}

// check if the character is valid for a word
int validChar(char c){
	return (tolower(c) >= 'a' && tolower(c) <='z') ||
					(c >= '0' && c <= '9');
}

char *getWord(char *chunk, int *i){
	char *buffer = (char *)malloc(sizeof(char) * chunkSize);
	memset(buffer, '\0', chunkSize);
	int j = 0;
	while((*i) < strlen(chunk)) {
		// read a single word at a time from chunk
		// printf("%d\n", i);
		if (chunk[(*i)] == '\n' || chunk[(*i)] == ' ' || !validChar(chunk[(*i)]) || chunk[(*i)] == 0x0) {
			buffer[j] = '\0';
			if(strlen(buffer) > 0){
				(*i)++;
				return buffer;
			}
			j = 0;
			(*i)++;
			continue;
		}
		buffer[j] = chunk[(*i)];
		j++;
		(*i)++;
	}
	if(strlen(buffer) > 0)
		return buffer;
	return NULL;
}

void createOutputDir(){
	mkdir("output", ACCESSPERMS);
	mkdir("output/MapOut", ACCESSPERMS);
	mkdir("output/ReduceOut", ACCESSPERMS);
}

char *createMapDir(int mapperID){
	char *dirName = (char *) malloc(sizeof(char) * 100);
	memset(dirName, '\0', 100);
	sprintf(dirName, "output/MapOut/Map_%d", mapperID);
	mkdir(dirName, ACCESSPERMS);
	return dirName;
}

void removeOutputDir(){
	pid_t pid = fork();
	if(pid == 0){
		char *argv[] = {"rm", "-rf", "output", NULL};
		if (execvp(*argv, argv) < 0) {
			printf("ERROR: exec failed\n");
			exit(1);
		}
		exit(0);
	} else{
		wait(NULL);
	}
}

void bookeepingCode(){
	removeOutputDir();
	sleep(1);
	createOutputDir();
}