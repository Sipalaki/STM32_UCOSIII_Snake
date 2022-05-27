#include<stdio.h>
#include<stdlib.h>
struct Queue{ 
 int *data;   
 int capacity; 
 int front;  
 int rear;  
};

void init(struct Queue *pq,int capacity);
int isFull(const struct Queue *pq);
int isEmpty(const struct Queue *pq);
int enQueue(struct Queue *pq,int x);
int deQueue(struct Queue *pq);
void clearQueue(struct Queue*xq,struct Queue*yq);
int searchQueue(int x,int y,struct Queue *xq,struct Queue *yq);
