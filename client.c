/*******************************************************************************
Base Code : Provided by Prof 
Author :Chinmay Shah 
File :dfsclient.c
Last Edit : 11/1

File for implementation of a Distrubuted File Client 
******************************************************************************/

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/time.h>
#include <stdlib.h>
#include <memory.h>
#include <errno.h>
#include <openssl/md5.h>
#include <sys/time.h>
#include <sys/time.h>


#define MAXBUFSIZE 60000
#define MAXPACKSIZE 1000
#define MAXCOMMANDSIZE 100
#define PACKETNO 7
#define SIZEMESSAGE 7
#define MAXFRAMESIZE (PACKETNO+SIZEMESSAGE+MAXPACKSIZE)
#define ACKMESSAGE 4+PACKETNO
#define MAXREPEATCOUNT 3
#define RELIABILITY
#define ERRMESSAGE 20

//#define NON_BLOCKING

int nbytes;                             // number of bytes send by sendto()
int sock;                               //this will be our socket
char buffer[MAXBUFSIZE];

struct sockaddr_in remote;              //"Internet socket address structure"
struct sockaddr *remoteaddr;
//struct hostent *server;
struct sockaddr_in from_addr;
int addr_length = sizeof(struct sockaddr);
char ack_message[ACKMESSAGE];

typedef char type2D[10][MAXCOMMANDSIZE];



typedef enum TYPEACK{NOACK,ACK,TIMEDACK}ACK_TYPE;


//Time set for Time out 
struct timeval timeout={0,0};     
/* You will have to modify the program below */
#define LISTENQ 1000 
#define SERV_PORT 3000

#define MAXCOLSIZE 100
#define HTTPREQ 	30

#define MAXDFSCOUNT 4
#define MAXBUFSIZE 60000
#define MAXPACKSIZE 1000
#define ERRORMSGSIZE 1000
#define MAXCOMMANDSIZE 100
#define MAXCONTENTSUPPORT 15





#define DEBUGLEVEL

#ifdef DEBUGLEVEL
	#define DEBUG 1
#else
	#define	DEBUG 0
#endif

#define DEBUG_PRINT(fmt, args...) \
        do { if (DEBUG) fprintf(stderr, "\n %s:%d:%s(): " fmt, __FILE__, \
                                __LINE__, __FUNCTION__, ##args); } while (0)





typedef char type2D[10][MAXCOMMANDSIZE];

typedef enum HTTPFORMAT{
							HttpExtra,//Extra Character
							HttpMethod,//Resource Method
							HttpURL,// Resource URL
							HttpVersion //Resource Version 
						}HTTP_FM;// Resource format

//HTTP Mehthod supported 
typedef enum HTTPMETHOD{
							HTTP_GET,//GET
							HTTP_POST

						}HTTP_METHOD;

//For configuration File

typedef enum CONFIGFORMAT{
							Extra,//Format Extra Character
							ConfigType,//Config Type 
							ConfigContent,//Config Content
							ConfigFileType,//Config File type 
							ConfigAddData //additional data 
						}CONFIG_FM;// Config File format

//For IP:PORT

typedef enum IPFORMAT{
							FmtExtra,//Format Extra Character
							IP,//IP
							PORT,//Port
						}IP_FM;// IP File format						


struct configClient{
		char DFSName[MAXDFSCOUNT][MAXCOLSIZE];
		char DFSIP[MAXDFSCOUNT][MAXCOLSIZE];
		char DFSPortNo[MAXDFSCOUNT][MAXCOLSIZE];
		char DFCUsername[MAXCOLSIZE];
		char DFCPassword[MAXCOLSIZE];
 };


struct configClient config;
int maxtypesupported=0; //typedef enum HTTPFORMAT{RM,RU,RV}HTTP_FM;


int server_sock,client_sock;                           //This will be our socket
struct sockaddr_in server, client;     //"Internet socket address structure"
unsigned int remote_length;         //length of the sockaddr_in structure
int nbytes;                        //number of bytes we receive in our message




//fixed Root , take from configuration file 
//char * ROOT = "/home/chinmay/Desktop/5273/PA2/www";
char *configfilename ="dfc.conf"; // take input name 

/*******************************************************************************************
//Parse a configutation file 
//
I/p : File name 

Checks : 
		1) Config File present or not
		2) Discard Commented out lines "#"
		3) Discard Blank Lines(Not ewotking)
		4) 
			a) Identify the DFS server IP and PORT number 
			b) Identify Username and Password 

 	  
o/p : Structure of data for configuration file 

Basic Start refernece for coding 
https://www.pacificsimplicity.ca/blog/simple-read-configuration-file-struct-example

Format :
*********************************************************************************************/
int config_parse(char Filename[MAXCOLSIZE]){

	int i=0 ;
	FILE *filepointer;
	ssize_t read_bytes,length;
	//struct ConfigData config;
	char readline[MAXBUFSIZE];
	char (*split_attr)[MAXCOLSIZE];
	char tempcopy[MAXCOLSIZE];
	int content_location=0,total_attr_commands=0;
	
	DEBUG_PRINT("In");
	//Read File 
	//FILE *filetoread = fopen(Filename,"r");
	if ((filepointer=fopen(Filename,"r"))==NULL){//if File  not found 
			printf("Configuration file not found Try Again \n");
			//perror("File not Found");
			exit(-1);
		}
	else
	{
		while((fgets(readline,sizeof(readline),filepointer))!=NULL){//read line by line 			

			readline[strlen(readline)-1] = '\0';
			//check for comments 			
			if (readline[0]=='#'){//check for commented out lines
				DEBUG_PRINT("comment");
			}
			else if (!strcmp(readline,"\n\r")){// ignore the blank Line 
				DEBUG_PRINT("Blank Line ");
			}	
			else
			{
				DEBUG_PRINT("%s\n",readline);	
				//parse and store file 

				if ((split_attr=malloc(sizeof(split_attr)*MAXCOLSIZE))){	
					total_attr_commands=0;
					if((total_attr_commands=splitString(readline," ",split_attr,5))<0)
					{
						DEBUG_PRINT("Error in Split \n\r");				
					}
					else
					{
						DEBUG_PRINT("%d",total_attr_commands);
						DEBUG_PRINT("Config Type %s",split_attr[ConfigType]);
						//split_attr[ConfigFileType][sizeof(ConfigType)]='\0';
						length=strlen(split_attr[ConfigType]);
	
						//strncpy(tempcopy,split_attr[ConfigType],length);
						//DEBUG_PRINT("Copied%s",tempcopy);
						////Check for user
						if(split_attr[ConfigType]!=NULL)
						{
							////Check for user
							if(!(strncmp(split_attr[ConfigType],"Username",length))){							
								
								bzero(config.DFCUsername,sizeof(config.DFCUsername));
								strcpy(config.DFCUsername,split_attr[ConfigContent]);
								DEBUG_PRINT("Found Username %s => %s",split_attr[ConfigContent],config.DFCUsername);
							}
							else
							{
								DEBUG_PRINT("Username not found ");
							}
														
							////Check for Password for user
							if(!(strncmp(split_attr[ConfigType],"Password",length))){
					
								bzero(config.DFCPassword,sizeof(config.DFCPassword));
								strcpy(config.DFCPassword,split_attr[ConfigContent]);
								DEBUG_PRINT("Found DFCPassword %s %s",split_attr[ConfigContent],config.DFCPassword);
							}
							else
							{
								DEBUG_PRINT("Password not found ");
							}

							////Check for Document Index
							/*
							if(!(strncmp(split_attr[ConfigType],"DirectoryIndex",length))){

								
								//);
								bzero(config.directory_index,sizeof(config.directory_index));
								strcpy(config.directory_index,split_attr[ConfigContent]);
								DEBUG_PRINT("Found DirectoryIndex %s %s",split_attr[ConfigContent],config.directory_index);
							}
							else
							{
								DEBUG_PRINT("DirectoryIndex not found ");
							}
							*/
							
							////Check for DFS Servers Configuration 
							if(!(strncmp(split_attr[ConfigType],"Server",length))){//if server configuration										
								
								DEBUG_PRINT("Found Server %s",split_attr[ConfigContent]);
								
								//DFS name
								bzero(config.DFSName[content_location],sizeof(config.DFSName[ConfigContent]));
								strncpy(config.DFSName[content_location],split_attr[ConfigContent],sizeof(split_attr[ConfigContent]));
								

								DEBUG_PRINT("Found IP:PORT %s",split_attr[ConfigFileType]);
								//split IP and Port "IP:PORT"
								//if ((split_ip=malloc(sizeof(split_attr)*MAXCOLSIZE))){	
								//total_attr_commands=0;
								DEBUG_PRINT("Found IP:PORT => Split them");
								if((total_attr_commands=splitString(split_attr[ConfigFileType],":",split_attr,3))<0)
								{
									DEBUG_PRINT("Error in Split \n\r");				
								}
								else
								{
									DEBUG_PRINT("%d",total_attr_commands);
									
									//split_attr[ConfigFileType][sizeof(ConfigType)]='\0';
									length=strlen(split_attr[IP]);
									////Check for IP
									if(split_attr[IP]!=NULL)
									{

										bzero(config.DFSIP[content_location],sizeof(config.DFSIP[content_location]));
										bzero(config.DFSPortNo[content_location],sizeof(config.DFSPortNo[content_location]));
										strncpy(config.DFSIP[content_location],split_attr[IP],sizeof(split_attr[IP]));
										strncpy(config.DFSPortNo[content_location],split_attr[PORT],sizeof(config.DFSPortNo[PORT]));
										DEBUG_PRINT("IP %s",split_attr[IP]);
										DEBUG_PRINT("PORT %s",split_attr[PORT]);
										content_location ++;
									}	
								//DEBUG_PRINT("Stored Server %s %s %d",split_attr[ConfigContent],split_attr[ConfigFileType],content_location);
								//DEBUG_PRINT("Found Server %s %d",split_attr[ConfigContent],content_location);
								//DEBUG_PRINT("Stored Server %s %d",split_attr[ConfigContent],content_location);
								}
							}		
							else
							{
								DEBUG_PRINT("Server not found ");
							}

						}	
					
					}
				}					

				else
				{
					DEBUG_PRINT("Cant Allocate Memory");
				}	
			}
		}	
		if (split_attr!=NULL){


			//free alloaction of memory 
			for(i=0;i<total_attr_commands;i++){
				free((*split_attr)[i]);
			}
			free(split_attr);//clear  the request recieved 

		}
		else{

			DEBUG_PRINT("Configuration Details could not be found ");

		}
		DEBUG_PRINT("AFter reading File ");
		fclose (filepointer);
		DEBUG_PRINT("Close File Pointer");
	}

	return (content_location);//return total content type 

}

/*************************************************************
//Split string on basis of delimiter 
//Assumtion is string is ended by a null character
I/p : splitip - Input string to be parsed 
	  delimiter - delimiter used for parsing 
o/p : splitop - Parsed 2 D array of strings
	  return number of strings parsed 

Referred as previous code limits number of strings parsed 	  
http://stackoverflow.com/questions/20174965/split-a-string-and-store-into-an-array-of-strings
**************************************************************/
int splitString(char *splitip,char *delimiter,char (*splitop)[MAXCOLSIZE],int maxattr)
{
	int sizeofip=1,i=1;
	char *p=NULL;//token
	char *temp_str = NULL;


	DEBUG_PRINT("value split %d",sizeofip);
	
	if(splitip==NULL || delimiter==NULL){
		printf("Error\n");
		return -1;//return -1 on error 
	}
	
	
	p=strtok(splitip,delimiter);//first token string 
	
	//Check other token
	while(p!=NULL && p!='\n' && sizeofip<maxattr )
	{
		
		
		temp_str = realloc(*splitop,sizeof(char *)*(sizeofip +1));
		
		if(temp_str == NULL){//if reallocation failed	

			
			//as previous failed , need to free already allocated 
			if(*splitop !=NULL ){
				for (i=0;i<sizeofip;i++)
					free(splitop[i]);
				free(*splitop);	
			}

			return -1;//return -1 on error 
		}
		
		
		//Token Used
		strcat(p,"\0");
		// Set the split o/p pointer 
		//splitop[0] = temp_str;

		//allocate size of each string 
		//copy the token tp each string 
		//bzero(splitop[sizeofip],strlen(p));
		memset(splitop[sizeofip],0,sizeof(splitop[sizeofip]));
		strncpy(splitop[sizeofip],p,strlen(p));
		strcat(splitop[sizeofip],"\0");
		DEBUG_PRINT	("%d : %s",sizeofip,splitop[sizeofip]);
		sizeofip++;

		//get next token 
		p=strtok(NULL,delimiter);
		
	}

	
	//if (sizeofip<maxattr || sizeofip>maxattr){
	if (sizeofip>maxattr+1){
		DEBUG_PRINT("unsuccessful split %d %d",sizeofip,maxattr);
		return -1;
	}	
	else
	{	
		//DEBUG_PRINT("successful split %d %d",sizeofip,maxattr);
		return sizeofip;//Done split and return successful }
	}	
		

	return sizeofip;	

	
}

/*************************************************************
Send message from  Client to Server
input 	
		msg   - String to be transmitted
		bytes -No of bytes transferred 
		Type  - Wait for ACK or Not 
*************************************************************/
int sendtoServer(char *msg,ssize_t bytes,ACK_TYPE type_ack)
{
	ssize_t send_bytes,recv_bytes;
	int rcvd_packno=0;
	char ack[ACKMESSAGE];
	//printf("send to Server %s , %d\n",msg,(int)bytes );

	//Send to Server 
	if ((send_bytes=sendto(sock,msg,bytes,0,remoteaddr,sizeof(remote))) < bytes)
	{
		fprintf(stderr,"Error in sending to clinet in sendtoClient %s %d \n",strerror(errno),(int)send_bytes );
	}


	if(type_ack==ACK){	//If Type is ACK
		if(recv_bytes = recvfrom(sock,ack,sizeof(ack),0,(struct sockaddr*)&remote,&addr_length)){
					//printf("ACK packet not found \n");
		}//If  recv didnt have an error
		else{
			//printf("%s\n",ack );			
			if (!strstr(ack_message,ack))
			{
				rcvd_packno=strtol(&ack[3],NULL,10);	//Parse the ack no rcvd 
			}
			else//ACK not present in packet 
			{
				//printf("ACK not recieved %s\n",ack);		//ACK not recieved 
				return -1;
			}		
		}
	}	
	return rcvd_packno	;
}

//Receive List from Server and Print it
void listRcv(){
	// Blocks till bytes are received
		
		bzero(buffer,sizeof(buffer));
		#ifdef NON_BLOCKING
			if(nbytes = recvfrom(sock,buffer,sizeof(buffer),0,(struct sockaddr*)&remote,&addr_length)<0){//recv from server and check for non-blocking 
				fprintf(stderr,"non-blocking socket not returning data  List %s\n",strerror(errno) );
			}
		#else
			nbytes = recvfrom(sock,buffer,sizeof(buffer),0,(struct sockaddr*)&remote,&addr_length);
		#endif	
		printf("%s\n", buffer);//Print the list 
}


/*************************************************************************************
Calculate MD5 and return value as string 
Assumtion : File is in same directory 
i/p - Filename
o/p - MD5 Hash value 
Ref for understanding :http://stackoverflow.com/questions/10324611/how-to-calculate-the-md5-hash-of-a-large-file-in-c?newreg=957f1b2b2132420fb1b4783484823624
Library :http://stackoverflow.com/questions/14295980/md5-reference-error
		gcc client.c -o client -lcrypto -lssl

***************************************************************************************/
int MD5Cal(char *filename, char *MD5_result)
{
	//unsigned char *MD5_gen;
	unsigned char MD5_gen[MD5_DIGEST_LENGTH];
	MD5_CTX mdCont;//
	int file_bytes;//bytes read
	unsigned char tempdata[10];//store temp data from file 
	bzero(tempdata,sizeof(tempdata));
	unsigned char temp;
	int i=0;

	FILE *fd = fopen(filename,"rb");
	if (fd ==NULL)
	{
		perror(filename);//if can't open
		return -1;
	}

	MD5_Init(&mdCont);//Initialize 
	
	while((file_bytes = fread(tempdata,1,10,fd)) != 0){//Read data from files
		MD5_Update(&mdCont,tempdata,file_bytes);//Convert and update context 		
	}	
	MD5_Final(MD5_gen,&mdCont);
	fclose(fd);	

	for ( i=0;i<MD5_DIGEST_LENGTH;i++) {
		temp = MD5_gen[i];
		sprintf(MD5_result,"%x",((MD5_gen[i]&0xF0)>>4) );
		*MD5_result++;
		sprintf(MD5_result,"%x",((MD5_gen[i]&0x0F)) );
		*MD5_result++;
		
	   }
	return 0;
}


//Function for Reciving packet - Not Utilized 

ssize_t recv_packet(char *recv_pack,char size_bytes)
{
	ssize_t bytes;
	bytes=recvfrom(sock,recv_pack,size_bytes,0,(struct sockaddr*)&remote,&addr_length);
	return bytes;
}
/****************************************************************************************************
// Get a file from Server to Client

I/p : filename - File name to be store data in  to client 

O/p :file is rcvd from server
	 return number of status

Reference for understanding basic udp send:
https://lms.ksu.edu.sa/bbcswebdav/users/mdahshan/Courses/CEN463/Course-Notes/07-file_transfer_ex.pdf	  
*****************************************************************************************************/
int rcvFile (char *filename){
	
	int fd; // File decsriptor 
	ssize_t file_bytes=0;
	int file_size=0;
	int rcount=0;//count for send count 

	
	char *err_indication = "Error"; 
	char *comp_indication = "Comp"; 
	char recv_pack[MAXPACKSIZE+1];
	char packet_no[PACKETNO];
	char prev_pack_no[PACKETNO];
	char rcvd_packet_no[PACKETNO];
	char rcvd_size[SIZEMESSAGE];
	char recv_frame[MAXFRAMESIZE];
	int  rcvd_bytes;
	int  pos_data=0;
	int  pos_size=0;
	//int  prev_pack_no=0;
	char *MD5_Client,*MD5_Server;

	//intial value of 
	strcpy(rcvd_packet_no,"0");

	//allocate MD5 
	MD5_Client =(char *)malloc(MD5_DIGEST_LENGTH*2*sizeof(char));
	if (MD5_Client == NULL)
	{
		perror(MD5_Client);
		return -1;

	}
	MD5_Server =(char*)malloc(MD5_DIGEST_LENGTH*2*sizeof(char));
	if (MD5_Server == NULL)
	{
		perror(MD5_Server);
		return -1;
	}
	//clear Md5
	bzero(MD5_Client,sizeof(MD5_Client));
	bzero(MD5_Server,sizeof(MD5_Server));


	//Recieve first packet 	
	file_bytes = recvfrom(sock,recv_frame,sizeof(recv_frame),0,(struct sockaddr*)&remote,&addr_length);


	
	if (!strchr(recv_frame,'|'))//Check if Packet is file data or normal 
	{
		//printf("Cant find | before loop \n");
		strcpy(recv_pack,recv_frame);
	}

	


	//open the file
	if (strcmp(recv_pack,err_indication)==0 && file_bytes >0 ){//check for error Message from server
				file_bytes = recvfrom(sock,recv_pack,sizeof(recv_pack),0,(struct sockaddr*)&remote,&addr_length);
				printf("\n%s\n",recv_pack );
				rcount =-1;
				//return -1;
			}
	else if((fd= open(filename,O_CREAT|O_RDWR|O_TRUNC,0666)) <0){//open a file to store the data in file
		perror(filename);//if can't open
		return -1;
	}
	else // open file successfully ,read file 
	{
		//debug 
		//printf("Rcvx %s\n",filename);
		//
		//check if any bytes read 
		do
		{
			//printf("Complete Frame %s\n",recv_frame);
			//split packet no and data 
			if (strchr(recv_frame,'|')){
				//extract packet No
				pos_size =(int)(strchr(recv_frame,'|') - recv_frame)+1;//posintion of '|'
				
				memcpy(rcvd_packet_no,recv_frame,pos_size-1);//packet of data 
				rcvd_packet_no[pos_size+1]='\0';
				
				//extract Size of Packet 
				pos_data =(int)(strchr(&recv_frame[pos_size],'|') - recv_frame)+1;
				memcpy(rcvd_size,&recv_frame[pos_size],pos_data-(pos_size));
				rcvd_size[pos_data+1]='\0';
				
				rcvd_bytes=strtol(rcvd_size,NULL,10);

				//extract File Data
				//printf("%s\n",recv_frame);
				memcpy(recv_pack,&recv_frame[pos_data],rcvd_bytes);//packet of data 
				recv_pack[rcvd_bytes]='\0';

				//printf("pos size,pos data %d %d %d\n",pos_size,pos_data,(int)sizeof(recv_frame));
				//printf("From Server Packet:Size  %s %s\n",rcvd_packet_no,rcvd_size);	
				//printf("%s\n",recv_pack );
				//Change the filebytes 
				//break;

			}
			else
			{
				//printf("Cant find | \n");
				strcpy(recv_pack,recv_frame);
			}
			
			if (strcmp(recv_pack,comp_indication)==0)//check for File Completion
			{
				//printf("\n%s\n",recv_pack );
				break;
			}
			
		#ifndef RELIABILITY
			else if(write(fd,recv_pack,rcvd_bytes)<0){//write to file , with strlen as sizeof has larger value 
						perror("Error for writing to file");
						rcount =-1;	
				}
			else// when write has be done
			{	
					//printf("packetno:bytes:written bytes %d:%d:%d\n",rcount,(int)file_bytes,rcvd_bytes );
					file_size = file_size + rcvd_bytes;
					strcmp(prev_pack_no,packet_no);		
		        	rcount++;//recvd packet counter
		        	//FOmrating ACK|PacketNO
					bzero(ack_message,sizeof(ack_message));
					//sprintf(packet_no,"%d",(int)strtol(rcvd_packet_no,NULL,10));
					strcpy(ack_message,"ACK");
					strcat(ack_message,rcvd_packet_no);
					sendtoServer(ack_message,sizeof(ack_message),NOACK);

			}
	    #else
	      
			else if (!strcmp(prev_pack_no,rcvd_packet_no)){//check if packet was already recieved before and ACK was missed 
					//strcat(ack_message,sprintf(packet_no,"%d",);
					strcpy(ack_message,"ACK");
					strcat(ack_message,prev_pack_no);
					//printf("Resending ACK %s\n",ack_message );
					sendtoServer(ack_message,sizeof(ack_message),NOACK);	
			}	
			else{//if previous packet no is not same as present 
			
				if(write(fd,recv_pack,rcvd_bytes)<0){//write to file , with strlen as sizeof has larger value 
					perror("Error for writing to file");
					rcount =-1;	
				}
				else// when write has be done
				{
				
					//printf("packetno:bytes:written bytes %d:%d:%d\n",rcount,(int)file_bytes,rcvd_bytes );
					file_size = file_size + rcvd_bytes;
					
		        	//printf("packet: \n%s\n",recv_pack);	
		        	rcount++;//recvd packet counter
		        	//FOmrating ACK|PacketNO
					bzero(ack_message,sizeof(ack_message));
					strcpy(ack_message,"ACK");
					//sprintf(packet_no,"%d",rcount);
					sprintf(packet_no,"%d",(int)strtol(rcvd_packet_no,NULL,10));
					//printf("%s\n",ack_message);
					strcat(ack_message,packet_no);
					sendtoServer(ack_message,sizeof(ack_message),NOACK);
					strcpy(prev_pack_no,rcvd_packet_no);

		        }
	        
			}
		#endif							
			//clear packets for avoiding extra last packet
			
			file_bytes =0 ;
			pos_data =0;
			bzero(recv_pack,sizeof(recv_pack));
			bzero(recv_frame,sizeof(recv_frame));
		}while(((file_bytes = recvfrom(sock,recv_frame,sizeof(recv_frame),0,(struct sockaddr*)&remote,&addr_length))) >0);//read file 
		//while (file_bytes=recv_packet(recv_frame,sizeof(recv_frame))>0);


		if (rcount >0){				
				printf("Rcvxd packet: total Bytes %d:%d\n",rcount,(int )file_size);	
		}	
		// Close file	
		if(close(fd) <0)//check if file closed 
		{
			perror(filename);
			rcount =-1;
		}					
		else
		{
			//printf("%s closed successfully\n",filename );
		}
		//Calculate for file at Client end 
		

		//Receive MD5 value from Server,//Wait for MD5 hash value 
		if(file_bytes = recvfrom(sock,MD5_Server,MD5_DIGEST_LENGTH*2,0,(struct sockaddr*)&remote,&addr_length)>0){
			MD5Cal(filename,MD5_Client);
			//printf("Client: %s\n Server: %s\n",MD5_Client,MD5_Server);
			//Compare 
			if(!strcmp(MD5_Client,MD5_Server))
			{
				printf("Files Recieved match with Server \n" );	
			}
			else
			{
				printf("Retry ,Files dont match!!\n");		
			}
		}
		//Acknowledge  
	}
	//printf("Completed Rcvxd Routine\n");
	//reinitialize 
	free(MD5_Client);
	free(MD5_Server);
	
	bzero(recv_pack,sizeof(recv_pack));
	bzero(packet_no,sizeof(packet_no));
	bzero(prev_pack_no,sizeof(prev_pack_no));
	bzero(rcvd_packet_no,sizeof(rcvd_packet_no));
	bzero(rcvd_size,sizeof(rcvd_size));
	bzero(recv_frame,sizeof(recv_frame));
	rcvd_bytes=0;
	pos_data=0;
	pos_size=0;
	



	return rcount;
}


/*************************************************************
// Send a file from Client to Server
I/p : filename - File name to be pused to server 
o/p : find and file is pushed to server 
	  return number of status
Reference for understanding :
https://lms.ksu.edu.sa/bbcswebdav/users/mdahshan/Courses/CEN463/Course-Notes/07-file_transfer_ex.pdf	  
**************************************************************/
int sendFile (char *filename){
	
	int fd; // File decsriptor 
	ssize_t file_bytes,file_size;
	int scount=0;//count for send count 

	char *err_indication = "Error";
	char *comp_indication = "Comp";

	char err_message[ERRMESSAGE] = "File not Present"; 
	char *MD5_message;
	char MD5_temp[MD5_DIGEST_LENGTH*2];
	MD5_message = MD5_temp;
	char send_pack[MAXPACKSIZE];
	char send_frame[MAXFRAMESIZE];
	char send_size[SIZEMESSAGE];
	int temp_displace=PACKETNO+SIZEMESSAGE;
	int recvd_ack_no=0;
	int repeat_count=MAXREPEATCOUNT;
	int flags = fcntl(sock, F_GETFL);
	
	//open the file
	if((fd= open(filename,O_RDONLY)) <0 ){//open read only file 
		perror(filename);//if can't open
		//send to client the error 
		sendtoServer(err_indication,sizeof(err_indication),NOACK);
		sendtoServer(err_message,sizeof(err_message),NOACK);
		return -1;
	}
	else // open file successful ,read file 
	{
		//read bytes and send 
		while((file_bytes = read(fd,send_pack,MAXPACKSIZE)) != 0 ){//read data from file 			
			//Append packet no to packet --Frame -- PACKETNO|PACKETSIZE|PACKETDATA
			sprintf(send_frame,"%06d|%06d|",scount,(int)file_bytes);
			memcpy(send_frame+temp_displace,send_pack,file_bytes);//copy the send pack to send frame (** use memset as image files an issue)
			

			#ifndef RELIABILITY
				recvd_ack_no=sendtoServer(send_frame,sizeof(send_frame),ACK);	//send frame
			#else	
				//RELIABILITY 
				//Reset to UnBlocking 
				timeout.tv_sec = 0;
				timeout.tv_usec = 100000;

				if (setsockopt (sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,sizeof(timeout)) < 0){
	        		perror("setsockopt failed\n");
	    		}
	    		else
	    		{	//repeat for MAXREPEAT COUNT till acknowledge recieved 
	    			do
	    			{
		    			recvd_ack_no=sendtoServer(send_frame,sizeof(send_frame),ACK);	//send frame
		    
		    		}while (recvd_ack_no!=scount && repeat_count-- >=0);	
		    		if (recvd_ack_no != scount)//if even multiple tries failed 
		    		{
		    			//printf("try Again !!Files failing %d times\n",MAXREPEATCOUNT);
		    			repeat_count=MAXREPEATCOUNT;
		    		}

		    		else
		    		{	
			    		//printf("Send the packet in %d\n",repeat_count );
			    		repeat_count=MAXREPEATCOUNT;
			    		//recvd_ack_no=0;
			    	}	
	    		}
	    		//Reset to Blocking 
	    		timeout.tv_sec = 0;
				timeout.tv_usec = 0;
				if (setsockopt (sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,sizeof(timeout)) < 0){
	        		perror("setsockopt failed\n");
	    		}
				//Set it  blocking
				//fcntl(sock, F_SETFL, flags);

			#endif
			//update count 
			scount++;
			//clearing for new data
			file_bytes =0;
			bzero(send_pack,sizeof(send_pack));
			bzero(send_frame,sizeof(send_frame));
		}		
		close(fd);//close file if opened 
	}
	sendtoServer(comp_indication,(ssize_t)sizeof(comp_indication),NOACK);	//send Completion of File
	MD5Cal(filename,MD5_message);
	//printf("MD5 Message%s\n",MD5_message );
	//Send MD5 Hash value 
	sendtoServer(MD5_message,2*MD5_DIGEST_LENGTH,NOACK);

	
	//printf("No of packets send %d\n",scount );
	return scount;
}





/* You will have to modify the program below */


int main (int argc, char * argv[] ){
	char request[MAXPACKSIZE];             //a request to store our received message
	
	/*
	int *mult_sock=NULL;//to alloacte the client socket descriptor
	pthread_t client_thread;
	*/
	int i=0;
	
	//Configuration file before starting the Web Server 
	DEBUG_PRINT("Reading the config file ");
	

	maxtypesupported=config_parse("dfc.conf");

	// Print the Configuration 
	DEBUG_PRINT("Confiuration Obtain");
	
	//Check for file support available or not 
	if (!maxtypesupported)
	{
		printf("Zero DFS found !! Check Config File\n");
		exit(-1);
	}
	else
	{	DEBUG_PRINT("DFS Configured");
		for (i=0;i<maxtypesupported;i++)
		{
			//DEBUG_PRINT("%d %s %s",i,config.DFSIP[i],config.DFSPortNo[i]);
			DEBUG_PRINT("DFS IP:PORT %d %s - %s:%s",i,config.DFSName[i],config.DFSIP[i],config.DFSPortNo[i]);
		}
	}	
	/******************
	  This code populates the sockaddr_in struct with
	  the information about our socket
	 ******************/
	bzero(&server,sizeof(server));                    //zero the struct
	server.sin_family = AF_INET;                   //address family
	//server.sin_port = htons(atoi(argv[1]));        //htons() sets the port # to network byte order


	//Check if Port is present or not 
	/*
	if (strcmp(config.DFSPortNo,"")){
		DEBUG_PRINT("Port %s",config.DFSPortNo);
		if(atoi(config.DFSPortNo) <=1024){
			printf("\nPort Number less than 1024!! Check configuration file\n");
			exit(-1);
		}
		else
		{
			server.sin_port = htons(atoi(config.DFSPortNo));        		//htons() sets the port # to network byte order
		}	
		
	}
	else
	{	
		printf("\nPort Number not found !! Check configuration file");
		exit(-1);
	}
	
	server.sin_addr.s_addr = INADDR_ANY;           //supplies the IP address of the local machine
	remote_length = sizeof(struct sockaddr_in);    //size of client packet 

	*/
	//check if document root directory present 
	if (strcmp(config.DFCUsername,"")){
		DEBUG_PRINT("DFCUsername %s",config.DFCUsername);
	}
	else
	{	
		printf("\nDFCUsername  not found !! Check configuration file\n");
		exit(-1);
	}

	if (strcmp(config.DFCPassword,"")){
		DEBUG_PRINT("DFCPassword %s",config.DFCPassword);
	}
	else
	{	
		printf("Default File not found !! Check configuration file\n");
		//exit(-1);
	}

	//Keepalive for pipelining 
	/*
	DEBUG_PRINT("KeepaliveTime %s",config.keep_alive_time);
	
	//Causes the system to create a generic socket of type TCP (strean)
	if ((server_sock =socket(AF_INET,SOCK_STREAM,0)) < 0){
		DEBUG_PRINT("unable to create tcp socket");
		exit(-1);
	}
	*/
	/******************
	  Once we've created a socket, we must bind that socket to the 
	  local address and port we've supplied in the sockaddr_in struct
	 ******************/
	  /*
	if (bind(server_sock, (struct sockaddr *)&server, sizeof(server)) < 0){
		close(server_sock);
		printf("unable to bind socket\n");
		exit(-1);
	}
	//

	if (listen(server_sock,LISTENQ)<0)
	{
		close(server_sock);
		perror("LISTEN");
		exit(-1);
	}


	DEBUG_PRINT("Server is running wait for connections");

	//Accept incoming connections 
	while((client_sock = accept(server_sock,(struct sockaddr *) &client, (socklen_t *)&remote_length))){
		if(client_sock<0){	
			perror("accept  request failed");
			exit(-1);
			close(server_sock);
		}
		DEBUG_PRINT("connection accepted  %d \n",(int)client_sock);	
		mult_sock = (int *)malloc(1);
		if (mult_sock== NULL)//allocate a space of 1 
		{
			perror("Malloc mult_sock unsuccessful");
			close(server_sock);
			exit(-1);
		}
		DEBUG_PRINT("Malloc successfully\n");
		//bzero(mult_sock,sizeof(mult_sock));
		*mult_sock = client_sock;

		DEBUG_PRINT("connection accepted  %d \n",*mult_sock);	
		//Create the pthread 
		if ((pthread_create(&client_thread,NULL,client_connections,(void *)(*mult_sock)))<0){
			close(server_sock);
			perror("Thread not created");
			exit(-1);

		}		
		*/		
		/*
		//as it does  have to wait for it to join thread ,
		//does not allow multiple connections 
		if(pthread_join(client_thread, NULL) == 0)
		 printf("Client Thread done\n");
		else
		 perror("Client Thread");
		 */
	/*	
		free(mult_sock);
		DEBUG_PRINT("Freed");

	}	
	if (client_sock < 0)
	{
		perror("Accept Failure");
		close(server_sock);
		exit(-1);
	}
	close(server_sock);
	*/

}
		

