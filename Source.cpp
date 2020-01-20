#include <iostream>
#include <pthread.h>
#include <stdio.h>						
#include <unistd.h>						
#include <string>
#include <stdlib.h>
#include <queue>
#include <cstdlib>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>


//Rabia Aydemir 23492//

#define NUM_THREADS 10
#define MEMORY_SIZE 20

#define randnum(min, max) \
	((rand() % (int)(((max) + 1) - (min))) + (min))

char  memory[MEMORY_SIZE] ;
bool checkterminate = false;
bool allocate[NUM_THREADS]; 
pthread_t server_thread;

using namespace std;

struct Node
{
	int id;
	int size;
	int index;
	Node* next;
	Node(int idnum, int sizenum, int indexnum) : id(idnum), size(sizenum), index(indexnum), next(NULL) {}
	Node() {};
};


struct req
{
	int id;
	int size;
	req(int i, int s) : id(i), size(s)
	{};
};


	Node* head;


Node* Head()  //headi kaybetme :)
{
	return head;
}

void characid(Node*ptr,int id,int reqsize)
{
	for (int i = ptr->index; i <ptr->index + reqsize; i++)
			{
				char c = '0' + id;
				memory[i] = c;
			}
}


int allocated(int id, int reqsize)
{
	Node *ptr = head;
	while (ptr != NULL)
	{
		if (  ptr->size > reqsize && ptr->id == -1)
		{
			Node* temp = new Node(-1, ptr->size - reqsize, ptr->index + reqsize);
			ptr->id = id;
			ptr->size = reqsize;

			if (ptr->next != NULL)
			{
				temp->next = ptr->next;
				ptr->next = temp;
			}

			else
			ptr->next = temp;
			characid(ptr,id,reqsize);
			return 1;
		}
		else if (ptr->size == reqsize && ptr->id == -1  )
		{
			ptr->id = id;
			ptr->size = reqsize;
			characid(ptr,id,reqsize);
			return 1;
		}
	

		ptr = ptr->next;
	}
	return 0;
}


queue<req> myqueue;
pthread_mutex_t sharedLock = PTHREAD_MUTEX_INITIALIZER;
sem_t semlist[NUM_THREADS]; 




void use_mem()   //thread will sleep in this function 1-5
{
	int t =randnum(1,5);           
	sleep(t);
	
}

void tempid(int i,Node*temp)
{
	for (i; i < temp->size; i++)
	{
	memory[i] = 'X';
	}

}

void tempid2(int k,Node*temp)
{
for (k; k < temp->size + temp->index; k++)
	{
				
	memory[k] = 'X';
	}
}

void free_mem(int &id)
{
	Node* temp =Head();
	Node*ptr = temp;
	while (temp != NULL && temp->id != id)
	{
		ptr = temp;
		temp = temp->next;
	}
	
	if (temp->next == NULL) 
	{
		if (ptr->id == -1)
		{
			ptr->next = NULL;
			ptr->size += temp->size;
			delete temp;
			temp = NULL;
			int i = ptr->index;
		
		}
		else
		{
			temp->id = -1;
			int i = temp->index;
			int size=temp->size;
			
			tempid(i,temp);
		}
	}
	
	
	else if (temp->next != NULL && ptr != temp)
	{
		Node* ktr = temp->next;
		if (ptr->id == -1 && ktr->id == -1)
		{
			ptr->size = ptr->size + temp->size + ktr->size;
			ptr->id = -1;
			ptr->next = ktr->next;
			Node*s= temp;
			delete s;
			delete ktr;
			int i = ptr->index;

			
		}

		else if (ptr->id != -1 && ktr->id != -1)
		{
			int i = temp->index;
			tempid2( i,temp);
			temp->id = -1;
		}
		else if (ptr->id != -1 && ktr->id == -1)
		{
			temp->size = temp->size + ktr->size;
			temp->id = -1;
			temp->next = ktr->next;
			delete ktr;
			
			
		
		int i = temp->index;
			for (i; i < temp->size + temp->index ; i++)
			{
				
				memory[i] = 'X';
			}

		}

		else if (ktr->id != -1 && ptr->id == -1)
		{
			ptr->next = temp->next;
			ptr->size += temp->size;
			ptr->id = -1;
			delete temp;
		
			int i = ptr->index;
			for (i; i < ptr->index + ptr->size; i++)
			{
				
				memory[i] = 'X';
			}
		}
	}

else if (temp == Head() && temp->next == NULL)
	{
		temp->id = -1;
		temp->size = MEMORY_SIZE;
		temp->index = 0;
		
		for (int i=0 ; i < MEMORY_SIZE; i++)
		{
			
			memory[i] = 'X';
		}
	}

	else if (temp ==Head())//nodes is at the beginning
	{
		Node* ktrm = temp->next;
		if (ktrm->id != -1)
		{
			temp->id = -1;
			int t = temp->index;
			 tempid(t,temp);
		}
		else
		{
			temp->size += ktrm->size;
			temp->id = -1;
			temp->next = ktrm->next;
			delete ktrm;
			int s= temp->index;

			tempid(s,temp);
	
		}
	}

	else
		return;



}

void release_function()
{
	checkterminate = true;
	Node*ptr = Head();
	Node* a = ptr;
	while (ptr != NULL)
	{
		a= ptr->next;
		delete ptr;
		ptr = a;
		
	}
	
	ptr= NULL;
	
	for (int i = 0; i < MEMORY_SIZE; i++)
	{
		memory[i] = 'X';
	}

}
void dump_memory()                      //PRINTING THE LIST
{
	
	Node* p = Head();
	cout << "List: " << endl;
	while (p != NULL)
	{
		cout << "[" << p->id << "][" << p->size << "][" << p->index << "]";
		if (p->next != NULL)
			cout << "---";

		p = p->next;
	}
	cout << endl;
	cout << "Memory Dump: "<<endl;
	for (int i = 0; i < MEMORY_SIZE; i++)
	{
		cout << memory[i];
	}
	cout <<endl<< " ********************************* " <<endl;
}
void my_malloc(int thread_id, int size)
{

	//This function will add the struct to the queue
	pthread_mutex_lock(&sharedLock);
	req temp(thread_id ,size);
	myqueue.push(temp);
	pthread_mutex_unlock(&sharedLock); //unlock

}
void* thread_function(void * id)
{
	
	while (!checkterminate) {
		int* idthread = (int*)id;
		int size = rand() % (MEMORY_SIZE / 3) + 1;
		my_malloc(*idthread, size);
		sem_wait(&(semlist[*idthread]));// down

		if (allocate[*idthread] == true)
		{
			use_mem();
			pthread_mutex_lock(&sharedLock);
			free_mem(*idthread);
			pthread_mutex_unlock(&sharedLock);
		}
	}
}


void * server_function(void *)
{
	
	while (!checkterminate)
	{
		if (!myqueue.empty()&& !checkterminate)
		{
			pthread_mutex_lock(&sharedLock); 
		
			req temp = myqueue.front();
			myqueue.pop();
			int reqsize = temp.size;
		
			if (/*list.*/allocated(temp.id, reqsize))
			{
				dump_memory();
				allocate[temp.id] = true;
			}
			else
				allocate[temp.id] = false;

			sem_post(&(semlist[temp.id])); 
			pthread_mutex_unlock(&sharedLock);  			
		}
	}
}

void init()
{
	pthread_mutex_lock(&sharedLock);
	for (int i = 0; i < NUM_THREADS; i++)
	{
		sem_init(&semlist[i], 0, 0);
	}
	pthread_create(&server_thread, NULL, server_function, NULL); 
	pthread_mutex_unlock(&sharedLock);
}


int main(int argc, char*argv[])
{
	head =  new Node(-1, MEMORY_SIZE, 0);
	for (int i = 0; i < MEMORY_SIZE; i++)
	{
		memory[i] = 'X';
	}
	for (int i = 0; i < NUM_THREADS; i++)
	{
		allocate[i] = false;
	}
	
	int  threadid[NUM_THREADS];
	for (int i = 0; i<NUM_THREADS; i++)
	{
		threadid[i] = i;
	}
	
	init();
	
	pthread_t idth[NUM_THREADS];
	for (int i = 0; i<NUM_THREADS; i++)
	{
		pthread_create(&idth[i], NULL, &thread_function, (void*)&threadid[i]);  ////creating threads
	}

	sleep(10);          //sleeping the  main thread
	checkterminate = true;
	
	pthread_join(server_thread, 0);
	for (int i = 0; i<NUM_THREADS; i++)          
	{
		pthread_join(idth[i], 0);
	}

	release_function();                 
	printf("\nTerminating...\n");
	
	return 0;
}