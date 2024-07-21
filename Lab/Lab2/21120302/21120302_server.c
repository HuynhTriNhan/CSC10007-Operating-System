/* 1.Tạo các #include cần thiết */
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
/* dành riêng cho AF_INET */
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <math.h>

#define PORT 8080

// Cấu trúc dữ liệu cho Stack bằng Linked List
typedef struct Node
{
	int data;
	struct Node *next;
} Node;

typedef struct
{
	Node *top;
} Stack;

// Khai báo prototype các hàm
// Các hàm cơ bản của stack
void push(Stack *stack, int data);
int pop(Stack *stack);
int isEmpty(Stack stack);
// Xét độ ưu tiên của toán tử
int precedence(char op);
// Kiểm tra xem có phải là toán tử không
int isOperator(char c);
// Chuyển đổi từ biểu thức trung tố sang hậu tố
void ConvertInfixToPostfix(char *infix, char *postfix);
// Tính toán biểu thức hậu tố
int calculatePostfix(char *postfix);
// Kiểm tra format của biểu thức
bool checkformat(char *infix);

// Xử lý yêu cầu của client trả về kết quả phép tính
void handle_client(int sock)
{
	int read_size;
	char client_message[2000];
	// Nhận tin nhắn từ client và gửi lại
	while ((read_size = recv(sock, client_message, 2000, 0)) > 0)
	{
		int result = 0;
		char postfix[2000];
		char infix[2000];
		char wrongformat[] = "Wrong format";
		// In ra tin nhắn từ client
		printf("\nClient: %s\n", client_message);

		strcpy(infix, client_message);

		// kiêm tra lỗi format
		if (!checkformat(infix))
		{
			sprintf(client_message, "%s", wrongformat);
		}
		else
		{
			// Xử lý phép tính
			ConvertInfixToPostfix(infix, postfix);
			printf("Postfix Converted: %s\n", postfix);
			result = calculatePostfix(postfix);
			sprintf(client_message, "%d", result);
		}
		// In ra tin nhắn đã được gửi lại cho client
		printf("Server: %s\n", client_message);
		write(sock, client_message, strlen(client_message));
		// Làm sạch buffer trước mỗi lần đọc
		memset(client_message, 0, sizeof(client_message));
	}

	if (read_size == 0)
	{
		puts("Client disconnected");
		fflush(stdout);
	}
	else if (read_size == -1)
	{
		perror("recv failed");
	}
	close(sock);
	exit(0);
}

int main()
{
	int ret;
	int server_sockfd, client_sockfd;
	int server_len, client_len;
	struct sockaddr_in server_address;
	struct sockaddr_in client_address;

	/* 2. Thực hiện khởi tạo socket mới cho trình chủ */
	server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_sockfd < 0)
	{
		printf("Error in creating socket.\n");
		exit(1);
	}

	/* 3. Đặt tên và gán địa chỉ kết nối cho socket theo giao thức Internet */
	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = inet_addr("127.0.0.1");
	server_address.sin_port = htons(PORT);
	server_len = sizeof(server_address);

	/* 4. Ràng buộc tên với socket */
	ret = bind(server_sockfd, (struct sockaddr *)&server_address, server_len);
	if (ret < 0)
	{
		printf("Error in binding.\n");
		exit(1);
	}

	/* 5. Mở hàng đợi nhận kết nối - cho phép đặt hàng vào hàng đợi tối đa 5 kết nối */
	if (listen(server_sockfd, 5) == 0)
	{
		printf("Listening\n");
	}
	else
	{
		printf("Error in binding.\n");
		exit(1);
	}

	/* 6. Lặp vĩnh viễn để chờ và xử lý kết nối của trình khách */

	int cnt = 0;
	// Tạo ra một tiến trình con để xử lý kết nối của trình khách
	pid_t childpid;
	while (1)
	{
		printf("server waiting...\n");
		/* Chờ và chấp nhận kết nối */
		client_sockfd = accept(server_sockfd, (struct sockaddr *)&client_address, &client_len);
		if (client_sockfd < 0)
		{
			printf("Error in accepting.\n");
			exit(1);
		}
		// In ra thông tin kết nối đã được thiết lập
		printf("\nConnection accepted from %s:%d\n",
			   inet_ntoa(client_address.sin_addr),
			   ntohs(client_address.sin_port));

		// In ra số lượng kết nối đã được thiết lập
		printf("Clients connected: %d\n\n",
			   ++cnt);

		if ((childpid = fork()) == 0)
		{
			// Đóng socket chính của server
			close(server_sockfd);
			// Xử lý kết nối của trình khách
			handle_client(client_sockfd);
			exit(0);
		}
		/* Đóng kết nối */
		close(client_sockfd);
	}
}

void push(Stack *stack, int data)
{
	Node *newNode = (Node *)malloc(sizeof(Node));
	if (!newNode)
		return;
	newNode->data = data;
	newNode->next = stack->top;
	stack->top = newNode;
}

int pop(Stack *stack)
{
	if (!stack->top)
		return 0;
	Node *temp = stack->top;
	int popped = temp->data;
	stack->top = stack->top->next;
	free(temp);
	return popped;
}

int isEmpty(Stack stack)
{
	return stack.top == NULL;
}
// Xác định độ ưu tiên của toán tử
int precedence(char op)
{
	if (op == '+' || op == '-')
		return 1;
	if (op == '*' || op == '/')
		return 2;
	return 0;
}

int isOperator(char c)
{
	return (c == '+' || c == '-' || c == '*' || c == '/');
}

// Chuyeerr đổi từ biểu thức trung tố sang hậu tố
void ConvertInfixToPostfix(char *infix, char *postfix)
{
	// Khởi tạo stack, stack này lưu trữ các toán tử
	Stack stack;
	stack.top = NULL;
	// Biến k theo dõi mảng postfix
	int k = 0;

	for (int i = 0; infix[i]; i++)
	{
		// Nếu là số thì đưa ngay vào profix
		if (isdigit(infix[i]))
		{
			postfix[k++] = infix[i];
		}
		// Nếu là dấu mở ngoặc thì đưa vào stack
		else if (infix[i] == '(')
		{
			push(&stack, infix[i]);
		}
		// Nếu là dấu đóng ngoặc thì lấy các toán tử trong stack cho đến khi gặp dấu mở ngoặc
		else if (infix[i] == ')')
		{
			// Trong khi stack không rỗng và phần tử đầu tiên của stack khác dấu mở ngoặc
			// Thì tiến hành lấy các toán tử trong stack cho đến khi gặp dấu mở ngoặc
			while (!isEmpty(stack) && stack.top->data != '(')
			{
				postfix[k++] = pop(&stack);
			}
			pop(&stack); // pop '('
		}
		// Nếu là toán tử thì lấy các toán tử trong stack có độ ưu tiên lớn hơn hoặc bằng toán tử hiện tại
		else if (isOperator(infix[i]))
		{
			// Nếu stack không rỗng và phần tử đầu tiên của stack có độ ưu tiên lớn hơn hoặc bằng toán tử hiện tại
			// Thì tiến hành lấy các toán tử trong stack có độ ưu tiên lớn hơn hoặc bằng toán tử hiện tại ra để
			// đưa vào postfix
			while (!isEmpty(stack) && precedence(stack.top->data) >= precedence(infix[i]))
			{
				postfix[k++] = pop(&stack);
			}
			push(&stack, infix[i]);
		}
	}

	while (!isEmpty(stack))
	{
		postfix[k++] = pop(&stack);
	}

	postfix[k] = '\0';
}

int calculatePostfix(char *postfix)
{
	// Stack để lưu kết quả các phép tính
	Stack stack;
	stack.top = NULL;

	for (int i = 0; postfix[i]; i++)
	{
		if (isdigit(postfix[i]))
		{
			// Trừ cho ký tự 0 vì postfix[i] là mảng char => để lấy số nguyên
			push(&stack, postfix[i] - '0');
		}
		else
		{
			int val1 = pop(&stack);
			int val2 = pop(&stack);

			switch (postfix[i])
			{
			case '+':
				push(&stack, val2 + val1);
				break;
			case '-':
				push(&stack, val2 - val1);
				break;
			case '*':
				push(&stack, val2 * val1);
				break;
			case '/':
				push(&stack, val2 / val1);
				break;
			}
		}
	}
	return pop(&stack);
}

bool checkformat(char *infix)
{
	int cnt = 0;
	for (int i = 0; infix[i]; i++)
	{
		// Kiểm tra giữa 2 số có tồn tại toán tỷ không
		if (isdigit(infix[i]))
		{

			for (int j = i - 1; j >= 0; j--)
			{
				if (isOperator(infix[j]))
				{
					break;
				}
				else if (isdigit(infix[j]))
				{
					return false;
				}
			}
		}
		// Regex để check xem có ký tự lạ không
		if (!isdigit(infix[i]) && infix[i] != '(' && infix[i] != ')' && infix[i] != '+' && infix[i] != '-' && infix[i] != '*' && infix[i] != '/' && infix[i] != ' ')
			return false;

		if (infix[i] == '(')
			cnt++;
		else if (infix[i] == ')')
			cnt--;
		// Kiểm tra giữa 2 số có tồn tại toán tỷ không
		if (cnt < 0)
			return false;
	}

	if (cnt != 0)
		return false;
	return true;
}
