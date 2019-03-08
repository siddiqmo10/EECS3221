/* 
** Family Name: Siddiq	

** Given Name: Mohammed

** Student Number: 213054614

** CSE Login: siddiqmo


/**
 * Description: Some helper functions to be used for CSE3221 "scheduler" project
 */

/* Some hints to use these helper functions in this project */
/*  
   Step 1:  You should include the header file in your main scheduler code:

#include "sch-helpers.h"   

   Step 2: you should declare a global array in your own fcfs.c to hold info for all processes:

process processes[MAX_PROCESSES+1];   // a large structure array to hold all processes read from data file 
int numberOfProcesses=0;              // total number of processes 

   Step 3: you can call function readProcess() to read all data from stdio and sort the processes array ascending by arrival time:

... 
while (status=readProcess(&processes[numberOfProcesses]))  {
         if(status==1)  numberOfProcesses ++;
}   // it reads pid, arrival_time, bursts to one element of the above struct array
...
qsort(processes, numberOfProcesses, sizeof(process), compareByArrival);
qsort(processes, numberOfProcesses, sizeof(process), compareByArrival);
...

  Step 4: You may consider to use the following linked-list based Queue management functions to impelement all scheduling queues (Ready Q and Device Q) for your scheduler:

process_node *createProcessNode(process *);
void initializeProcessQueue(process_queue *);
void enqueueProcess(process_queue *, process *);
void dequeueProcess(process_queue *);

  Step 5: After you are done, you can submit your fcfs.c as well as sch-helpers.h sch-helpers.c to the system. 
          Your code should compile as:

         $$ gcc -o fcfs fcfs.c sch-helpers.c

       In this case you can redirect all CPU load data to stdio when you run your FCFS schedule:

          $$ fcfs < CPULoad.dat    OR     $$ cat CPULoad.dat | fcfs 
*/

//including libraries
#include <stdio.h>
#include <stdlib.h>
#include<string.h>
#include<ctype.h>
#include "sch-helpers.h" //helper file included



int end_time = 0;   

int numberOfProcesses = 0; // total number of processes 
int util_time = 0;    //utilization time
int time = 0; 
int in = 0;
int cs = 0;
		
process *cpu[NUMBER_OF_PROCESSORS];     //number of processors
process_queue ready; // ready queue
process_queue wait; // wait queue

	
process processes[MAX_PROCESSES+1];    // a large structure array to hold all processes read from data file 
process *temp[MAX_PROCESSES+1];   //temperory queue to sort before adding into ready/waiting queue
int temp_s = 0;  // size of temp queue

/*declared functions */
	void wait_temp(void);
	void init_temp(void);
	void run_wait(void);
	void ready_running(void);
	void update_ready(void);
	void io_step(void);
	void cpu_step(void);
	void read(void);
	int compare_pid(const void *a, const void *b);
    process *scheduler(void);
    int run_p(void);

int main(void){

int i = 0;
int rem = 0;
int wait_total = 0;
int process_time = 0;

initializeProcessQueue(&ready);
initializeProcessQueue(&wait); 

    read();
    error_input(numberOfProcesses);
    qsort(processes, numberOfProcesses, sizeof (process), compareByArrival);


while(1){

    //printf("In the while loop\n");
	wait_temp();
	init_temp();
	run_wait();
	ready_running();
	update_ready();
	io_step();
	cpu_step();

    util_time += run_p();
	rem -= in;	

    	// if theres no more proccesses running and the wait queue is empty
        // leave the while loop
	if(run_p() == 0 && wait.size == 0)
		break; 
	
	time++;

}

  //printf("Out the while loop\n");


while (i< numberOfProcesses){
process_time += (processes[i].endTime - processes[i].arrivalTime); 
wait_total += processes[i].waitingTime; 
i++;
}


    printf("Average wait time:\t\t\t\t %.2f\n", wait_total / (float) numberOfProcesses);
    printf("Average turnaround time:\t\t\t %.3f\n", process_time / (float) numberOfProcesses);
    printf("Time all processes finished:\t\t\t %d\n", time);
	
    printf("Average CPU utilization:\t\t\t %.2f\n",  (util_time * 100.0) /  (time));
    printf("Context switches occured:\t\t\t %d\n", cs); 
    printf("PID of last process terminated:\t\t\t");
	
	i = 0;
	while (i< numberOfProcesses){
        if (processes[i].endTime == time) {
            printf(" %d\n", processes[i].pid);
        }
		i++;
    }

}
// compare by pid expilicitly
int compare_pid(const void *a, const void *b){ 

	process *one = *((process**) a);
	process *two = *((process**) b);

	if(one->pid < two->pid) // pidOne>pidtwo
		return -1;
	if(one->pid > two->pid ) // pidOne<pidtwo
		return 1;
	return 0;				//equal
	
}

// waiting queue to temp queue
void wait_temp(void){ 
    
    int i;
    int W_size = wait.size;

	for(i = 0; i < W_size; i++){

		process *W = wait.front->data; // get the data from the waiting queue
		dequeueProcess(&wait); // take out from the waiting queue
		
        // check if the io bursts are done so we can move from
        // the waiting queue to the temp queue else we put it back in
        // in waiting queue because the io bursts isnt done
        
		if((W->bursts[W->currentBurst].step) == (W->bursts[W->currentBurst].length)){ 
			W->currentBurst++;
			temp[temp_s] = W; 
			temp_s++;
		} else{

			enqueueProcess(&wait, W); 
		}

	}
}
// initialize the temp queue
void init_temp(void){ 

    while (numberOfProcesses) {
            //run_tim here
    		if (in < numberOfProcesses && processes[in].arrivalTime <= time){
    			temp[temp_s] =&processes[in];
    			temp_s++;
    			in++;
    		}else{
    	break;
    		}
    	}
}

// after checking if the bursts is finished so we can place those
// processes in the waiitng queue

void run_wait(void){ 

    int i = 0;
		for (i = 0;i<NUMBER_OF_PROCESSORS;i++){
			
			// running cpu
			if(cpu[i] != NULL){
				if ((cpu[i]->bursts[cpu[i]->currentBurst].step) == (cpu[i]->bursts[cpu[i]->currentBurst].length)){
					cpu[i]->currentBurst++; // move on to next burst as its done
					
					if((cpu[i]->numberOfBursts) > (cpu[i]->currentBurst)){
						enqueueProcess(&wait,cpu[i]);// since bursts isnt over we add it to the waiting queue
					}
					else {
						
                        cpu[i]->endTime = time;
						end_time = cpu[i]->endTime;
					}
				cpu[i] = NULL; //free up cpu
				
					}
			}
		} 
}

// cpu scheduler chooses the processs for the cpu
process *scheduler(void){
    		
    	if (ready.size == 0){
    		return NULL;
    	} 
    		process *next = ready.front->data;
    		dequeueProcess(&ready);
    		return next;
    	
    }


// increasing the step size of the cpus
void cpu_step(void){
		int i = 0;
		while( i < NUMBER_OF_PROCESSORS){
			if (cpu[i] !=  NULL){
				cpu[i]->bursts[cpu[i]->currentBurst].step++;
			}
			i++;
		}
	}

// increasing step size of the process in the wait queue
void io_step(void){
		int i = 0;
		while(i < wait.size){
			process *inc_wait = wait.front->data;
			dequeueProcess(&wait);
			inc_wait->bursts[inc_wait->currentBurst].step++;
			enqueueProcess(&wait,inc_wait);
            i++;
		}
		
	}

// setting the scheduler to run after the ready queue is added from the temp queue
void ready_running(void){ //get ready processes

    int i = 0;
    	int j = 0;
    	//sort the temp queue by pid
    	qsort(temp,temp_s,sizeof(process*),compare_pid);
    	
    	//add from the sorted temp to the ready queue
    	while( i < temp_s) {
    		enqueueProcess(&ready,temp[i]);
    		i++;
    	}
    	temp_s =0; //reset temp
    	// setting cpu to scheduler
    	while(j<NUMBER_OF_PROCESSORS) {
    		if(cpu[j] == NULL){
    			cpu[j] = scheduler();
    		}
    		j++;
    	}

}

	

// how many cpu are running normally
int run_p(void){
		int run = 0;
		int i = 0;
		
		while( i < NUMBER_OF_PROCESSORS){
			if (cpu[i] != NULL){
				run++;
			}
			i++;
		}
		return run;
	}

    
// update the ready process
void update_ready(void){
    	int i = 0;
    	while ( i < ready.size ) {
    		process *next1 = ready.front->data;
    		dequeueProcess(&ready);
    		next1->waitingTime++;
    		enqueueProcess(&ready,next1);
    		i++;
    	}
    }

// it reads pid, arrival_time, bursts to one element of the above struct array
void read(void){
	int status =0;
		while (status=readProcess(&processes[numberOfProcesses]))  {
         if(status==1)  numberOfProcesses ++;
        }
    }
    

	