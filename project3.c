//Göksel Tokur - 150116049
//Buse Batman - 150117011

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <semaphore.h>
#include <ctype.h>

//constants declaration
#define LINESIZE 200
#define NUMBEROFLINES 1000

//creating mutex and semaphores
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
sem_t semaphore_queue_read;
sem_t semaphore_queue_upper;
sem_t semaphore_queue_replace;
sem_t semaphore_queue_write;

//creating struct for the lines and array for structs
struct lines_str {
	char line_str[LINESIZE];
	int readed;		/*to check if readed before*/
	int uppered;		/*to check if uppered before*/
	int underscored;	/*to check if underscored before*/
	int written;		/*to check if written before*/
};

struct lines_str lines_struct[NUMBEROFLINES];

char filename[20];
int fileIndex = 0;
int fileIndex2 = 0;
int linecnt = 0;  /*for read count*/
int linecnt2 = 0; /*for uppercase count*/
int linecnt3 = 0; /*for replace count*/
int linecnt4 = 0; /*for write count*/
int lineEnd = 0;  /*for length of line*/	
int fileEnd = 0;  /*for length of file*/

void *readFile(void *threadid) {
	long tid;
	tid = (long)threadid;

	while (1) {
		FILE *file = fopen(filename, "r");
		//check if we came to the end of the file; if it is, then return
		if (fileIndex > fileEnd) {
			return (NULL);
		}
		//check if file opened successfully and not end of file
		if (file != NULL && feof(file) != EOF) {
			char line[LINESIZE]; 
			sem_wait(&semaphore_queue_read); //lock the semaphore
			pthread_mutex_lock(&lock); //lock the mutex
			//set the file index
			fseek(file, fileIndex, SEEK_SET);
			//read the line
			if (fgets(line, sizeof(line), file) != NULL) {
				strcpy(lines_struct[linecnt].line_str, line); // write the line
				lines_struct[linecnt].readed = 1; //change readed to 1
				printf("Read_%ld	  	Read_%ld read the line %d which is \"%s\"\n", tid, tid, linecnt, lines_struct[linecnt].line_str);
				linecnt++; //increment the line count
				//sleep(1); //wait for 1 sec
			}
			fileIndex += strlen(line); //increment the file index with the length of the line
			pthread_mutex_unlock(&lock); //unlock the mutex
			sem_post(&semaphore_queue_read); //unlock the semaphore
			fclose(file); //close the file
		}
		else{
			perror(filename); //error check
		}
	}
}

void *repToUpper(void *threadid) {
	long tid;
	tid = (long)threadid;

	int i, j;
	while (1) {
		//check if we came to the end of the file; if it is, then return
		if (linecnt2 + 1 >= lineEnd)
			return (NULL);
		//check if current looking index of the array is not empty (readed) or not uppercased before
		if (lines_struct[linecnt2].line_str != "" && lines_struct[linecnt2].uppered != 1) {
			sem_wait(&semaphore_queue_upper); //lock the semaphore
			pthread_mutex_lock(&lock); //lock the mutex
			if(linecnt2 < linecnt){
				printf("Upper_%ld	  	Upper_%ld read index %d and converted “%s”", tid, tid, linecnt2, lines_struct[linecnt2].line_str);
				//for every alphabetic character of the line, turn it to uppercase
				for (j = 0; lines_struct[linecnt2].line_str[j] != '\0'; j++) {
					lines_struct[linecnt2].line_str[j] = toupper((unsigned char)lines_struct[linecnt2].line_str[j]);
				}
				lines_struct[linecnt2].uppered = 1; //set the uppered to 1
				printf(" to “%s”\n", lines_struct[linecnt2].line_str);
				linecnt2++; //increment the line count
				//sleep(1); //wait for 1 sec
			}
			pthread_mutex_unlock(&lock); //unlock the mutex
			sem_post(&semaphore_queue_upper); //unlock the semaphore
		}
	}

	//pthread_exit(NULL);
}

void *repToUnderscore(void *threadid){
	long tid;
	tid = (long)threadid;

	int i, j;
	while (1) {
		//check if we came to the end of the file; if it is, then return
		if (linecnt3 + 1 >= lineEnd)
			return (NULL);
		//check if current looking index of the array is not empty (readed) or not underscored before
		if (lines_struct[linecnt3].line_str != "" && lines_struct[linecnt3].underscored != 1) {
			sem_wait(&semaphore_queue_replace); //lock the semaphore
			pthread_mutex_lock(&lock); //lock the mutex
			if(linecnt3 < linecnt2){
				printf("Replace_%ld	Replace_%ld read index %d and converted “%s”", tid, tid, linecnt3, lines_struct[linecnt3].line_str);
				//for every space character in the line, turn it to underscore character
				for (j = 0; lines_struct[linecnt3].line_str[j] != '\0'; j++) {
					if (lines_struct[linecnt3].line_str[j] == ' ')
						lines_struct[linecnt3].line_str[j] = '_';
				}
				lines_struct[linecnt3].underscored = 1; //set the undercased to 1
				printf(" to “%s”\n", lines_struct[linecnt3].line_str);
				linecnt3++; //increment the line count
				//sleep(1); //wait for 1 sec
			}
			pthread_mutex_unlock(&lock); //unlock the mutex
			sem_post(&semaphore_queue_replace); //unlock the semaphore
		}
	}
}

void *writeFile(void *threadid){
	long tid;
	tid = (long)threadid;

	int i, j;
	while (1) {
		//check if we came to the end of the file; if it is, then return
		if (linecnt4 + 1 >= lineEnd)
			return (NULL);
		//check if current looking index of the array is not empty (readed) or not written to file before
		if (lines_struct[linecnt4].line_str != "" && lines_struct[linecnt4].written != 1){
			sem_wait(&semaphore_queue_write); //lock the semaphore
			pthread_mutex_lock(&lock); //lock the mutex
			if (linecnt4 < linecnt3){
				//open the file in read-write mode and write the line back to the file
				FILE *fp;
				fp = fopen(filename, "r+");	
				fseek(fp, fileIndex2, SEEK_SET);
				fputs(lines_struct[linecnt4].line_str, fp); //write back to the file
				fflush(fp); 
				fileIndex2 += strlen(lines_struct[linecnt4].line_str); //increment fileIndex2 with the length of the line
				fclose(fp);
				lines_struct[linecnt4].written = 1; //set the written to 1
				printf("Writer_%ld	Writer_%ld write line %d back which is “%s”\n", tid, tid, linecnt4, lines_struct[linecnt4].line_str);
				linecnt4++; //increment the line count
				//sleep(1); //wait for 1 sec
			}
			pthread_mutex_unlock(&lock); //unlock the mutex
			sem_post(&semaphore_queue_write); //unlock the semaphore
		}
	}
}

int main(int argc, char *argv[]) {
	//error control
	if (argc != 8){ /* check for valid number of command-line arguments */
		fprintf(stderr, "ERROR: Usage: ./project3.o -d filename.txt -n numOfReadThreads numOfUpperThreads numOfReplaceThreads numOfWrtiteThreads\n");
		return 1;
	}
	else if (strcmp(argv[1], "-d") != 0){ /* check for valid character */
		fprintf(stderr, "ERROR: Usage: ./project3.o -d filename.txt -n numOfReadThreads numOfUpperThreads numOfReplaceThreads 		numOfWrtiteThreads\n");
		return 1;
	}
	else if (access(argv[2], F_OK) == -1){ /* check for given .txt file exists */
		fprintf(stderr, "ERROR: File does not exists !\n");
		return 1;
	}
	else if (strcmp(argv[3], "-n") != 0){ /* check for valid character */
		fprintf(stderr, "ERROR: Usage: ./project3.o -d filename.txt -n numOfReadThreads numOfUpperThreads numOfReplaceThreads 	 numOfWrtiteThreads\n");
		return 1;
	}

	int NUM_READ_THREADS = atoi(argv[4]);
	int NUM_UPPER_THREADS = atoi(argv[5]);
	int NUM_REPLACE_THREADS = atoi(argv[6]);
	int NUM_WRITE_THREADS = atoi(argv[7]);

	pthread_t readThreads[NUM_READ_THREADS];
	pthread_t upperThreads[NUM_UPPER_THREADS];
	pthread_t replaceThreads[NUM_REPLACE_THREADS];
	pthread_t writeThreads[NUM_WRITE_THREADS];

	//initialize the values of the array
	int k;
	for (k = 0; k < NUMBEROFLINES; k++){
		strcpy(lines_struct[k].line_str, "");
		lines_struct[k].readed = 0;
		lines_struct[k].underscored = 0;
		lines_struct[k].uppered = 0;
		lines_struct[k].written = 0;
	}

	//read filename
	strcpy(filename, argv[2]);
	FILE *file = fopen(filename, "r");
	int o;
	size_t len = 0;
	ssize_t read;
	char *line = NULL;
	
	while ((read = getline(&line, &len, file)) != -1) {
		lineEnd++;
	}

	//calculate the length of the file
	fseek(file, fileIndex, SEEK_END);
	fileEnd = ftell(file);

	int rc;
	long t;

	//mutex initialization
	if (pthread_mutex_init(&lock, NULL) != 0){
		printf("\n mutex init has failed\n");
		return 1;
	}

	//semaphore initialization
	sem_init(&semaphore_queue_read, 0, NUM_READ_THREADS);
	sem_init(&semaphore_queue_upper, 0, NUM_UPPER_THREADS);
	sem_init(&semaphore_queue_replace, 0, NUM_REPLACE_THREADS);
	sem_init(&semaphore_queue_write, 0, NUM_WRITE_THREADS);

	//createing the threads
	for (t = 0; t < NUM_READ_THREADS; t++){ /* creating read threads */
		rc = pthread_create(&readThreads[t], NULL, &readFile, (void *)t);
		if (rc){
			printf("ERROR; return code from pthread_create() is %d\n", rc);
			exit(-1);
		}
	}

	for (t = 0; t < NUM_UPPER_THREADS; t++){ /* creating read threads */
		rc = pthread_create(&upperThreads[t], NULL, &repToUpper, (void *)t);
		if (rc){
			printf("ERROR; return code from pthread_create() is %d\n", rc);
			exit(-1);
		}
	}

	for (t = 0; t < NUM_REPLACE_THREADS; t++){ /* creating replace threads */
		rc = pthread_create(&replaceThreads[t], NULL, repToUnderscore, (void *)t);
		if (rc){
			printf("ERROR; return code from pthread_create() is %d\n", rc);
			exit(-1);
		}
	}

	for (t = 0; t < NUM_WRITE_THREADS; t++){ /* creating write threads */
		rc = pthread_create(&writeThreads[t], NULL, writeFile, (void *)t);
		if (rc){
			printf("ERROR; return code from pthread_create() is %d\n", rc);
			exit(-1);
		}
	}

	//joining the threads
	for (t = 0; t < NUM_READ_THREADS; t++)
		pthread_join(readThreads[t], NULL);

	for (t = 0; t < NUM_UPPER_THREADS; t++)
		pthread_join(upperThreads[t], NULL);

	for (t = 0; t < NUM_REPLACE_THREADS; t++)
		pthread_join(replaceThreads[t], NULL);

	for (t = 0; t < NUM_WRITE_THREADS; t++)
		pthread_join(writeThreads[t], NULL);

	//destroying the mutex
	pthread_mutex_destroy(&lock);
	pthread_exit(NULL);
}
