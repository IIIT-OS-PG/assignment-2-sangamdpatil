#include<iostream>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include <fcntl.h>
#include<netinet/in.h>
#include<cstring>
#include<string.h>
#include<pthread.h>
#include<bits/stdc++.h>
#define PORT 8081 
using namespace std; 
struct thread_args 
{
	int port;
	int socket;
	char ip[33];
    char buff[100];
	char direc[100];
};
/*struct server_args
{
	int port;
	
};*/


int tokenizer(char *msg,char **temp,char *delim)
{
		
		char *token; 
		char *rest = msg;
		int i=0;
		while ((token = strtok_r(rest, delim, &rest))) 
		{	
			temp[i++]=token;
		}
		return i;
}
void *handler(void *arg)
{
	struct thread_args *req = (thread_args *)arg;
	char *temp[10];
	char msg[100]="";
	//cout << "in peer server handler"<<endl;
	int rc = recv(req->socket,msg,sizeof(msg),0);
	//cout << rc<<endl;
	auto unused = tokenizer(msg,temp,"-");
	//auto unused = tokenizer(req->msg,temp," ");
	if(strcmp(temp[0],"upload")==0)
	{
		
		FILE *fp = fopen(temp[1],"rb");
		fseek ( fp , 0 , SEEK_END);
		size_t sz = ftell( fp );
		rewind(fp);
		//struct fileInfo *data = (struct fileInfo *)malloc(sizof(struct fileInfo));
		string sendmsg = to_string(sz);
		sendmsg = sendmsg+"-"+"hash"+to_string(req->port);
		//cout << sendmsg<<endl;
		send(req->socket,sendmsg.c_str(),strlen(sendmsg.c_str()),0);
	}
	else if(strcmp(temp[0],"download")==0)
	{
		FILE *fp = fopen(temp[1],"rb");
		
		fseek ( fp , 0 , SEEK_END);
		size_t sz = ftell( fp );
		rewind(fp);
		string dir(temp[2]);
		string sendmsg = to_string(sz)+"-"+dir;
		send(req->socket,sendmsg.c_str(),strlen(sendmsg.c_str()),0);
		size_t n;
		char buff[1024];
		while ( ( n = fread( buff , sizeof(char) , 1024 , fp ) ) > 0 && sz > 0 )
		{
			send (req->socket , buff, n, 0 );
			memset ( buff , '\0', 1024);
			sz = sz - n ;//cout << sz<<endl;
		}
		close(req->socket);
		fclose(fp);
	}
}
void* server(void *arg)
{
	struct thread_args *server = (struct thread_args *) arg;
	
	//cout << "In server thread"<<endl;
	int server_socket = socket(AF_INET,SOCK_STREAM,0);
	if(server_socket < 0)
		cout << "Socket Error";
	struct sockaddr_in server_address;
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(server->port);
	server_address.sin_addr.s_addr = INADDR_ANY;
	auto b = bind(server_socket,(struct sockaddr *) &server_address,sizeof(server_address));
	if(b<0)cout << "bind error";
	auto listen_status = listen(server_socket,2);
	if(listen_status<0) cout << "listening error";
	int client_socket,i=0,c;
	pthread_t thread[5];
	while(1)
	{
		client_socket = accept(server_socket,NULL,NULL);
		struct thread_args *req = (struct thread_args *)malloc(sizeof(struct thread_args));
		if(c<0) cout <<"accept socket error";
		req->socket = client_socket;
		req->port = server->port;
		strcpy(req->ip,server->ip);
		pthread_create(&thread[i++],NULL,handler,req);
		
		//cout <<"Peer server thread"<<endl;
	}
	
	/*FILE *in = fopen(server->second,"rb");
	fseek ( in , 0 , SEEK_END);
	int fsize = ftell ( in );
	rewind ( in );
	char buff[1024];
	size_t n;
	send( client_socket , &fsize, sizeof(fsize), 0);
	while ( ( n = fread( buff , sizeof(char) , 1024 , in ) ) > 0 && fsize > 0 )
	{
		send (client_socket , buff, n, 0 );
		memset ( buff , '\0', 1024);
		fsize = fsize - n ;
	}
	close(client_socket);
	fclose(in);*/
	
	return NULL;

}


void * downloadHandler(void *arg)
{
	struct thread_args *req=(struct thread_args *)arg;
	int sock = 0, valread; 
    struct sockaddr_in serv_addr; 
    //char *hello = "Hello from client"; 
    char buffer[1024] ; 
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    { 
        printf("\n Socket creation error \n"); 
		exit(1); 
    } 
   
    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_port = htons(req->port); 
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
    { 
        printf("\nConnection Failed \n"); 
        exit(1);
    }
	string fname(req->buff);
	string direc(req->direc);
	string msg="download-"+fname+"-"+direc;
	cout << msg<<endl;
	auto sd = send(sock,msg.c_str(),strlen(msg.c_str()),0);
	cout << sd<<endl;
	char temp[100]="";
	auto rc = recv(sock,temp,sizeof(temp),0);
	cout << rc<<endl;
	char *tk[10];
	
	string fsize(temp);
	printf("%s\n",temp);
	FILE *out=fopen(req->direc,"wb");
	size_t file_size=stoi(fsize);
	recv(sock, &file_size, sizeof(file_size), 0);
	size_t n;
	char buff[1024];
	
	while(( n = recv( sock , buff , 1024, 0) ) > 0 && file_size  > 0)
	{
		fwrite(buff, sizeof(char), n, out);
		memset( buff , '\0', 1024);
		file_size = file_size - n;//cout << file_size<<endl;
	}
	close(sock);
	fclose(out);
}
void * client(void *arg)
{
	struct thread_args *req = (struct thread_args *)arg;
	int sock = 0, valread; 
    struct sockaddr_in serv_addr; 
    //char *hello = "Hello from client"; 
    char buffer[1024] ; 
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    { 
        printf("\n Socket creation error \n"); 
		exit(1); 
    } 
   
    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_port = htons(PORT); 
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
    { 
        printf("\nConnection Failed \n"); 
        exit(1);
    }
	//struct thread_args *client_req = (struct thread_args *)arg;
	while(true)
	{
		char temp[100]="";
		cin.getline(temp,100);
		if(strncmp(temp, "login", 5)==0)
		{	
			string str(temp);
			str = str+" "+req->ip+" "+to_string(req->port);
			strcpy(temp,str.c_str());
			//strcat(temp,"");	
		}
		auto sd = send(sock,temp,sizeof(temp),0);
		char recvdMsg[100]="";
		memset(recvdMsg,0,sizeof(recvdMsg));
		recv(sock,recvdMsg,sizeof(recvdMsg),0);
		if(strncmp(recvdMsg,"download",8)==0)
		{	cout <<"in download peer client"<<endl;
			char *temp[30];
			auto tokCount = tokenizer(recvdMsg,temp,"-");
			char *temp2[30];
			struct thread_args *req = (struct thread_args*)malloc(sizeof(struct thread_args));
			strcpy(req->buff,temp[1]);//buff holds fileName
			strcpy(req->direc,temp[2]); 
		 	pthread_t thread[5];
		 	for(int i=3;i<tokCount;i++)
			{	
				int unused = tokenizer(temp[i],temp2,":");
				cout <<temp2[0] << ","<<temp2[1]<<endl;
				//auto unused = tokenizer(temp[i],temp2,":");
				strcpy(req->ip,temp2[0]);
				string pp(temp2[1]);
				req->port = stoi(pp);
				
				pthread_create(&thread[i++],NULL,downloadHandler,req);
				if(i==5 || i==6)
					pthread_join(thread[i],NULL);
			}
		}
		printf("%s\n",recvdMsg);
		memset(temp,0,sizeof(temp));
	}
}
int main(int argc, char const *argv[]) 
{
	pthread_t t1,t2;
	//pthread_create(&t1, NULL, server, NULL);
	struct thread_args *client_req = (struct thread_args *)malloc(sizeof(thread_args));
	struct thread_args *server_req = (struct thread_args *)malloc(sizeof(thread_args));
	//char temp[msg.size() + 1];
	//strcpy(temp, msg.c_str());
	//strcpy(client_req->msg,temp);
	server_req->port = atoi(argv[2]);
	strcpy(server_req->ip,argv[1]);
	client_req->port = atoi(argv[2]);
	strcpy(client_req->ip,argv[1]);
	pthread_create(&t1, NULL, client, client_req);
	pthread_create(&t2, NULL, server, server_req);
	pthread_join( t1, NULL);
    pthread_join( t2, NULL);
	return 0;
}