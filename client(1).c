#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define cmp_get "get"
#define cmp_put "put"
#define get_code "103"
#define put_code "112"
#define responce_found "102"
#define responce_not_found "110"
#define correct_put "100"




int main(int argc, char *argv[]) {

	int i;
	int network_socket;
	int parameter_server_port_int;

	
	char *parameter_server_address = argv[1];
	char *parameter_server_port = argv[2];
	char send_code_get[128];
	char send_code_put[128];
	char send_key_get[128];
	char send_key_put[128];
	char send_value_put[128];
	char client_send_merged[128];
	char server_send_merged[128];
	char client_send_argc_responce[128];
	char print_get[128];
	char ignore_from_get_found[512];
	char ignore_from_get_not_found[512];
	char client_send_argc[512]; 
	char buffer[512];
	char buffer_get[512];
	char *server_send_key;


	char getchrclient = '2' ;
	char *put_code_chr = "p";

	
	
	struct sockaddr_in server_address;
	struct hostent *server;


	//correct command, ./client address port put/get parameterers
	if(argc >5 ||  argc == 5){

		//creating the socket
		network_socket = socket(AF_INET, SOCK_STREAM, 0);

		//specify address for the socket
		server = gethostbyname(parameter_server_address);
		//na to fiw meta
		if (server == NULL) {
        		//fprintf(stderr,"ERROR, no such host\n");
       			 exit(0);
    		}
		bzero((char *) &server_address, sizeof(server_address));
		parameter_server_port_int = atoi(parameter_server_port);
		server_address.sin_family = AF_INET;
		bcopy((char *)server->h_addr, (char *)&server_address.sin_addr.s_addr, server->h_length);
		server_address.sin_port = htons(parameter_server_port_int); //check

		//server_address.sin_addr.s_addr = INADDR_ANY
		

		//connection()
		int connection_status = connect(network_socket, (struct sockaddr *) &server_address, sizeof(server_address));
				
		//check for connection error
		if(connection_status == -1){
		//printf("there was an error in the connection\n");
		}
		else if(connection_status == 0){
		//printf("connected\n");
		}

		/*client_send_argc = (char *)malloc(256 * sizeof(char));
		snprintf(client_send_argc, 100,"%d",argc);
		printf("me to client_send_argc stelnw:%s\n",client_send_argc);
		printf("hostname:%s port:%s get/put:%s value:%s complete send string:%s\n",argv[1], parameter_server_port,argv[3],send_value_put,client_send_merged);*/
		//printf("%lu\n",sizeof(get_code));
		//printf(get_code"\n");
		for(i=3;  i < argc; i++){

			//in case typed get
			if(strcmp(argv[i],cmp_get) == 0){

				//send_code_get = (char *) malloc(strlen(get));
				//send_code_get = get;
				strcpy(send_code_get, get_code);
				//send_key_get = (char *)malloc(strlen( argv[i] ));
				i++;
				strcpy( send_key_get, argv[i] );
				

				//unify string before send 
				//client_send_merged = (char *) realloc(client_send_merged , strlen(send_code_get) + strlen(send_key_get) + 2);
				bzero(client_send_merged, 512);
				strcat(client_send_merged,send_code_get);
				strcat(client_send_merged,send_key_get);


//printf("hostname:%s port:%s get/put:%s value:%s complete send string:%s\n",argv[1], parameter_server_port,argv[i],send_code_get,client_send_merged);
		

				//send data
				/*bzero(buffer,512);
				strcpy(buffer, client_send_argc);
				write(network_socket, buffer, strlen(buffer));
				printf("write 1\n");
				client_send_argc_responce = (char *)malloc(256 * sizeof(char));
				bzero(buffer, 512);
				read(network_socket, buffer, 511);
				printf("read 2\n");
				strcpy(client_send_argc_responce, buffer);
				printf("did client_send_argc went to server:%s\n",client_send_argc_responce);*/
				
				bzero(buffer,512);
				strcpy(buffer, client_send_merged);
				//printf("%s\n", buffer);
				write(network_socket, buffer, strlen(buffer));
				//printf("write get\n");
				//receive data
				bzero(buffer, 512);
				//server_send_merged = (char *) malloc(256 * sizeof(char));
				read(network_socket, buffer, 511);
				//printf("i received the answer from server:%s \n",buffer);	
				strcpy(server_send_merged, buffer);
				//printf("%s\n" , server_send_merged);
					//print server responce
					bzero(ignore_from_get_found , strlen(ignore_from_get_found));
					bzero(ignore_from_get_not_found , strlen(ignore_from_get_not_found));
					bzero(buffer_get , strlen(buffer_get));
					strcpy(buffer_get , buffer);
					strcpy(ignore_from_get_not_found , strtok(buffer_get , "0"));
				 	strcpy(ignore_from_get_found, strtok(buffer, "2"));		
					//printf("%s\n" , server_send_merged);			

					if( strcmp(ignore_from_get_not_found , "11") == 0){
						printf("\n");			
				
					}
					else if(strcmp(ignore_from_get_found , "10") == 0){
					
					server_send_key = strchr(server_send_merged , getchrclient) + 1;
					printf("%s\n" ,server_send_key); 
					
					}
					else if(strcmp(ignore_from_get_not_found , "11") != 0  |  strcmp(ignore_from_get_found , "10") != 0 ){
					close(network_socket);	
					}
				bzero(ignore_from_get_found , strlen(ignore_from_get_found));
				bzero(ignore_from_get_not_found , strlen(ignore_from_get_not_found));
				bzero(client_send_merged , 512);
				bzero(buffer, 512);
				bzero(buffer_get , strlen(buffer_get));
				bzero(server_send_key , strlen(server_send_key));
				//i++;
			}

			//in case typed put
			else if(strcmp(argv[i],cmp_put) == 0){


				//assign and unify string
				//send_code_put = (char *)malloc( strlen(put) + 1 );
				//send_code_put = put;
				bzero(send_code_put , strlen(send_code_put));
				strncat(send_code_put, put_code , 3);
				//send_key_put =(char *) malloc( strlen( argv[i] ) + 1 );
				i++;
				strncpy( send_key_put, argv[i], strlen(argv[i]) );
				//send_value_put =(char *) malloc( strlen( argv[i] ) + 1 );
				i++;
				strncpy( send_value_put, argv[i] , strlen(argv[i]) );
				//client_send_merged = (char *) malloc( strlen(send_code_put) + strlen(send_key_put) + strlen(send_value_put) + 3);
				bzero(client_send_merged , 512);
				strncat(client_send_merged,send_code_put, strlen(send_code_put));
				strncat(client_send_merged,send_key_put,strlen(send_key_put) + 1);
				strncat(client_send_merged,"\0",2);
				strncat(client_send_merged,send_value_put , strlen(send_value_put));
				//printf("%lu\n", strlen(client_send_merged));


				//printf("hostname:%s port:%s get/put:%s value:%s complete send string:%s\n",argv[1], parameter_server_port,argv[i-2],send_code_put,client_send_merged);
				

				//send data
				/*bzero(buffer , 512);
				strcpy(buffer, client_send_argc);
				write(network_socket, buffer, strlen(buffer));
				bzero(buffer , 512);
				read(network_socket , buffer , 511);
				strcpy(client_send_argc_responce , buffer);
				printf("did server received the argc:%s\n", client_send_argc_responce);*/
				bzero(buffer, 512);
				strcpy(buffer , client_send_merged);
				write(network_socket, buffer, strlen(buffer));

				//receive data
				bzero(buffer , 512);
				read(network_socket, buffer, 511);
				strcpy(server_send_merged, buffer);
				//printf("i received the answer from server:%s \n",server_send_merged);
	
				//ena to fiw meta apla gia na elegxw
				//if(strcmp(server_send_merged,correct_put ) == 0){
				//printf("%s\n",server_send_merged);

				//}
					
				bzero(client_send_merged, 512);
				bzero(buffer, 512);
				//i++;
			}

		//bzero(buffer, 512);
		//bzero(client_send_merged , 512);
		}





//printf("exit loop correct \n");


	
	}

	//wrong command,only ./client address port or less
	else {
		//printf("Not enough information\n");
	}
	

//printf("client finished correctly \n");
	
//close function				
close(network_socket);		
return 0;
}

/*printf("hostname:%s port:%s get/put:%s value:%s complete send string:%s\n",argv[1], parameter_server_port,argv[3],send_value_put,client_send_merged);*/




