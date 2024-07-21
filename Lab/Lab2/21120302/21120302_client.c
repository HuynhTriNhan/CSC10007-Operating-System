/* 1. Tạo các #include cần thiết để gọi hàm socket */
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
/* dành riêng cho AF_INET */
#include <netinet/in.h>
#include <arpa/inet.h>

#include <string.h>
#include <stdlib.h>

#define PORT 8080
int main()
{
	int sockfd; /* số mô tả socket – socket handle */
	int len;
	struct sockaddr_in address; /* structure sockaddr_in, chứa các thông tin về socket AF_INET */
	int result;
	char message[1000], server_reply[2000];

	/* 2. Tạo socket cho trình khách. Lưu lại số mô tả socket */
	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	/* 3. Đặt tên và gán địa chỉ kết nối cho socket theo giao thức Internet */
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = inet_addr("127.0.0.1");
	address.sin_port = htons(PORT);
	len = sizeof(address);

	/* 4. Thực hiện kết nối */
	result = connect(sockfd, (struct sockaddr *)&address, len);
	if (result == -1)
	{
		perror("Oops: client1 problem");
		return 1;
	}
	else
	{
		printf("Connected to server with PORT = %d\n", PORT);
	}

	/* 5. Sau khi socket kết nối, chúng ta có thể đọc ghi dữ liệu của socket tương tự đọc ghi trên file */

	// Giao tiếp với server
	while (1)
	{
		// Nhập dữ liệu
		char c;
		int i = 0;
		printf("\nEnter the expression: ");
		while ((c = getchar()) != '\n')
		{
			message[i++] = c;
		}
		message[i] = '\0';

		// Gửi dữ liệu
		int bytes_sent = send(sockfd, message, strlen(message), 0);
		if (bytes_sent < 0)
		{
			printf("Send failed");
			return 1;
		}
		else if (bytes_sent < strlen(message))
		{
			printf("Not all bytes sent");
		}

		// Nhận phản hồi từ server
		memset(server_reply, 0, sizeof(server_reply)); // clear the buffer
		int bytes_received = recv(sockfd, server_reply, sizeof(server_reply) - 1, 0);
		if (bytes_received < 0)
		{
			printf("Receive failed\n");
			break;
		}
		else if (bytes_received == 0)
		{
			printf("Connection closed\n");
			break;
		}

		// In ra phản hồi từ server
		printf("Server: %s\n", server_reply);
	}

	close(sockfd);
	return 0;
}