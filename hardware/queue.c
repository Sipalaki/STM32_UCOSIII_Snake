#include"queue.h"

void init(struct Queue *pq,int capacity){ //?????? 
 pq->capacity=capacity;
 pq->data=(int*)malloc(sizeof(int)*(capacity+1));
 pq->front = 0;  //??????,?????????????????????? 
 pq->rear = 0;
}

int isFull(const struct Queue *pq){  //????????
 if((pq->rear + 1)%(pq->capacity+1) == pq->front) return 1;
 else return 0;
}

int isEmpty(const struct Queue *pq){ //?????? 
 return pq->front == pq->rear;
}

int enQueue(struct Queue *pq,int x){ //???? 
 if(isFull(pq)) return 0;
 else{
  pq->data[pq->rear] = x;
  pq->rear = (pq->rear+1) % (pq->capacity+1);
  return 1;  //????1,????0 
 } 
}

int deQueue(struct Queue *pq){  //???? 
 if(isEmpty(pq)) return 0;
 else {
  pq->front = (pq->front+1) % (pq->capacity+1);
  return 1;  //????1,????0 
 }
}

int searchQueue(int x,int y,struct Queue *xq,struct Queue *yq){
	int p = xq->front;
	int q = xq->rear;
	if(q-1<0) q=16;else{q = q-1;}
	if(isEmpty(xq)) return 0;
	else{
		while(p!=q){
			if(xq->data[p]==x && yq->data[p]==y){
				return 1;
		}else{
				p = (p+1) % (xq->capacity+1);
		}
	}
	}
	return 0;
}

void clearQueue(struct Queue*xq,struct Queue*yq){
xq->front = 0;
xq->rear = 0;

yq->front = 0;
yq->rear = 0;
}
