#include <stdio.h>
#include "qstack.h"

void stack_push(Stack* S, void* val)
{
	//TODO: malloc(sizeof(void *)); ??
	S->value[S->top] = val; 
	(S->top)++;
}

void* stack_pop(Stack* S)
{
	//TODO: free(s->value);
	(S->top)--;
	return (S->value[S->top]);
}

void stack_init(Stack* S)
{
	S->top = 0;
}

int stack_full(Stack* S)
{
	//TODO: should be dynamic in the future
	return (S->top >= Q_STACK_MAX);
}

void stack_free(Stack* S) {
	//noop at the moment
}


//void dump_stack(Stack* S)
//{
//	int i;
//	if (S->top == 0)
//		printf("Stack is empty.\n");
//	else
//	{
//		printf("Stack contents: ");
//		for (i=0;i<S->top;i++)
//		{
//			printf("%p  ", S->value[i]); 
//		}
//		printf("\n");
//	}
//}