/* *
** Family Name: Siddiq	

** Given Name: Mohammed

** Student Number: 213054614

** CSE Login: siddiqmo

*/
#include <stdio.h>
#include <stdlib.h>
#include<string.h>
//#include<ctype.h>
#include "sch-helpers.h" 



int end_time = 0;   

int numberOfProcesses = 0; // total number of processes 
int util_time = 0;    //utilization time
int time = 0; 
int in = 0;
int cs = 0;
int q_time = 0;
		
process *cpu[NUMBER_OF_PROCESSORS];     //number of processors
process_queue ready; // ready queue
process_queue wait; // wait queue

	
process processes[MAX_PROCESSES+1];    // a large structure array to hold all processes read from data file 
process *temp[MAX_PROCESSES+1];   //temperory queue to sort before adding into ready/waiting queue
int temp_s = 0;  // size of temp queue

process *processes_expire[MAX_PROCESSES+1]; // need to store processes that haven't finished within time slice

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

int main(int argc, char *argv[]){

int i = 0;
int rem = 0;
int wait_total = 0;
int process_time = 0;
q_time = atoi(argv[1]);  // convert the time slice from string to interger 

    if (q_time == 0){
        error_bad_quantum();
    }
    
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


    printf("Average wait time:\t\t\t\t %.2f\n", wait_total / (double) numberOfProcesses);
    printf("Average turnaround time:\t\t\t %.2f\n", process_time / (double) numberOfProcesses);
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
return 1;
}
// compare by pid expilicitly
int compare_pid(const void *a, const void *b){ 

	process *first = *((process**) a);
	process *second = *((process**) b);

	if(first->pid < second->pid) // pidOne>pidtwo
		return -1;
	if(first->pid > second->pid ) // pidOne<pidtwo
		return 1;
	return 0;				//equal
	
}

// move process from waiting queue to temp queue
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
            W->quantumRemaining = q_time; //set up the time slice
		} else{

			enqueueProcess(&wait, W); 
		}

	}
}
// initialize the temp queue with time slice
void init_temp(void){ 

    while (numberOfProcesses) {
            //run_tim here
    		if (in < numberOfProcesses && processes[in].arrivalTime <= time){
    			temp[temp_s] =&processes[in]; // add the recent process in teh temp queue
                temp[temp_s]->quantumRemaining = q_time; //add the time slice value to the process
    			temp_s++;
    			in++;
    		}else{
    	break;
    		}
    	}
}

// after checking if the bursts and the time slice is finished so we can place those processes in the waiitng queue

void run_wait(void){ 
    int e = 0;
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
				
					} else if (cpu[i]->quantumRemaining == 0){
                    // We need to place the process somewhere
                    // since the process isn't finished but the time 
                    // slice is expired
                        processes_expire[e] = cpu[i]; //i wont work need new variable (e) to iterate over expire array
                       
                        processes_expire[e]->quantumRemaining = q_time; //update the remianing time slice time it needs to finsh
                        
                        cs++; // since we move on from the process there's a contextswitch
                        e++; //increment the expire array
                        cpu[i] = NULL; //free up cpu
                    }
                        
                }
                
			}
                        // need to add expire processes to teh waiting queue
                        // but after sorting them 
                        qsort(processes_expire,e,sizeof(process *),compare_pid);
                        i = 0;
                        while(i < e){
                            enqueueProcess(&ready,processes_expire[i]);
                            i++;
                        }
    
		} 

// cpu scheduler chooses the processs for the cpu
process *scheduler(void){
    
        // if ready queue is empty then return nothing
    	if (ready.size == 0){
    		return NULL;
    	} 
    		process *next = ready.front->data; // dequeue the front process
    		dequeueProcess(&ready);
    		return next; //return the next process
    	
    }


// increasing the step size of the cpus and decrement the time slice
void cpu_step(void){
		int i = 0;
		while( i < NUMBER_OF_PROCESSORS){
			if (cpu[i] !=  NULL){
				cpu[i]->bursts[cpu[i]->currentBurst].step++;
                cpu[i]->quantumRemaining--;
			}
			i++;
		}
	}

// increasing step size of the process in the wait queue
void io_step(void){
		int i = 0;
		while(i < wait.size){
			process *inc_wait = wait.front->data; //processahead of the wait queue
			dequeueProcess(&wait); 
			inc_wait->bursts[inc_wait->currentBurst].step++; //add step to top process and then add it back to the queue
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
		while ((status=readProcess(&processes[numberOfProcesses]))) {
         if(status==1)  numberOfProcesses ++;
        }
    }
    

	