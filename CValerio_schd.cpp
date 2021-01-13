/*
 * CValerio_schd.cpp
 *
 * Created on: Nov 27, 2020
 *      Author: Christopher
 *
 *
 * Compile: g++ -o CValerio_schd.exe CValerio_schd.cpp
 *
 * Execute: CValerio_schd.exe Proj3InputFile.txt > rslts.txt
 */

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <stdlib.h>

using namespace std;


struct Process {
private:
	int process;
	int priority;
	int burst;
	int arrival;
	int waitTime;

public:
	Process(int proc, int pri, int bur, int arr, int wait) {
		process = proc;
		priority = pri;
		burst = bur;
		arrival = arr;
		waitTime = wait;
	}

	void toString() {
		cout << this->process << " " << this->priority << " " << this->burst << " " << this->arrival << "\n";
	}

	int getProc() {
		return process;
	}

	int getPri() {
		return priority;
	}

	int getBurst() {
		return burst;
	}

	int getArrival() {
		return arrival;
	}
	int getWait() {
		return waitTime;
	}

	void setProc(int proc) {
		process = proc;
	}

	void setPri(int pri) {
		priority = pri;
	}

	void setBurst(int bur) {
		burst = bur;
	}

	void setArrival(int arr) {
		arrival = arr;
	}

	void setWaitTime(int wait) {
		waitTime = wait;
	}

};

struct Node{
	//unsigned long int data;
	Process* data;
	struct Node *next;
};



Node* rr = new Node(); // linked List containing processes with the same priority level in the event of a Round Robin Tie Breaker
Node* allJobs = new Node(); // Linked List loaded from input file, with each processes information that still needs to be executed
ifstream myFile; // file stream use to read inputFile
int currTime = 0; // global integer variable representing the "systems" current time in execution
int numTotalJobs = 0;



void add(struct Node* head, Process* newData) {
	//cout << "addJob" << endl;
	//printf("Adding %d to sumList\n", newData);
	struct Node* new_node = new Node();
	new_node->data  = newData;
	new_node->next  = NULL;

	struct Node* curr = head;
	while(curr->next != NULL) {
		curr = curr->next;
	}
	curr->next = new_node;
}

void printList(struct Node* head) {
	long long i = 0;
	struct Node* curr = head->next;
	printf("Printing Jobs List\n");
	while(curr != NULL) {
		curr->data->toString();
		curr = curr->next;
	}
}

bool isEmpty(struct Node* head) {
	Node* curr = head->next;
	bool ret = true;
	while(curr != NULL) {
		if(curr->data->getPri() > 0)
			ret = false;

		curr = curr->next;
	}
	return /*(head->next == NULL)*/ret;
}

bool checkPreemption(Node* head, int currPriority) {
	// loops through the jobs list and checks if a new process has entered the queue with a higher priority
	// return true if one has indeed entered, otherwise return false
	// if we return true then the currently executing process should be preempted for one with a higher priority

	Node* currNode = head->next;
	while(currNode != NULL) {
		if(currNode->data->getArrival() <= currTime && currNode->data->getPri() < currPriority) {
			return true;
		}
		currNode = currNode->next;
	}

	return false;
}

void removeJob(Node* head, Node* procToRemove) {
	//cout << "removeJob" << endl;
	Node *prev = head;
	while(prev->next != NULL && prev->next != procToRemove)
		prev = prev->next;

	// Check if node really exists in Linked List
	if(prev->next == NULL)
	{
		cout << "\nGiven node is not present in Linked List";
		return;
	}

	// Remove node from Linked List
	prev->next = prev->next->next;
	return;
}

bool containsProc(Node* head, int proc) {
	// traverse lists and checks if indicated process number has yet been completed
	Node* currNode = head->next;
	while(currNode != NULL) {
		if(currNode->data->getProc() == proc) {
			return true;
		}
		currNode = currNode->next;
	}
	return false;
}

void addWaitTime(/*Node* head*/int* waitTimeArr, int addTime, int exceptThisProc) {
	// adds indicated  wait time to all processes waiting

	/*Node* currNode = head->next;
	while(currNode != NULL) {
		if(currNode->data->getProc() != exceptThisProc)
			currNode->data->setWaitTime(   (currNode->data->getWait() + addTime)   );

		currNode = currNode->next;

	}*/

	for(int i = 1; i < numTotalJobs + 1; i++) {
		if(i != exceptThisProc && (containsProc(allJobs, i) || containsProc(rr, i)) ) {
			waitTimeArr[i] += addTime;
		}
	}
}

void printWaitTimes(/*Node* head*/ int* waitTimeArr) {
	/*Node* currNode = head->next;
	cout << "Time\tProcesss" << endl;
	while(currNode != NULL) {

		//currNode->data->setWaitTime(   (currNode->data->getWait() + addTime)   );
		cout << currNode->data->getProc() << "\t" << currNode->data->getWait() << endl;
		currNode = currNode->next;

	}*/

	cout << "\n-- Process Wait-Times --" << endl;
	cout << "Time\tProcesss" << endl;
	for(int i = 1; i < numTotalJobs + 1; i++) {
		cout << i << "\t" << waitTimeArr[i] << endl;
	}
}

Node* getNextJob2(struct Node* head) { // finds and returns only 1 job with elligible priority level. Used within getNextJob()
	//cout << "getNextJob2" << endl;

	int highestPri = 1000000; // lowest number is highest priority

	Node* tempNext = NULL; // reference to job node that will be returned to be processed

	Node* currNode = head->next; // reference use to traverse jobs list
	while(currNode != NULL) { // traverse through whole list
		if(currNode->data->getArrival() <= currTime /*&& (currNode->data->getPri() != -1)*/) { // meaning job has actually arrived and is eligible for consideration
			if(currNode->data->getPri() < highestPri) {
				highestPri = currNode->data->getPri();
				tempNext = currNode;
			}
		}
		currNode = currNode->next;
	}
	//tempNext->data->setPri(-1); // instead of removing
	removeJob(head, tempNext);
	return tempNext;
}

Node* getNextJob(struct Node* head, bool preempting, int* waitTimeArr) { // returns NULL in the event no jobs is available next, otherwise returns process node to be executed next
	// can only consider jobs where (arrival time <= currTime)

	int highestPri = 1000000; // lowest number is highest priority

	Node* tempNext = NULL; // reference to job node that will be returned to be processed

	Node* currNode = head->next; // reference use to traverse jobs list
	while(currNode != NULL) { // traverse through whole list
		if(currNode->data->getArrival() <= currTime /*&& (currNode->data->getPri() != -1)*/) { // meaning job has actually arrived and is eligible for consideration
			if(currNode->data->getPri() < highestPri) {
				highestPri = currNode->data->getPri();
				//tempNext = currNode;
			}
		}
		currNode = currNode->next;
	}

	//cout << "highest Priority: " << highestPri << endl;

	currNode = head->next;
	int size = 0; // size of jobs list sharing same priority level
	while(currNode != NULL) {
		if(currNode->data->getPri() == highestPri && currNode->data->getArrival() <= currTime) {
			add(rr, currNode->data); // add to new round robin queue
			removeJob(head, currNode); // remove from overall jobs queue
			size++;
		}
		currNode = currNode->next;
	}

	if(size == 1) { // if no round robin necessary
		//cout << "NO RR" << endl;
		tempNext = rr->next;
		removeJob(rr, tempNext);
	}
	else if(size > 1){ // handle round robin scheduling
		//cout << "HANDLING RR" << endl;
		//cout << "Priority: " << highestPri << endl;
		//cout << "Size: " << size << endl;

		//printList(rr);

		//return NULL;
		cout << flush;
		while(!isEmpty(rr)) {

			Node* nextJob = getNextJob2(rr);
			//cout << "\nnextJob: " << nextJob->data->getProc() << endl;
			//cout << endl << endl;
			//printList(head);

			printf("%d \t\t%d\n", currTime, nextJob->data->getProc());
			//printList(head);
			cout << flush;
			int i = 0;
			int q = 0; // time quantum used for round robin
			for( ; i < nextJob->data->getBurst(); ) {

				if(q++ >= 10 && !isEmpty(rr)) { // the time quantum has exceeded and should preempt
					// add currently executing job back onto queue and fetch another job from queue to preempt with
					// and update wait times


					addWaitTime(waitTimeArr/*head*/, i, nextJob->data->getProc());

					Node* newJob = getNextJob2(rr);
					nextJob->data->setBurst((nextJob->data->getBurst()) - i);
					add(rr, nextJob->data);
					nextJob = newJob;
					printf("%d \t\t%d\n", currTime, nextJob->data->getProc());
					//printList(head);
					cout << flush;



					q = 0;
					i = 0;
				}
				else
					i++;


				//i++;
				currTime++;

				// after each "second" goes by check if we can should preempt
				// if we preempt during round robin we must add RR jobs back to main jobs queue
				// and fetch the next highest priority for preemption
				if(preempting == true && checkPreemption(head, nextJob->data->getProc())) {

					//cout << endl << "Should Now Preempt" << endl;
					nextJob->data->setBurst(nextJob->data->getBurst() - i); // update burst time for process about to be preempted

					// add all jobs in Round Robin to allJobs and remove from RR List
					Node* currNod = rr->next;
					while(!isEmpty(rr) && currNod != NULL) {
						/*currNod->data->setBurst(currNod->data->getBurst() - i);*/
						add(head, currNod->data);
						//Node* tempRef = currNod->next;
						removeJob(rr, currNod);
						//currNod = tempRef;
						currNod = currNod->next;
					}

					// update jobs burst time and add back onto job queue for later processing
					// retrieve nextjob

					nextJob->data->setBurst(nextJob->data->getBurst() - i);
					add(head, nextJob->data);

					Node* nextJob = getNextJob(head, preempting, waitTimeArr);
					if(nextJob == NULL) {
						printWaitTimes(/*allJobs*/ waitTimeArr);
						/*return*/exit(1);
					}


					i = nextJob->data->getBurst(); // set i to break inner for loop and get new job

				}

			}
			//update wait times here
			addWaitTime(waitTimeArr/*head*/, i, nextJob->data->getProc());

			//cout << "looping" << endl;
			//cout << flush;
		}

		return getNextJob(head, preempting, waitTimeArr);
	}

	return tempNext;
}


void loadJobsFromFile(char* fileName) {

	ifstream myFile;
	myFile.open(fileName, fstream::in);

	if(!myFile) {
		cerr << "unable to open input file" << endl;
		exit(1);
	}
	numTotalJobs = 0;
	string x;

	myFile >> x; myFile >> x; myFile >> x; myFile >> x; // Skips header information from input file

	string process;
	int proc;
	string priority;
	int pri;
	string burst;
	int burr;
	string arrival;
	int arr;

	int cnt = 1;

	//Node* allJobs = new Node();

	while(myFile >> x) {

		if(cnt == 1) {
			//process = x;
			stringstream toInt(x);
			toInt >> proc;
		}
		else if(cnt == 2) {
			//priority = x;
			stringstream toInt(x);
			toInt >> pri;
		}
		else if(cnt == 3) {
			//burst = x;
			stringstream toInt(x);
			toInt >> burr;
		}
		else if(cnt == 4) {
			//arrival = x;
			stringstream toInt(x);
			toInt >> arr;
		}

		cnt++;
		if(cnt > 4) {
			//cout << (process + " " + priority + " " + burst + " " + arrival + "\n");
			Process* job = new Process(proc, pri, burr, arr, 0);
			add(allJobs, job);
			cnt = 1;
			numTotalJobs++;
		}
		//cout << x << endl;
	}
	myFile.close();

}





void run_part1(struct Node* head, int* waitTimeArr) { // NON-PREEMPTIVE priority scheduling with round-robin scheduling using time quantum q=10 as a tie breaker
	//cout << "run_part1" << endl;
	// will grab highest priority job and wont preempt unless there is a tie in priority levels

	//When a process arrives at the ready queue, its priority is compared with the priority of the currently running process
	//A preemptive priority scheduling algorithm will preempt the CPU if the priority of the newly arrived process is higher than the
	//priority of the currently running process. A nonpreemptive priority scheduling algorithm will simply put the new process at the head of the ready queue.

	currTime = 0; // current system time
	// can only consider jobs where (arrival time <= currTime)


	cout << "\n-- Part 1 Output --" << endl;
	cout << "Time\tProcesss" << endl;
	while(!isEmpty(head)) {

		Node* nextJob = getNextJob(head, false, waitTimeArr);
		if(nextJob == NULL)
			return;
		//cout << "\nnextJob: " << nextJob->data->getProc() << endl;

		//cout << endl << endl;
		//printList(head);

		printf("%d \t\t%d\n", currTime, nextJob->data->getProc());
		cout << flush;
		int i = 0;
		for( ; i < nextJob->data->getBurst(); ) {

			i++;
			currTime++;

		}
		// add 'i' time to each processes wait time
		addWaitTime(waitTimeArr/*head*/, i, nextJob->data->getProc());
	}
}

void run_part2(struct Node* head, int* waitTimeArr) { // PREEMPTIVE priority scheduling with round-robin scheduling using time quantum q=10 as a tie breaker
	//cout << "run_part1" << endl;
	// will grab highest priority job and wont preempt unless there is a tie in priority levels

	//When a process arrives at the ready queue, its priority is compared with the priority of the currently running process
	//A preemptive priority scheduling algorithm will preempt the CPU if the priority of the newly arrived process is higher than the
	//priority of the currently running process. A nonpreemptive priority scheduling algorithm will simply put the new process at the head of the ready queue.

	currTime = 0; // current system time
	// can only consider jobs where (arrival time <= currTime)


	cout << "\n-- Part 2 Output --" << endl;
	cout << "Time\tProcesss" << endl;
	while(!isEmpty(head)) {

		Node* nextJob = getNextJob(head, true, waitTimeArr);
		if(nextJob == NULL) {
			return;
		}

		//cout << "\nnextJob: " << nextJob->data->getProc() << endl;

		//cout << endl << endl;
		//printList(head);

		printf("%d \t\t%d\n", currTime, nextJob->data->getProc());
		cout << flush;
		int i = 0;
		for( ; i < nextJob->data->getBurst(); ) {

			i++;
			currTime++;
			//cout << endl << "Should Preempt: " << checkPreemption(head, nextJob->data->getProc()) << endl;

			// after each "second" goes by check if we can/should preempt
			if(checkPreemption(head, nextJob->data->getProc())) {

				//cout << endl << "Should Now Preempt" << endl;

				// if we need to preempt update jobs burst time and add back onto job queue for later processing
				// retrieve nextjob and update wait times

				addWaitTime(waitTimeArr/*head*/, i, nextJob->data->getProc());

				nextJob->data->setBurst(nextJob->data->getBurst() - i);
				add(head, nextJob->data);

				/*Node* nextJob = getNextJob(head);
				if(nextJob == NULL)
					return;*/

				i = nextJob->data->getBurst() + 1;

			}

		}
		// add 'i' time to each processes wait time
		addWaitTime(waitTimeArr/*head*/, i, nextJob->data->getProc());

	}

}

int main(int argc, char** argv) {

	//cout << argv[0] << " " << argv[1];

	loadJobsFromFile(argv[1]);


	//cout << "Total number of Jobs: " << numTotalJobs << endl;
	int* waitTimes = new int[numTotalJobs + 1];
	for(int i = 0; i < numTotalJobs + 1; i++) {
		waitTimes[i] = 0;
	}


	//printList(allJobs);

	cout << "\nRunning Part1: Non-Preemptive Priority-Based Scheduling Algorithm With Round-Robin Scheduling" << endl;
	run_part1(allJobs, waitTimes);
	//myFile.close();

	printWaitTimes(/*allJobs*/ waitTimes);

	for(int i = 0; i < numTotalJobs + 1; i++) {
		waitTimes[i] = 0;
	}


	loadJobsFromFile(argv[1]);



	cout << "\nRunning Part2: Preemptive Priority-Based Scheduling Algorithm With Round-Robin Scheduling" << endl;
	run_part2(allJobs, waitTimes);
	printWaitTimes(/*allJobs*/ waitTimes);

	return 0;
}
