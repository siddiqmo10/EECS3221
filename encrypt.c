/*
Family Name: Siddiq
Given Name:  Mohammed
Section: Z
Student Number: 213054614
CS Login: siddiqmo
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#define TEN_MILLIS_IN_NANOS 10000000

// some global variables
int activeinput;
int activework;
int size_b = 0;
FILE *input;
FILE *output;

// mutex locks for threads
pthread_mutex_t in;
pthread_mutex_t work;
pthread_mutex_t out;

// single item character in buffer
typedef struct{
	char data;
	off_t offset;
	char state;
} BufferItem;

BufferItem *buff;

// putting the thread to sleep for a random amount of time
void sleep_nano(void){
	struct timespec t;
	int seed = 0;
	t.tv_sec = 0;
	t.tv_nsec = rand_r((unsigned int*)&seed)%(TEN_MILLIS_IN_NANOS+1);
	nanosleep(&t, NULL);
}


//check if the there is an empty buffer by returning 1 is there exsists an empty buffer and -1 if it doesn't exsist an empty buffer
int empty(){
    int k= 0;
    while(k<size_b){
        //char state = buff[k].state;
        if (buff[k].state == 'e'){
            return 1; //empty buffer is present
        }
        k++;
    }
    return -1; //no empty buffer
}



// add onto the empty buffer
int add_empty(){
    int k =0;
    if (empty()){
        while(k<size_b){
     
        if (buff[k].state == 'e'){
            return k; //empty buffer is present
        }
        k++;
    }

    }
        return -1; //no empty buffer
}


//add onto the working buffer by returning an int value if there exsists a working buffer to add on and -1 if it doesn't exsist a working buffer
int working(){
        int k= 0;
    while(k<size_b){
        if (buff[k].state == 'w'){
            return k; //working buffer is present
        }
        k++;
    }
    return -1; //no working buffer
    
}



//add onto the output buffer by returning an int value if there exsists a output buffer to add on and -1 if it doesn't exsist a output buffer
int output_buff(){
        int k= 0;
    while(k<size_b){
        if (buff[k].state == 'o'){
            return k; //outpupt buffer is present
        }
        k++;
    }
    return -1; //no output buffer
    
}


// reading input from test file and encrypting and placing it in buffer
void *input_thread(void *arg) {
    int bufferNum;
    char file_char;
    off_t offsetNum;
    
    
    
    //Each IN thread goes to sleep (use nanosleep) for some random timeEach IN thread goes to sleep (use nanosleep) for some random time
   
    sleep_nano();
    
   while(!feof(input)){
       
    pthread_mutex_lock(&work); //open work brace   
       
    // We start the main code from here
    // initialize empty buffer
    bufferNum = add_empty();
    
   
    while(bufferNum > -1){
        
        if (empty() == 1){
            sleep_nano();
        }
        
        pthread_mutex_lock(&in); // open in brace 
        
        // Critical section
        
        offsetNum = ftell(input); //current value of the position indicator
        file_char = fgetc(input);
        pthread_mutex_unlock(&in); // close in brace
        if (file_char == EOF){
            break; //exit the loop as we reached the end of the file
        }
        else {
        // set the characters of the struct
        buff[bufferNum].offset = offsetNum;
        buff[bufferNum].state = 'w'; //set to w since we read it
        buff[bufferNum].data = file_char;
        bufferNum = add_empty(); // next empty item
        }
    }
        pthread_mutex_unlock(&work); // close work brace 
    
   }
    
        sleep_nano();
        pthread_mutex_lock(&work); // open work brace 
        
    
        // Critical section    
        activeinput--;
    
        pthread_mutex_unlock(&work); // close work brace 
        pthread_exit(0);
    
    
}

// reading encrypted from buffer and decrypt it and placing it in buffer
void *work_thread(void *arg){
    
    int key = atoi(arg); //key from the command line
    int bufferNum = 0;
    char file_char;
    int active;
    
    // as per professor's requirement sleep for some time
    sleep_nano();
    
   while(bufferNum > -1 || active > 0){
       
        pthread_mutex_lock(&work); //open work brace to read what has to be encrypted
      
       // Critical section
       bufferNum = working();
               
        
        if (bufferNum > -1){
            file_char = buff[bufferNum].data; // input char stored
            
            if (empty() == 1){
               sleep_nano();
            }
            
            if (file_char == EOF || file_char == '\0'){
				break;
			}
			
                // encryption if k >= 0
                // encypt code from the professor
                if(key > 0 && file_char>31 && file_char<127){

                file_char = (((int)file_char-32)+2*95+key)%95+32;
                }
            
                 // decryption if key < 0
                 else if (key < 0 && file_char>31 && file_char<127 ){

                     file_char = (((int)file_char-32)+2*95-(-1*key))%95+32 ;
                 }
    
            
            buff[bufferNum].data = file_char;// setting up the buffer withupdated value
            buff[bufferNum].state = 'o'; // set state to o for output
        }
        
        active = activeinput;
        pthread_mutex_unlock(&work); //close work brace
        
    }
    
    // as per professor's requirement sleep for some time
    sleep_nano();
    
    pthread_mutex_lock(&work); //open work brace
    
    // critical section
    activework--;
    
    pthread_mutex_unlock(&work); //close work brace
    pthread_exit(0);
    
    
}

// output the decrypted characters from the buffer to the file
void *out_thread(){
   
    int bufferNum = 0;
    char file_char;
    off_t offsetNum;
    int active;
    
   // as per professor's requirement sleep for some time
    sleep_nano();
    
     while(bufferNum > -1 || active > 0){
            
            pthread_mutex_lock(&work); // open work brace
            bufferNum = output_buff();
          
          // critical section
            if (bufferNum > -1){
             
            file_char = buff[bufferNum].data; // input char
            offsetNum = buff[bufferNum].offset; //current value of the position indicator
                
            if (empty() == 1){
               sleep_nano();
            }
           
            pthread_mutex_lock(&out); // open out brace
            
                //critical section output to file
                fseek(output, offsetNum, SEEK_SET);
                fputc(file_char,output);
                
            pthread_mutex_unlock(&out); // close out brace
                
            buff[bufferNum].data = '\0';// setting up the buffer to be empty
            buff[bufferNum].state = 'e'; // set state to e for empty
            buff[bufferNum].offset = 0; // clear offset
                
        }
            active = activework;
            pthread_mutex_unlock(&work); //close work brace               
      }

    sleep_nano();
    pthread_exit(0);
    
}


//initialize the buffer to be empty
void init_buffer(){
    int k;
    for(k = 0; k < size_b; k++){
		buff[k].state = 'e';
	} 
}


int main(int argc, char **argv){
    
    // encrypt <KEY> <nIN> <nWORK> <nOUT> <file_in> <file_out> <bufSize>
    
    // Getting all the information and assinging them accordingly
    int key = atoi(argv[1]);
    int in_buffer = atoi(argv[2]);
    int work_buffer = atoi(argv[3]);
    int out_buffer = atoi(argv[4]);
    input = fopen(argv[5],"r");
    output = fopen(argv[6],"w");
    size_b = atoi(argv[7]);
    activeinput = in_buffer;
    activework = work_buffer;
    int k = 0;

    // error checks
    
    if (argc<8){
        printf("Error pleasae follow follwing format <KEY> <nIN> <nWORK> <nOUT> <file_in> <file_out> <bufSize>");
        exit(1);
    }
    
    if(input == NULL || output == NULL){
        printf("fopen error");
    }
    //if (key < 0 || key > 127){
    if (key < -127 || key > 127){
        printf("Error! key is out of valid range");
        exit(1);
    }
    
    if (in_buffer < 1|| out_buffer < 1 || work_buffer < 1 ) {
        printf("Error! Incorrect number of threads");
        exit(1);
    }
    
    //initialization of threads
    pthread_mutex_init(&in, NULL); //initialize 
	pthread_mutex_init(&work, NULL); //initialize 
	pthread_mutex_init(&out, NULL); //initialize
    pthread_t inThread[in_buffer]; //input thread created
    pthread_t workThread[work_buffer]; //work thread created
    pthread_t outThread[out_buffer]; //out thread created
    pthread_attr_t attr;
	pthread_attr_init(&attr);
    buff = (BufferItem*)malloc(sizeof(BufferItem)*size_b); // allocating memory for Bufferitem
    init_buffer();
    
    
    
    // create threads
    while(k < in_buffer){
      pthread_create(&inThread[k], &attr, (void *) input_thread, input);
      k++;
  }
    k=0;
    while(k < work_buffer){
        pthread_create(&workThread[k], &attr, (void *) work_thread, argv[1]);
        k++;
  }
    k=0;
    while(k < out_buffer){
        pthread_create(&outThread[k], &attr, (void *) out_thread, output);
        k++;
  }
   
    //jjoin the threads
    k=0;  
    while(k < in_buffer){
        pthread_join(inThread[k],NULL);
        k++;
  }
    
     k=0;  
    while(k < work_buffer){
        pthread_join(workThread[k],NULL);
        k++;
  }
     k=0;  
    while(k < out_buffer){
        pthread_join(outThread[k],NULL);
        k++;
  }
    
    
    //close files
    fclose(input);
    fclose(output);
    free(buff);
    
    return 1;
    
}
