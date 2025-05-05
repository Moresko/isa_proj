/*  ISA-project 
    tftp-client and server

    author: Martin Mores
*/

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<string.h>
#include<signal.h>
#include <arpa/inet.h>

// funkcia na spracovanie informácií od klienta
void convert_info_from_client(const char *buff, char *hostIp, char *filePath, char *dest_fileP)
{
    sscanf(buff, "%s %s %s", hostIp, filePath, dest_fileP);

}

void convert(char *sym, int digit)
{
    if (digit == 0) {
        sym[0] = '0';
        sym[1] = '0';
        sym[2] = '\0';
    } else if (digit < 10) {
        sym[0] = '0';
        sym[1] = '0' + digit;
        sym[2] = '\0';
    } else if (digit < 100) {
        sym[0] = '0' + digit / 10;
        sym[1] = '0' + digit % 10;
        sym[2] = '\0';
    } else {
        sym[0] = '9';
        sym[1] = '9';
        sym[2] = '\0';
    }

}

//funckia pomocou ktorej program zisti ze sa jedna o ACK operáciu
char* ACK(char* block_number)
{
	size_t packet_size = 2 + strlen(block_number) + 1;
    char *packet = calloc(packet_size, sizeof(char));
	strcat(packet, "04");
	strcat(packet, block_number);

    return packet;
}

//funckia pomocou ktorej program zisti ze sa jedna o ERROR operáciu
char *ERROR(char *message)
{
	size_t packet_size = 4 + strlen(message) + 1;
	char *packet = calloc(packet_size, sizeof(char));
	strcat(packet, "05");
	strcat(packet, message); 

	return packet;
}

//funckia pomocou ktorej program zisti ze sa jedna o DATA operáciu
char* DATA(int block_number, char *data)
{
    char temp[3];
    convert(temp, block_number);
    size_t packet_size = 4 + strlen(temp) + strlen(data) + 1; 
    char *packet = calloc(packet_size, sizeof(char));
    strcat(packet, "03");
    strcat(packet, temp);
    strcat(packet, data);

    return packet;
}

int main(int argc, char *argv[]){
	int server_sock;
	int number;
    int port = 69;
	char buffer[1024];
	char hostIp[16];
	char filePath[256];
	char dest_filePath[256];
	char needed_buffer[1024];
	char needed_buffer_two[1024];
	char abs_path[256];
    char *root_Path = NULL;
	struct sockaddr_storage their_addr;
	socklen_t addr_len;
	
	//spracovanie argumentov, musia byť presne 4 argumenty
	if (argc != 4 ){
        perror("ERROR: Arguments\n");
        exit(EXIT_FAILURE);
    }

	port = atoi(argv[2]);
    if(port < 0 || port > 65535){
        perror("ERROR: Port\n");
        exit(EXIT_FAILURE);
    }

	root_Path = argv[3];

	//koniec spracovanie argumentov

	//začiatok nastavovanie UDP serveru pomocou soketu a server addresy, kde sa najskor nastavi soket a potom bind
	if((server_sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
        perror("ERROR: Server socket\n");
        exit(EXIT_FAILURE);
    }

	printf("Server done with socket\n");
	
	struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
	// inet_pton(AF_INET, hostIp, &server_address.sin_addr);
    server_address.sin_port = htons(port);

	addr_len = sizeof their_addr;

	if(bind(server_sock, (struct sockaddr*)&server_address, sizeof(server_address)) < 0)
	{
       perror("ERROR: Bind\n");
 	   exit(EXIT_FAILURE);
    }
	printf("Server done with bind\n");
	//koniec nastavovania serveru

	//potrebné informácie od clienta pre server
	ssize_t information_from_client = recvfrom(server_sock, needed_buffer, sizeof(needed_buffer), 0, (struct sockaddr *)&their_addr, &addr_len);
    needed_buffer[information_from_client] = '\0'; 
	convert_info_from_client(needed_buffer, hostIp, filePath, dest_filePath);

	snprintf(needed_buffer_two, sizeof(needed_buffer_two), "%s", root_Path);
    sendto(server_sock, needed_buffer_two, strlen(needed_buffer_two), 0, (struct sockaddr *)&their_addr, addr_len);

	strcpy(abs_path, root_Path);
	strcat(abs_path, dest_filePath);

	//errory kedy sa majú vypísať
	if(access(abs_path, F_OK) == 0)
	{	
		char *error_message = ERROR("-ALREADY EXISTING trying to rewrite the file\n");
		sendto(server_sock, error_message, strlen(error_message), 0, (struct sockaddr *)&their_addr, addr_len);
		printf("ERROR %s : %d : %d 06 %s \n", hostIp, port, port, error_message);
		exit(EXIT_FAILURE);
	} 
	
	if(filePath[1] != 'a')	
	{
		if(access(filePath, F_OK) == -1)
		{	
			char *error_message = ERROR("-NOT EXISTING\n");
			sendto(server_sock, error_message, strlen(error_message), 0, (struct sockaddr *)&their_addr, addr_len);
			printf("ERROR %s : %d : %d 01 %s \n", hostIp, port, port, error_message);
			exit(EXIT_FAILURE);
		} 
	}

	//od klienta pride RRQ alebo WRQ a podľa buffer[1] rozhodne čo server bude robiť
	if ((number = recvfrom(server_sock, buffer, 550-1 , 0, (struct sockaddr *)&their_addr, &addr_len)) == -1)
	{
        perror("ERROR: NUMBER recvfrom\n");
        exit(EXIT_FAILURE);
	}

	buffer[number] = '\0';

	//vieme že prišla WRQ požiadavka od klienta
	if(buffer[1] == '2')
	{
		char *mess = ACK("00");
		char last_mess[550];
		strcpy(last_mess, buffer);
		char ack[10];
		strcpy(ack, mess);
		if((number = sendto(server_sock, mess, strlen(mess), 0, (struct sockaddr *)&their_addr, addr_len)) == -1)
		{
			perror("ERROR: NUMBER sednto\n");
			exit(EXIT_FAILURE);
		}

		//ak zadáme meno do stdout ale už existuje taký súbor s takým menom tak nám program vyhodí error
		
		FILE *fp = fopen(abs_path, "wb");
		if(fp == NULL)
		{
			printf("probably wrong path\n");
			exit(EXIT_FAILURE);
		}

		int write;
		do{
			if ((number = recvfrom(server_sock, buffer, 550-1 , 0, (struct sockaddr *)&their_addr, &addr_len)) == -1)
			{
				perror("ERROR WRITE recvfrom\n");
				exit(EXIT_FAILURE);
			}

			buffer[number] = '\0';

			if(strcmp(buffer, last_mess) == 0)
			{
				sendto(server_sock, ack, strlen(ack), 0, (struct sockaddr *)&their_addr, addr_len);
				continue;
			}

			write = strlen(buffer+4);
			fwrite(buffer+4, sizeof(char), write, fp);
			strcpy(last_mess, buffer);

			char block_number[3];
			strncpy(block_number, buffer + 2, 2);
			block_number[2] = '\0';
			char *fileT = ACK(block_number);
			if((number = sendto(server_sock, fileT, strlen(fileT), 0, (struct sockaddr *)&their_addr, addr_len)) == -1)
			{
				perror("ERROR: WRITE ack\n");
				exit(EXIT_FAILURE);
			}

			printf("ACK  : %d %s \n", port, block_number);

			strcpy(ack, fileT);
		} while(write == 512);

		fclose(fp);
	}

	//od klienta prišla požiadavka na RRQ operáciu
	else if(buffer[1] == '1')
	{
		char filename[100];
		strcpy(filename, buffer+2);

		//program nám hodí error ak program neexituje a my ho chceme stiahnuť
		
		FILE *fp = fopen(filename, "rb");
		if(fp == NULL)
		{
			printf("probably wrong path\n");
			exit(EXIT_FAILURE);
		}

		int block_number = 1;
		fseek(fp, 0, SEEK_END);
		int t = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		int rem = t;
		if(rem == 0)
			++rem;
		else if(rem%512 == 0)
			--rem;

		while(rem>0)
		{	
			char temp[512+5];
			if(rem>512)
			{
				fread(temp, 512, sizeof(char), fp);
				temp[512] = '\0';
				rem -= (512);
			} else 
			{
				fread(temp, rem, sizeof(char), fp);
				temp[rem] = '\0';
				rem = 0;
			}

			char *t_msg = DATA(block_number, temp);
			if((number = sendto(server_sock, t_msg, strlen(t_msg), 0, (struct sockaddr *)&their_addr, addr_len)) == -1)
			{
				perror("ERROR: RRQ data\n");
				exit(EXIT_FAILURE);
			}

			printf("DATA  %s : %d : %d \n", hostIp, port, block_number);
		
			buffer[number] = '\0';
							
			++block_number;
			if(block_number>99)
				block_number = 1;
		}
		fclose(fp);

		
	}

	close(server_sock);
	printf("ALL DONE\n");
	
	return 0;
}
