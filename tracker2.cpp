#include<iostream>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include <fcntl.h>
#include<netinet/in.h>
#include<cstring>
#include<pthread.h>
#include<bits/stdc++.h>
#define PORT 8081 
using namespace std;

struct thread_args 
{
	int socket;
    char msg[100];
};
struct group
{
	string gid;
	string uid;
	vector<string> pending;
	vector<string> active;
};
struct fileInfo
{
	string path;
	string hash;
	size_t sz;
	string owner;
	vector<string> users;
};

map<string,string> userCred;
map<string,struct group * > groupDetails ;
map<string,vector<pair<string,struct fileInfo *> > > fileGroupInfo;
map<string,pair<string,int> > clientInfo;
bool isGroupValid(string gid)
{
	if(groupDetails.find(gid)!=groupDetails.end())
		return true;
	else 
		return false;
}
bool isUserInGroup(string gid,string uid)
{
	if(find(groupDetails[gid]->active.begin(),groupDetails[gid]->active.end(),uid)!=groupDetails[gid]->active.end())
		return true;
	else
		return false;
}
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
void init()
{
	//map<string,vector<pair<string,struct fileInfo *> > > fileGroupInfo;
	/*struct fileInfo
	{
	string path;
	string hash;
	size_t sz;
	string owner;
	vector<string> users;
	}*/
	for(auto it=fileGroupInfo["2411"].begin();it!=fileGroupInfo["2411"].end();it++)
	{
		if(it->first == "aps")
		{	
			it->second->users.push_back("qwe");
			it->second->users.push_back("asd");
			//it->second->users.push_back("poi");
			//it->second->users.push_back("lkj");
			
		}
	}
}
void * handler(void *arg)
{
	struct thread_args *req = (struct thread_args *) arg;
	//printf("%s\n",req->msg);
	string userId;
	bool loggedIn = false;
	
	while(1)
	{
		string msg="";
		char *temp[30];
		auto unused = tokenizer(req->msg,temp," ");
		if(strcmp(temp[0],"create_user")==0)
		{	
			string uid(temp[1]);
			string pass(temp[2]);
			userCred[uid]=pass;
			msg="user created";
			//cout << "user created"<<endl;
			//send(client_req->socket,"User Created.",14,0);
			auto sd = send(req->socket,(char *)msg.c_str(),strlen(msg.c_str()),0);
		}
		else if(strcmp(temp[0],"login")==0)
		{	//cout << "thread server"<<endl;
			string uid(temp[1]);
			string pass(temp[2]);
			string ip(temp[3]);
			string pp(temp[4]);
			int peerPort = stoi(pp);
			if(userCred.find(uid)!=userCred.end() && userCred[uid] == pass)
			{	
				//cout << "login successful"<<endl;
				msg="login successful";
				userId = uid;
				loggedIn = true;
				clientInfo[uid] = {ip,peerPort};
				
			}
			else
				msg="login failed";
			auto sd = send(req->socket,(char *)msg.c_str(),strlen(msg.c_str()),0);
		}
		else if(strcmp(temp[0],"create_group")==0)
		{	//cout << "Hello"<<endl;
			if(loggedIn)
			{
				string gid(temp[1]);
				struct group *gp = (struct group *)malloc(sizeof(struct group));
				gp->gid = gid;
				gp->uid = userId;
				vector<string> k;
				k.push_back(userId);
				gp->active=k;
				groupDetails[gid] = gp;
				
				msg="New group created";
				//for(auto it = groupDetails.begin();it!=groupDetails.end();it++)
					//cout << it->first<<" "<<it->second->uid<<endl;				
			}
			else
				msg="User not logged in";
			auto sd = send(req->socket,(char *)msg.c_str(),strlen(msg.c_str()),0);
		}
		else if(strcmp(temp[0],"join_group")==0)
		{
			if(loggedIn)
			{
				string gid(temp[1]);
				//struct group *gp = (struct group *)malloc(sizeof(struct group));
				//gp->gid = gid;
				//gp->uid = userId;
				//vector<string> temp;
				//temp.push_back(userId);
				
				groupDetails[gid]->pending.push_back(userId);
				msg ="Join request for "+gid+" sent";
				//cout << "UserID: "<<userId<<endl;	
				//auto temp = groupDetails[gid];
				//for(auto i:temp)
				//	cout << i<<endl;
			}
			else
				msg =  "User not logged in";
			auto sd = send(req->socket,(char *)msg.c_str(),strlen(msg.c_str()),0);
		}
		else if(strcmp(temp[0],"leave_group")==0)
		{
			if(loggedIn)
			{
				string gid(temp[1]);
				//struct group *gp = (struct group *)malloc(sizeof(struct group));
				//gp->gid = gid;
				//gp->uid = userId;
				//vector<string> temp;
				//temp.push_back(userId);
				
				//groupDetails[gid]->connected.erase(userId);
				//auto vec = groupDetails[gid]->pending;
				groupDetails[gid]->active.erase(find(groupDetails[gid]->pending.begin(),groupDetails[gid]->pending.end(),userId));
				msg =  userId+" left "+gid;
				//cout << "UserID: "<<userId<<endl;	
				//auto temp = groupDetails[gid];
				//for(auto i:temp)
				//	cout << i<<endl;
			}
			else
				msg= "User not logged in";
			auto sd = send(req->socket,(char *)msg.c_str(),strlen(msg.c_str()),0);
		}
		else if(strcmp(temp[0],"list_requests")==0)
		{	string list="",str="";
			if(loggedIn)
			{
				string gid(temp[1]);
				//auto vec = groupDetails[gid]->pending;
				if(groupDetails.find(gid)!=groupDetails.end() && groupDetails[gid]->uid == userId)
				{
					auto temp = groupDetails[gid]->pending;
					for(auto it:temp)
					{
						str=str+it+',';	
					}
					list = str.substr(0,str.size()-1);
					msg=list;
				}
				else if(groupDetails.find(gid)!=groupDetails.end() && groupDetails[gid]->uid != userId)
				{
					msg = "Not owner of group";	
				}
				else
					msg= "Invalid group";
			}
			else
				msg = "User not logged in";
		 	auto sd = send(req->socket,(char *)msg.c_str(),strlen(msg.c_str()),0);
		}
		else if(strcmp(temp[0],"accept_request")==0)
		{	
			if(loggedIn)
			{
				string gid(temp[1]);
				string uid(temp[2]);
				if(groupDetails.find(gid)!=groupDetails.end() && groupDetails[gid]->uid == userId)
				{
					
					groupDetails[gid]->pending.erase(find(groupDetails[gid]->pending.begin(),groupDetails[gid]->pending.end(),uid));
					groupDetails[gid]->active.push_back(uid);
					msg="joining request accepted";
				}
				else if(groupDetails.find(gid)!=groupDetails.end() && groupDetails[gid]->uid != userId)
				{
					msg ="Not owner of group";	
				}
				else
					msg= "Invalid group";
			}
			else
				msg = "User not logged in";
			auto sd = send(req->socket,(char *)msg.c_str(),strlen(msg.c_str()),0);
		}
		else if(strcmp(temp[0],"list_groups")==0)
		{	
			string str="",list="";
			if(loggedIn)
			{
				for(auto i=groupDetails.begin();i!=groupDetails.end();i++)
				{
					str = str + i->first+',';		
				}
				list = str.substr(0,str.size()-1);
				msg =  list;
			}
			else
				msg = "User not logged in";
			auto sd = send(req->socket,(char *)msg.c_str(),strlen(msg.c_str()),0);
		}
		else if(strcmp(temp[0],"logout")==0)
		{
				
		}
		else if(strcmp(temp[0],"upload_file")==0)
		{	//cout <<"Tracker Upload_file condition"<<endl;
			
			string path(temp[1]);
			string gid(temp[2]);
			if(loggedIn)
			{	
				if(isGroupValid(gid) && isUserInGroup(gid,userId))
				{
					char *token[30];
					int tokenCount = tokenizer(temp[1],token,"/");
					char fileName[100];
					strcpy(fileName,token[tokenCount-1]);
					string fname(fileName);
					int sock = 0, valread; 
					struct sockaddr_in serv_addr; 
					char buffer[1024];
					if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
					{
						printf("\n Socket creation error \n"); 
						exit(1); 
					} 
					
					serv_addr.sin_family = AF_INET; 
					
					serv_addr.sin_port = htons(clientInfo[userId].second); //Hardcoded port # of Client listener removed
					serv_addr.sin_addr.s_addr = INADDR_ANY;

					if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
					{ 
						printf("\nConnection Failed \n"); 
						exit(1);
					}
					string sendmsg ="upload-"+path;
					//cout << sendmsg;
					int sd = send(sock,sendmsg.c_str(),strlen(sendmsg.c_str()),0);
					char buff[100];
					//cout <<"tracker sent bytes"<<sd<<endl;
					int rc = recv(sock,buff,sizeof(buff),0);
					//cout <<"tracker recvd bytes"<<rc<<endl;
					printf("%s\n",buff);
					char *tok[30];
					auto unused = tokenizer(buff,tok,"-");
					string hash(tok[1]);
					string siz(tok[0]);
					//string pp(tok[2]);
					//int peerPort = stoi(pp)
					size_t sz= stoi(siz);
					bool flag=false;
					//map<string,vector<pair<string,struct fileInfo> > > fileGroupInfo;
					for(auto it = fileGroupInfo[gid].begin();it!=fileGroupInfo[gid].end();it++)
					{//map<string,vector<pair<string,struct fileInfo *> > > fileGroupInfo;
						if(it->first == fname)
						{
							flag=true;
							it->second->users.push_back(userId);
							break;
						}
					}
					if(!flag)
					{
						struct fileInfo *fp = (struct fileInfo *)malloc(sizeof(struct fileInfo));
						fp->path = path;
						fp->hash = hash;
						fp->owner = userId;
						fp->sz = sz;

						fp->users.push_back(userId);
						vector<pair<string,struct fileInfo *> > vec(fileGroupInfo[gid].begin(),fileGroupInfo[gid].end());
						vec.push_back({fname,fp});
						fileGroupInfo[gid]=vec;
					}
					//init();
					msg="File info received by tracker";
				}
				else
					msg="Either group doesn't exist or the user not part of group";
			}
			else
				msg= "User not logged in";
			auto sd = send(req->socket,(char *)msg.c_str(),strlen(msg.c_str()),0);
		}
		else if(strcmp(temp[0],"download_file")==0)
		{
			string gid(temp[1]);
			string fname(temp[2]);
			string destPath(temp[3]);
			
			if(loggedIn)
			{
				if(isGroupValid(gid) && isUserInGroup(gid,userId))
				{
					for(auto it = fileGroupInfo[gid].begin();it!=fileGroupInfo[gid].end();it++)
					{//map<string,vector<pair<string,struct fileInfo *> > > fileGroupInfo;
						if(it->first == fname)
						{
							string str ="download-"+fname+"-"+destPath+"-";
							for(auto x=it->second->users.begin();x!=it->second->users.end();x++)
							{
								str = str + clientInfo[*x].first+":"+to_string(clientInfo[*x].second)+"-";	
							}
							
							msg = str.substr(0,str.size()-1);
							break;
						}
					}
					
				}
				else
					msg="Either group doesn't exist or the user not part of group";
			}
			else
				msg= "User not logged in";
			auto sd = send(req->socket,(char *)msg.c_str(),strlen(msg.c_str()),0);
		}
		memset(req->msg,0,sizeof(req->msg));
		//auto sd = send(req->socket,(char *)msg.c_str(),strlen(msg.c_str()),0);
		//just to maintain the cyclic structure using req->msg.
		recv(req->socket,(char *)req->msg,sizeof(req->msg),0);
		
	}
}
void *server(void *arg)
{
	int server_socket = socket(AF_INET,SOCK_STREAM,0);
	if(server_socket < 0)
		cout << "Socket Error";
	struct sockaddr_in server_address;
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(PORT);
	server_address.sin_addr.s_addr = INADDR_ANY;
	auto b = bind(server_socket,(struct sockaddr *) &server_address,sizeof(server_address));
	if(b<0)cout << "bind error";
	auto listen_status = listen(server_socket,2);
	if(listen_status<0) cout << "listening error";
	int client_socket=0;
	pthread_t thread_id[5];int i=0;
	while((client_socket = accept(server_socket,NULL,NULL))>0)
	{	
		
		char buff[100];
		int rc = recv(client_socket,(char *)buff,sizeof(buff),0);
		//printf("%s\n",buff);
		struct thread_args *client_req= (struct thread_args *)malloc (sizeof (struct thread_args));
		client_req->socket = client_socket;
		//cout << client_req->socket;
		strcpy(client_req->msg , buff);
		//cout << client_req->msg;
		//cout <<strlen(buff)<<endl;
		pthread_create(&thread_id[i++], NULL,handler,client_req);
		if(i==5 || i==6)
			pthread_join(thread_id[i], NULL);
	}
}
int main()
{

	pthread_t t1,t2;
	pthread_create(&t1, NULL, server, NULL);
	pthread_join(t1, NULL); 
	//pthread_create(&t2, NULL, client, NULL);
	return 0;
}
