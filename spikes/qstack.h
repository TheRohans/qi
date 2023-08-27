
#define Q_STACK_MAX 100

typedef struct
{
	void* value[Q_STACK_MAX];
	int top;
} Stack;

void stack_push(Stack *S, void* val);
void* stack_pop(Stack *S);
void stack_init(Stack *S);
int stack_full(Stack *S);
void stack_free(Stack *S);
