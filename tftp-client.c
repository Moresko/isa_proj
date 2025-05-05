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

void convert_info_from_server(const char *buff, char *rootPath)
{
    sscanf(buff, "%s", rootPath);
}

void convert(char *sym, int digit){
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

//funckia pomocou ktorej program zisti ze sa jedna o RRQ operáciu
char* RRQ(char *filename){
    size_t packet_size = 2 + strlen(filename) + 1; 
    char *packet = calloc(packet_size, sizeof(char));
    strcat(packet, "01");
    strcat(packet, filename);

    return packet;
}

//funckia pomocou ktorej program zisti ze sa jedna o WRQ operáciu
char* WRQ(char *filename)
{
    size_t packet_size = 2 + strlen(filename) + 1; 
    char *packet = calloc(packet_size, sizeof(char));
    strcat(packet, "02");
    strcat(packet, filename);

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

//funckia pomocou ktorej program zisti ze sa jedna o ACK operáciu
char* ACK(char* block_number)
{
	size_t packet_size = 2 + strlen(block_number) + 1;
    char *packet = calloc(packet_size, sizeof(char));
	strcat(packet, "04");
	strcat(packet, block_number);

    return packet;
}

int main(int argc, char* argv[]){
	int port = 69;
	int client_socket;
	int number;
    char mod[10] = "octet";
	char *filePath = NULL;
	char *hostIp = NULL;
	char *dest_filePath = NULL;
	char buffer[1024];
    char needed_buffer[1024];
    char needed_buffer_two[1024];
    char root_path[256];
    char totalPath[256];
    int num = 1;
	struct sockaddr_storage their_addr;
	socklen_t addr_len;
    
	
    //spracovanie argumentov

    if (argc > 9 || argc < 7 ){
        perror("ERROR: Arguments\n");
        exit(EXIT_FAILURE);
    }

	for(int i = 0; i < argc; i++)
	{
		if(strcmp(argv[i], "-h") == 0){
            hostIp =  argv[i + 1];
        }
		if(strcmp(argv[i], "-p") == 0)
		{
			if(i + 1 < argc)
			{
				port = atoi(argv[i + 1]);
				if(port < 0 || port > 65536)
				{
					perror("ERROR: Bad port numbers\n");
					exit(EXIT_FAILURE);
				}
			}
        }
		if(strcmp(argv[i], "-f") == 0)
		{
            filePath =  argv[i + 1];
        }	
        if(strcmp(argv[i], "-t") == 0)
        {
            dest_filePath = argv[i +1];
        }

	}
	//koniec spracovania argumentov

    //nastavovanie UDP spojenia
	if((client_socket = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
        perror("ERROR: Client socket\n");
        exit(EXIT_FAILURE);
    }
    printf("Client done with socket\n");
    
	struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    // inet_pton(AF_INET, hostIp, &server_address.sin_addr);
    server_address.sin_port = htons(port);
    
    //koniec nastavovania spojenia UDP
    
    //posielanie potrebných informácií na server
    if(filePath == NULL)
    {   
        filePath = "aa";
        num = 2;
        snprintf(needed_buffer, sizeof(needed_buffer), "%s %s %s", hostIp, filePath, dest_filePath);
        sendto(client_socket, needed_buffer, strlen(needed_buffer), 0, (struct sockaddr *)&server_address, sizeof(server_address));
    }
    else
    {
        snprintf(needed_buffer, sizeof(needed_buffer), "%s %s %s", hostIp, filePath, dest_filePath);
        sendto(client_socket, needed_buffer, strlen(needed_buffer), 0, (struct sockaddr *)&server_address, sizeof(server_address));
    }

    if(num == 2)
    {
        filePath = NULL;
    }
  
    ssize_t information_from_client = recvfrom(client_socket, needed_buffer_two, sizeof(needed_buffer_two) - 1 , 0, (struct sockaddr *)&their_addr, &addr_len);
    needed_buffer_two[information_from_client] = '\0'; 
    convert_info_from_server(needed_buffer_two, root_path);


	strcpy(totalPath, root_path);
	strcat(totalPath, dest_filePath);
    
    //errory kedy sa majú vypísať
    if(access(totalPath, F_OK) == 0)
    {
        number = recvfrom(client_socket, buffer, 550-1 , 0, (struct sockaddr *)&their_addr, &addr_len);
        buffer[number] = '\0';

        if(buffer[1] == '5')
        {
            printf("ERROR: file already existing\n");
            exit(EXIT_FAILURE);
            close(client_socket);
        }
    }
    
    if(filePath != NULL )
    {
        if(access(filePath, F_OK) == -1)
        {
            number = recvfrom(client_socket, buffer, 550-1 , 0, (struct sockaddr *)&their_addr, &addr_len);
            buffer[number] = '\0';

            if(buffer[1] == '5')
            {
                printf("ERROR: file not existing\n");
                exit(EXIT_FAILURE);
                close(client_socket);
            }
        }
    }

    //ak nie je v argumentoch špecifikovaní -f program skáče do if
    if(filePath == NULL)
    {   
        FILE *fpr;
       
        char filename[100] = "pok.txt";

        fpr= fopen(filename, "w");

        printf("Napiste co ma obsahovat subor, potom stlacte ENTER a hned ctrl+D\n");
        //klient napíše na stdout čo má obsahovať súbor
        char storage[1024];
        while (fgets(storage, sizeof(storage), stdin) != NULL) 
        {
            fprintf(fpr, "%s", storage);
        }
        fclose(fpr);
        
        // klient pošle na server WRQ požiadavku
        char *mess = WRQ(filename);
        if((number = sendto(client_socket, mess, strlen(mess), 0, (struct sockaddr *)&server_address, sizeof(server_address))) == -1)
        {
            perror("EEROR: WRQ sendto\n");
            exit(EXIT_FAILURE);
        }

        printf("WRQ %s : %d %s %s \n", hostIp, port, filename, mod);

        number = recvfrom(client_socket, buffer, 550-1 , 0, (struct sockaddr *)&their_addr, &addr_len);
        buffer[number] = '\0';

        //ak buffer[1]=='4' tak vieme že má príst fukncia DATA
        if(buffer[1]=='4')
        {   
            
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
                char fileT[512+5];
                if(rem>512)
                {
                    fread(fileT, 512, sizeof(char), fp);
                    fileT[512] = '\0';
                    rem -= (512);
                } 
                else 
                {
                    fread(fileT, rem, sizeof(char), fp);
                    fileT[rem] = '\0';
                    rem = 0;
                }

                char *data_block_pack = DATA(block_number, fileT);
                if((number = sendto(client_socket, data_block_pack, strlen(data_block_pack), 0, (struct sockaddr *)&server_address, sizeof(server_address))) == -1)
                {
                    perror("ERROR: DATA_PACK sendto");
                    exit(EXIT_FAILURE);
                }

                printf("DATA %s : %d : %d %d \n", hostIp, port, port, block_number);

                number = recvfrom(client_socket, buffer, 550-1 , 0, (struct sockaddr *)&their_addr, &addr_len);
                buffer[number] = '\0';
                
                if(buffer[1]=='5')
                {
                    printf("ERROR apear\n");
                    exit(EXIT_FAILURE);
                }

                ++block_number;
                if(block_number>99)
                    block_number = 1;
            }
            
            fclose(fp);
        } 
        else 
        {
            perror("ERROR: Client");
            exit(EXIT_FAILURE);
        }

        remove(filename);
    }
    //máme špecifikovanú cestu -f
    else
    {
        //posiela sa RRQ požiadavka na server
        char *mess = RRQ(filePath);
		char last_mess[550];
        strcpy(last_mess, "");
		char ack[10];
        strncpy(ack, mess, sizeof(ack) - 1);
        ack[sizeof(ack) - 1] = '\0';
		if((number = sendto(client_socket, mess, strlen(mess), 0, (struct sockaddr *)&server_address, sizeof(server_address))) == -1)
        {
			perror("ERROR: NUMBER sendto");
			exit(EXIT_FAILURE);
		}
        printf("RRQ %s : %d %s %s \n", hostIp, port, filePath, mod);

		FILE *fp = fopen(totalPath, "wb");
        if(fp == NULL)
		{
			printf("probably wrong path\n");
			exit(EXIT_FAILURE);
		}

        int write;
		do{ 
			
			addr_len = sizeof their_addr;
			if ((number = recvfrom(client_socket, buffer, 550-1 , 0, (struct sockaddr *)&their_addr, &addr_len)) == -1) 
            {
				perror("ERROR: RRQ recvfrom");
				exit(EXIT_FAILURE);
			}

			buffer[number] = '\0';

            if(buffer[1]=='5'){
				printf("ERROR apear\n");
				exit(EXIT_FAILURE);
			}


			if(strcmp(buffer, last_mess) == 0)
            {
				sendto(client_socket, ack, strlen(ack), 0, (struct sockaddr *)&their_addr, addr_len);
				continue;
			}

			write = strlen(buffer+4);
			fwrite(buffer+4, sizeof(char), write, fp);
			strcpy(last_mess, buffer);

            char block_number[3];
			strncpy(block_number, buffer+2, 2);
			block_number[2] = '\0';
			char *fileT = ACK(block_number);
			if((number = sendto(client_socket, fileT, strlen(fileT), 0, (struct sockaddr *)&server_address, sizeof(server_address))) == -1)
            {
				perror("ERROR: RRQ ack");
				exit(EXIT_FAILURE);
			}

            printf("ACK  %s : %d %s \n", hostIp, port, block_number);

			strcpy(ack, fileT);
		} while(write == 512);

		fclose(fp);
    }
	
	close(client_socket);
	printf("ALL DONE\n");

	return 0;
}
