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
#include "keyvalue.h"

#define cmp_get "get"
#define cmp_put "put"
#define get_code "103"
#define put_code "112"
#define responce_found "102"
#define responce_not_found "110"
#define correct_put "100"
#define all_good_final "\nall good final\n"
#define all_good_get "all good get\n"
#define all_good_put "all good put\n"
#define initial "initial"


struct KVSstore {
    KVSpair *pairs;
    KVScompare *compare;
    size_t length;
    size_t space;
};

static const size_t kvs_pair_size = sizeof(KVSpair);

static const size_t kvs_store_size = sizeof(KVSstore);

static KVSpair *kvs_search(KVSstore *store, KVSkey *key, int exact) {
    KVSpair *pairs = store->pairs;
    size_t lbound = 0;
    size_t rbound = store->length;
    while (lbound < rbound) {
        size_t index = lbound + ((rbound - lbound) >> 1);
        KVSpair *element = pairs + index;
        int result = store->compare(key, element->key);
        if (result < 0) {
            rbound = index;
        } else if (result > 0) {
            lbound = index + 1;
        } else {
            return element;
        }
    }
    return exact ? NULL : pairs + lbound;
}

static KVSpair *kvs_get_pair(KVSstore *store, KVSkey *key) {
    if ((!store) || (!store->pairs)) {
        return NULL;
    }
    return kvs_search(store, key, 1);
}

static void kvs_abort_if_null(void *pointer, const char *message) {
    if (pointer == NULL) {
        fprintf(stderr, "%s\n", message);
        exit(-1);
    }
}

static void kvs_resize_pairs(KVSstore *store, size_t amount) {
    if (!store) {
        return;
    }
    store->length += amount;
    if (store->space > store->length) {
        return;
    }
    store->space += KVS_SPACE_INCREMENT;
    store->pairs = realloc(store->pairs, kvs_pair_size * store->space);
    kvs_abort_if_null(store->pairs, "out of memory");
}

static size_t kvs_get_pair_index(KVSstore *store, KVSpair *pair) {
    if ((!store) || (!pair)) {
        return -1;
    }
    return pair - store->pairs;
}

static size_t kvs_get_bytes_from_pair(KVSstore *store, KVSpair *pair) {
    size_t pair_index;
    if ((!store) || (!pair)) {
        return 0;
    }
    pair_index = kvs_get_pair_index(store, pair);
    return (store->length - pair_index) * kvs_pair_size;
}

static void kvs_create_pair(KVSstore *store, KVSkey *key, KVSvalue *value) {
    KVSpair *pair;
    if (!store) {
        return;
    }
    pair = kvs_search(store, key, 0);
    if (pair < store->pairs + store->length) {
        size_t bytes = kvs_get_bytes_from_pair(store, pair);
        memmove(pair + 1, pair, bytes);
    }
    pair->key = key;
    pair->value = value;
    kvs_resize_pairs(store, +1);
}

static void kvs_remove_pair(KVSstore *store, KVSpair *pair) {
    if ((!store) || (!pair)) {
        return;
    }
    memmove(pair, pair + 1, kvs_get_bytes_from_pair(store, pair + 1));
    kvs_resize_pairs(store, -1);
}

static int kvs_compare_pointers(const char *a, const char *b) {
    return a - b;
}

KVSstore *kvs_create(KVScompare compare) {
    KVSstore *store = malloc(kvs_store_size);
    kvs_abort_if_null(store, "out of memory");
    store->pairs = NULL;
    store->length = 0;
    store->space = 0;
    if (compare) {
        store->compare = compare;
    } else {
        store->compare = kvs_compare_pointers;
    }
    kvs_resize_pairs(store, 0);
    return store;
}

void kvs_destroy(KVSstore *store) {
    if (!store) {
        return;
    }
    if (store->pairs) {
        free(store->pairs);
    }
    free(store);
}

void put(KVSstore *store, KVSkey *key, void *value) {
    KVSpair *pair = kvs_get_pair(store, key);
    if (pair) {
        if (value) {
            pair->value = value;
        } else {
            kvs_remove_pair(store, pair);
        }
    } else if (value) {
        kvs_create_pair(store, key, value);
    }
}

KVSvalue *get(KVSstore *store, KVSkey *key) {
    KVSpair *pair = kvs_get_pair(store, key);
    return pair ? pair->value : NULL;
}

void kvs_remove(KVSstore *store, KVSkey *key) {
    put(store, key, NULL);
}

size_t kvs_length(KVSstore *store) {
    if (!store) {
        return 0;
    }
    return store->length;
}

KVSpair *kvs_pair(KVSstore *store, size_t index) {
    if ((!store) || (index < 0) || (index >= store->length)) {
        return NULL;
    }
    return store->pairs + index;
}




int main(int argc, char *argv[]) {

	char *parameter_server_port= argv[1];
	char send_code_get[512];
	char send_value_get[512];
	char send_code_put[512];
	char client_send_merged[512];
	char client_send_merged2[512];
	char client_send_merged_get_copy[512];
	char client_send_merged_put_copy[512];
	char server_send_merged[512];
	char *ignore_from_get;
	char *ignore_from_put;
	char client_send_argc[512];
	char client_send_argc_responce[5] = "YES";
	char buffer[512];
	char *key;
	char *value;
	char *keywithvalue1;
	char keywithvalue2[128];
	char keyvaluebuffer[512];
	

	char getchrserv = '3';
	char putchrserv = '2';


	int serv3port;
	int k = 0;
	int v = 0;
	int i = 0;
	int pid;
	int j;
	int client_len;
	int server_socket;
	int client_socket;
	int client_argc; //for loop
	int parameter_server_port_int;

	struct sockaddr_in server_address, client_address;


		
	KVSstore *store = kvs_create(strcmp);
	KVSpair *p;


	//command line arguments check
	if (argc==3){	//remember to change for serv3
		

		//server socket
		server_socket = socket(AF_INET, SOCK_STREAM, 0);
	

		//server address
		parameter_server_port_int = atoi(parameter_server_port);
		server_address.sin_family = AF_INET;
		server_address.sin_port = htons(parameter_server_port_int);
		server_address.sin_addr.s_addr = INADDR_ANY;//maybe htonl

		//bind function
		bind(server_socket, (struct sockaddr *) &server_address, sizeof(server_address));
		
		serv3port = atoi(argv[2]);
		
		//listen function
		listen(server_socket, serv3port);

		//accept function
		client_len = sizeof(client_address);

for(j=0 ; j <= serv3port ; j++ ){

	while (1) {
     		client_socket = accept(server_socket,(struct sockaddr *) &client_address, &client_len);
     		if (client_socket < 0) 
       		 	error("ERROR on accept");
    		pid = fork();
     		if (pid < 0)
			error("ERROR on fork");
   		if (pid == 0)  {
      			close(server_socket);
 

		//printf("connection successful\n");

		//read number of arguements from client to know how many time to loop

		//client_send_argc = (char *)malloc(256 * sizeof(char));
		/*read(client_socket, buffer, 511);
		printf("read 1\n");
		strcpy(client_send_argc,buffer);		

		printf("clients argc:%s\n",client_send_argc);
		bzero(buffer,512);
		strcpy(buffer, client_send_argc_responce);
		
		write(client_socket, buffer, strlen(buffer));
		printf("write 2\n");
		printf("client_argc is set\n");

		client_argc = atoi(client_send_argc);
		printf("%s\n", buffer);
		bzero(buffer , 512);
		printf("%s\n", buffer);
		printf("%d\n", client_argc);*/



		
		
		put(store , "city" , "larnaca");
		put(store , "street" , "vega");

		//printf("%lu\n", strlen(buffer));
		bzero(buffer , 512);
		read(client_socket, buffer, 511);
		//strcpy(buffer , initial);
		while( buffer != NULL ){ 
			//bzero(buffer , 512);
			//printf("read 1\n");
			//read(client_socket, buffer, 511);
			//read incoming data again if more than 1 get/put
			//client_send_merged = (char *) malloc( 100 * sizeof(char));
			//printf("%s\n", buffer);
			bzero(client_send_merged , strlen(client_send_merged));
			strcpy(client_send_merged , buffer);
			//printf("%s\n", client_send_merged);
			
			//ignore_from_get = strstr(client_send_merged,get);
			//strcpy(ignore_from_get , strstr(client_send_merged , get));
			//ignore_from_put = strstr(client_send_merged,put);
			//strcpy(ignore_from_put , strstr(client_send_merged, put) );	
			//forstrstr = strstr(client_send_merged , "103");		
			//if( strcmp(forstrstr, get), get) == 0){
			//ignore_from_get = "103";
			bzero(client_send_merged_get_copy , strlen(client_send_merged_get_copy));
			bzero(client_send_merged_put_copy , strlen(client_send_merged_put_copy));
			bzero(client_send_merged2 , strlen(client_send_merged2));
			strcpy(client_send_merged2 , client_send_merged);
			strcpy(client_send_merged_get_copy , client_send_merged);
			strcpy(client_send_merged_put_copy , client_send_merged);
			strtok(client_send_merged_get_copy , "3");
			
			if(strcmp(client_send_merged_get_copy , "10") == 0) {
				//printf("i received get\n");
				key = strchr(client_send_merged , getchrserv) + 1;
				//printf("%s\n" , key);
				bzero(buffer , 512);
					if( (get(store , key))!= NULL){
						strcat(buffer , responce_found);
						strcat(buffer , get(store , key));
						//printf("im sending %s\n" , buffer);
						write(client_socket, buffer, strlen(buffer));
						bzero(key , strlen(key));
					}
					else if( (get(store , key)) == NULL){
					strcpy(buffer , responce_not_found);
					//printf("im sending %s\n" , buffer);
					write(client_socket , buffer , strlen(buffer));
					bzero(buffer, 512);
					}
			}


			strtok(client_send_merged_put_copy, "2");

			if( strcmp(client_send_merged_put_copy,"11") == 0){
		
				//printf("i received put\n");
				
				keywithvalue1 = strchr(client_send_merged , putchrserv) + 1;
				
				//printf("%s\n" , keywithvalue1);
				/*strncpy(keywithvalue2 , keywithvalue1 , 50);
				key = strchr(keywithvalue1 , '\0');
				value = strchr(keywithvalue2 , '\0');
				put(store , key , value);
				printf("%s\n" , keywithvalue2);
				printf("%s\n" , key);
				bzero(buffer , 512);
				strcpy(buffer , all_good_put);*/
				//printf("%s\n" , keywithvalue1);

				for(k=0; keywithvalue1[k] != '\0' ; ++k){


					strncat(keyvaluebuffer , &keywithvalue1[k],1);
					//printf("%s\n" , keyvaluebuffer);


				}
				key = (char *)malloc(strlen(keyvaluebuffer));
				//bzero(key , strlen(key));
				strcpy(key , keyvaluebuffer);
				//printf("exit 1st loop\n");
				//printf("%s\n" , key);
				bzero(keyvaluebuffer , 512);

				for(v=k+1; keywithvalue1[v] != '\0' ; ++v ){


					strncat(keyvaluebuffer , &keywithvalue1[v],1);
					//printf("%s\n" , keyvaluebuffer);

				}
				//printf("exit 2nd loop\n");
				//printf("%s\n" , keyvaluebuffer);
				//printf("flag\n");
				//printf("%lu\n",strlen(keyvaluebuffer));
				value = (char *)malloc(strlen(keyvaluebuffer));
				//printf("%s\n" , value);
				//bzero(value , strlen(value));
				strcpy(value , keyvaluebuffer);
				//printf("%s\n" , value);
				//printf("key : %s value : %s \n", key , value);
				put(store , key , value);
				bzero(buffer , 512);
				strcpy(buffer , get(store , key) );
				write(client_socket, buffer, strlen(buffer));
				//bzero(key , strlen(key));
				//bzero(value , strlen(value));
				free(value);
				free(key);

			
			}	
			

			//send function
			bzero(buffer , 512);
			strcpy(buffer , "\n");
			write(client_socket, buffer, strlen(buffer));
			bzero(buffer , 512);
			bzero(client_send_merged_get_copy, 512);
			bzero(client_send_merged_put_copy, 512);
			read(client_socket, buffer, 511);
			//printf("%lu\n", strlen(buffer));
		}
		
    				    
   			 }
   		  else close(client_socket);
	 } /* end of while */

}
	}
	else {
		printf("wrong command\n");
	}


	//closing socket
	close(server_socket);

//printf("server finished correctly\n");
	return 0;
}

