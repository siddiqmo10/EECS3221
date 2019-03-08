/*

# Family Name: Siddiq   

# Given Name: Mohammed  

# Section: N/A

# Student Number: 213054614

# CSE Login: siddiqmo

*/

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <sys/types.h>
#include <sys/wait.h>

struct WordArray {
	char word[101];
	int freq;
} WordArray;

struct Output{
	char med_word[101];
	char file[101];
	int size;
} Output;

int compare_word(const void *r, const void *s);
int compare_freq(const void *p, const void *q);
int sort(const void *a, const void *b);


int main (int argc, char *argv[]){
    
    int i;
    int j;
    pid_t pid;
    int pipe_array[2*(argc-1)]; // multiple pipes for child processes
    
    for(j=0;j<(2*(argc-1));j=j+2)
    {
    if (pipe(&pipe_array[j]) < 0)
        printf("ERROR\n");
    
    }
    
    for(i=0; i < argc-1;i++) // setting up the parallel processes
    {
        pid = fork();
        
        if (pid < 0)
        {
            printf("Error in making process");
        }
        if (pid == 0) // in child process
        {   
            close(pipe_array[2*i]);
            break;
        }
        if (pid > 0) // in parent process
        {
          close(pipe_array[(2*i)+1]);   
        }
        
    }
    
    if (pid == 0)
    {
        	
	int k = 0;
    
	int l = 0;
	//int m = 0;
	//int temp = 0;
	//int size[argc-1];
	int index;
	//int median[argc-1];
	FILE *fp;
	
	char buff[256];
	struct WordArray arr[10000]; //create WordArray arr initialized with size l
	struct Output out[argc-1];
	
		fp = fopen(argv[i+1], "r"); //open file i+1 because argv[0] = "a.out"
		
		if (fp==NULL){
			printf("File %s does not exist.\n", argv[i+1]); 
			return 0; 
        } 
        
        
        l = 0;
		
		
		fclose(fp); //close file
		
		fp = fopen(argv[i+1], "r"); // open file to point to begin of file
		
		index = 0;
		
		while(fscanf(fp, "%s", buff) == 1){ //read file and copy word to buff
			for(k=0;k<(index+1);k++){ //increment total count
				if(k == index){
					strcpy(arr[k].word, buff);
					l++;
					arr[k].freq = 1;
					index = index +1;
					break;
				}
				else if(strcmp(arr[k].word, buff) == 0){
					arr[k].freq++;
					break;
				}
				
				
			}
			
		}
		l = l+1;
		
		//printf("Total Words: %d\n", index);
		fclose(fp);

		qsort((void*)arr, index, sizeof(WordArray), sort);
		
		strcpy(out[i].med_word, arr[(l/2) -1].word);
		out[i].size = l - 1;
		strcpy(out[i].file, argv[i+1]);
        
        write(pipe_array[(2*i)+1],&out[i],sizeof(Output));
			
    }
    
    if (pid > 0)
    {
        struct Output out[argc-1];
        
        while ((waitpid(-1,NULL,0))!=-1);
       
         for(i=0; i < argc-1;i++)
         {
            read(pipe_array[(2*i)],&out[i],sizeof(Output));
             
         }
        
         qsort((void*)out, argc-1, sizeof(Output), compare_freq);
	
   
        for(i=0;i<argc-1;i++)
	       {
		      printf("%s %d %s\n", out[i].file, out[i].size, out[i].med_word);
	       }
 	 
    }

    return 0;
}



int compare_freq(const void *p, const void *q){
struct Output *freq1 = (struct Output *)p;
struct Output *freq2 = (struct Output *)q;
    return (freq2->size - freq1->size);
}

int sort(const void *a, const void *b){
	struct WordArray *sort1 = (struct WordArray *)a;
	struct WordArray *sort2 = (struct WordArray *)b;
	if(sort1->freq == sort2->freq)
		return strcmp(sort1->word, sort2->word);
	else
		return (sort2->freq - sort1->freq);
}

