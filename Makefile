.Phony: all run 

all:
	gcc -Wall -Werror -Wextra -lm tftp-server.c -o tftp-server
	gcc -Wall -Werror -Wextra -lm tftp-client.c -o tftp-client

run: 
	gcc tftp-server.c -o server
	gcc tftp-client.c -o client
