/*******************************************************************************
Base Code : Provided by Prof 
Author :Chinmay Shah 
File :server.c
Last Edit : 9/25
******************************************************************************/

#include <sys/types.h>
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
#include <string.h>
#include <dirent.h>
#include <openssl/md5.h>
#include <sys/time.h>
#include <pthread.h> // For threading , require change in Makefile -lpthread
//#include <time.h>

//#include <time.h>

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



//Time set for Time out 
struct timeval timeout={0,0};     


#define MAXCOLSIZE 100
#define HTTPREQ 	30

#define MAXDFSCOUNT 4
#define MAXBUFSIZE 60000
#define MAXPACKSIZE 1000
#define ERRORMSGSIZE 1000
#define MAXCOMMANDSIZE 100
#define MAXCONTENTSUPPORT 15

#define LISTENQ 1000 
#define SERV_PORT 3000


#define DEBUGLEVEL

#ifdef DEBUGLEVEL
	#define DEBUG 1
#else
	#define	DEBUG 0
#endif

#define DEBUG_PRINT(fmt, args...) \
        do { if (DEBUG) fprintf(stderr, "\n %s:%d:%s(): " fmt, __FILE__, \
                                __LINE__, __FUNCTION__, ##args); } while (0)



//For IP:PORT

typedef enum IPFORMAT{
							FmtExtra,//Format Extra Character
							IP,//IP
							PORT//Port
						}IP_FM;// IP File format						

//For configuration File

typedef enum CONFIGFORMAT{
							Extra,//Format Extra Character
							ConfigType,//Config Type 
							ConfigContent,//Config Content
							ConfigFileType,//Config File type 
							ConfigAddData //additional data 
						}CONFIG_FM;// Config File format

// For input command split 
typedef enum COMMANDLOCATION{
							CommandExtra,//Extra Character
							command_location,//Resource Method
							file_location,// Resource URL
							
						}COMMAND_LC;// Resource format

// For Server Client Packet request/data Format and location 
typedef enum PACKETLOCATION{
							requestExtra,//Extra Character
							DFCUserloc,//User Location
							DFCPassloc,//Password Location
							DFCCommandloc,//Command Location
							DFCFileloc,//Command Location
							DFCDataloc,//Data Location
							
						}PACKET_LC;// Resource format


//Request/command exchnage format b/w DFS and DFC

struct requestCommandFmt{		
		char DFCRequestUser[MAXCOLSIZE];
		char DFCRequestPass[MAXCOLSIZE];
		char DFCRequestCommand[MAXCOLSIZE];
		char DFCRequestFile[MAXCOLSIZE];
		int socket;
};

//Data exchnage format b/w DFS and DFC

struct DataFmt{		
		char DFCRequestUser[MAXCOLSIZE];
		char DFCRequestPass[MAXCOLSIZE];
		char DFCRequestCommand[MAXCOLSIZE];
		char DFCRequestFile[MAXCOLSIZE];
		char DFCData[MAXPACKSIZE];
		int socket;
};



//For IP:PORT

typedef enum CREDENTIALS{
							FmtCreExtra,//Format Extra Character
							DFC_username,//Client User name
							DFC_password,//Client Password
						}CREDENTIALS_FMT;// IP File format						


struct configServer{
		char DFCUsername[MAXDFSCOUNT][MAXCOLSIZE];
		char DFCPassword[MAXDFSCOUNT][MAXCOLSIZE];
		char listen_port[MAXCOLSIZE];
		char DFSdirectory[MAXCOLSIZE];
 };


struct configServer config;
int maxtypesupported=0; //typedef enum HTTPFORMAT{RM,RU,RV}HTTP_FM;


typedef char type2D[10][MAXCOMMANDSIZE];

typedef enum TYPEACK{NOACK,ACK,TIMEDACK}ACK_TYPE;


int sock;                           //This will be our socket
int server_sock,client_sock;                           //This will be our socket
struct sockaddr_in server, client;     //"Internet socket address structure"
unsigned int remote_length;         //length of the sockaddr_in structure
int nbytes;                        //number of bytes we receive in our message
char ack_message[ACKMESSAGE];

int *mult_sock=NULL;//to alloacte the client socket descriptor

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


/*******************************************************************************************
//Parse a configutation file for Client 
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
		#Comment
		#DFS Configured 
		Server <DFS Name> <IP>:<PORT>
		Username <user name>
		Password <Pass>
		#Comment

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
						////Check for DFC user name
						if(split_attr[ConfigType]!=NULL)
						{																		
								DEBUG_PRINT("Found Server %s",split_attr[ConfigType]);								
								//DFS name
								bzero(config.DFCUsername[content_location],sizeof(config.DFCUsername[DFC_username]));
								strncpy(config.DFCUsername[content_location],split_attr[DFC_username],sizeof(split_attr[DFC_username]));								
								
								DEBUG_PRINT("Found Password %s",split_attr[ConfigContent]);
								bzero(config.DFCPassword[content_location],sizeof(config.DFCPassword[DFC_password]));
								strncpy(config.DFCPassword[content_location],split_attr[DFC_password],sizeof(split_attr[DFC_password]));								
								content_location ++;
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




/*************************************************************************************
Calculate MD5 and return value as string 
Assumtion : File is in same directory 
i/p - Filename
o/p - MD5 Hash value 
Ref for coding :http://stackoverflow.com/questions/10324611/how-to-calculate-the-md5-hash-of-a-large-file-in-c?newreg=957f1b2b2132420fb1b4783484823624
Library :http://stackoverflow.com/questions/14295980/md5-reference-error
		gcc server.c -o server -lcrypto -lssl
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


/*************************************************************
Send message recieved to Client 
*************************************************************/

int sendtoClient(char *msg,ssize_t bytes,ACK_TYPE ack_type)
{
	ssize_t send_bytes,recv_bytes;
	int rcvd_packno=0;
	char ack[ACKMESSAGE];
	//printf("sendtoClient %s , %d\n",msg,(int)bytes );
	if((send_bytes=send(mult_sock,msg,strlen(msg),0))< bytes)
	//if ((send_bytes=sendto(sock,msg,bytes,0,(struct sockaddr*)&sin,remote_length)) < bytes)
	{
		fprintf(stderr,"Error in sending to clinet in sendtoClient %s %d \n",strerror(errno),(int)send_bytes );
	}
	//check for ACK
	/*
	if(ack_type==ACK){	

		//Wait for Acknowledege 	   
		if (recv_bytes = recvfrom(sock,ack,sizeof(ack),0,(struct sockaddr*)&sin,&remote_length)<0){
			//printf("ACK not recieved \n");//Not ACk Packet 
		}
		else{
				//printf("%s\n",ack );
			
			if (!strstr(ack_message,ack))
			{
				rcvd_packno=strtol(&ack[3],NULL,10);	//Parse the ack no rcvd 
			}
			else
			{
				//printf("ACK not recieved %s\n",ack);		//ACK not recieved 
				return -1;
			}		
		}
	}	
	*/
	return rcvd_packno	;

}
// To close socket and exit server
void exitServer(){
	printf("in exit Server\n");
	//close the socket 
	sendtoClient("ok",sizeof("ok"),NOACK);
	close(sock);
}

char comp_msg[MAXCOLSIZE]="COMP";
/*************************************************************
List the files on local directory of user and send all file parts 
details to server 

o/p : list of files in present directory posted to client 

Ref:http://www.lemoda.net/c/list-directory/
**************************************************************/

int list(char * directory,int listsock){
	DIR * d;
	//char * directory = "."; // by default present directory
	char  * listtosend=NULL ;
	//open directory 
	DEBUG_PRINT("In LIST => directory %s",directory);
	d = opendir (directory);
	if (!d){
		fprintf(stderr,"Error in opening directory %s\n",strerror(errno));
		//sendtoClient(strerror(errno),(ssize_t)sizeof(strerror(errno)),NOACK);
		return -1;
	}

	

	listtosend = (char *)calloc(MAXBUFSIZE,sizeof(char));
	//memset(listtosend,NULL,MAXBUFSIZE);
	
	if (listtosend <0 ){
		fprintf(stderr,"Error in allocating memory %s\n",strerror(errno));
		return -1;
	}
	bzero(listtosend,strlen(listtosend));
	DEBUG_PRINT("Check LIST(should be blank) send => %s",listtosend);
	while(1){
		bzero(listtosend,strlen(listtosend));
		struct  dirent *listfiles;
		//ref: http://nxmnpg.lemoda.net/3/readdir
		listfiles= readdir(d);
		if (!listfiles){
			break;
		}
		
		if(!(strcmp(listfiles->d_name,".") == 0 || strcmp(listfiles->d_name,"..") ==0)){//excluding present and previous dir
			DEBUG_PRINT("File %s",listfiles->d_name);
			strcat(listtosend,listfiles->d_name);
			//strcat(listtosend,"\t");
			DEBUG_PRINT("LIST concat %s",listtosend);
			DEBUG_PRINT(" directory=> %s send => %s,",directory,listtosend);
			//Send the files to client 
			//Rectify the 
			if((send(listsock,listtosend,strlen(listtosend),0))<0)		
			{
				fprintf(stderr,"Error in sending to client in list send %s\n",strerror(errno));
			}
		}
	}
	//close the directory 
	if(closedir(d)){
		fprintf(stderr,"Error in closing directory %s\n",strerror(errno));
		//sendtoClient(strerror(errno),(ssize_t)sizeof(strerror(errno)),NOACK);
		return -1;
	}
	DEBUG_PRINT("Send Completion Message => %s",comp_msg);
	
	if((send(listsock,comp_msg,strlen(comp_msg),0))<0)		
			{
				fprintf(stderr,"Error in sending to clinet in lsit send %s\n",strerror(errno));
			}
	DEBUG_PRINT(" socket => %d ",listsock);
	
	
	//sendtoClient(listtosend,(ssize_t)sizeof(listtosend));
	free(listtosend);
	return 0;//sucess 	
}

/*************************************************************
// Get a file from Client to Server

I/p : filename - File name to be store data in  to server 

O/p :file is rcvd from client
	 return number of status

Reference for  basic understanding :
https://lms.ksu.edu.sa/bbcswebdav/users/mdahshan/Courses/CEN463/Course-Notes/07-file_transfer_ex.pdf	  
**************************************************************/
/*
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
	char *MD5_Client,*MD5_Server;
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


	file_bytes =recvfrom(sock,recv_frame,sizeof(recv_frame),0,(struct sockaddr*)&sin,&remote_length);	
	//copy the data of file 
	
	if (strchr(recv_frame,'|')){
		//printf("file to write  %s\n",recv_pack );
	}
	else
	{
		//printf("Cant find | before loop \n");
		strcpy(recv_pack,recv_frame);
	}
	//open the file
	if (strcmp(recv_pack,err_indication)==0 && file_bytes >0 ){//check for error Message from server
				file_bytes =recvfrom(sock,recv_frame,sizeof(recv_frame),0,(struct sockaddr*)&sin,&remote_length);	
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
		//check if any bytes read 
		do
		{
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
				memcpy(recv_pack,&recv_frame[pos_data],rcvd_bytes);//packet of data 

				recv_pack[rcvd_bytes]='\0';

				//printf("pos size,pos data %d %d %d\n",pos_size,pos_data,(int)sizeof(recv_frame));
				//printf("From Client Packet:Size  %s %s\n",rcvd_packet_no,rcvd_size);	
				//printf("%s\n",recv_pack );	
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
					sendtoClient(ack_message,sizeof(ack_message),NOACK);

			}
	    #else
	      
			else if (!strcmp(prev_pack_no,rcvd_packet_no)){//check if packet was already recieved before and ACK was missed 
					//strcat(ack_message,sprintf(packet_no,"%d",);
					strcpy(ack_message,"ACK");
					strcat(ack_message,prev_pack_no);
					//printf("Resending ACK %s\n",ack_message );
					sendtoClient(ack_message,sizeof(ack_message),NOACK);	
			}	
			else{//if previous packet no is not same as present 
			
				if(write(fd,recv_pack,rcvd_bytes)<0){//write to file , with strlen as sizeof has larger value 
					perror("Error for writing to file");
					rcount =-1;	
				}
				else// when write has be done
				{
				
					//printf("packetno:bytes:written bytes %d:%d:%d\n",rcount,(int)file_bytes,rcvd_bytes );
					file_size = file_size + rcvd_bytes;//Calculate the complete file Size 
					
		        
		        	rcount++;//recvd packet counter
		        	
		        	//Fomrating ACK|PacketNO
					bzero(ack_message,sizeof(ack_message));
					strcpy(ack_message,"ACK");
					sprintf(packet_no,"%d",(int)strtol(rcvd_packet_no,NULL,10));
					strcat(ack_message,packet_no);
					sendtoClient(ack_message,sizeof(ack_message),NOACK);
					strcpy(prev_pack_no,rcvd_packet_no);

		        }
	        
			}
		#endif							
			
			file_bytes =0 ;
			pos_data =0;
			bzero(recv_pack,sizeof(recv_pack));
			bzero(recv_frame,sizeof(recv_frame));
		}while(((file_bytes =recvfrom(sock,recv_frame,sizeof(recv_frame),0,(struct sockaddr*)&sin,&remote_length))) >0);//read file 
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
		
		//Receive MD5 value from Server,//Wait for MD5 hash value 
		if(file_bytes =recvfrom(sock,MD5_Client,MD5_DIGEST_LENGTH*2,0,(struct sockaddr*)&sin,&remote_length)>0){
			MD5Cal(filename,MD5_Server);//Calculate for file at Client end 
			//printf("Client: %s\n Server: %s\n",MD5_Client,MD5_Server);
			//Compare 
			if(!strcmp(MD5_Client,MD5_Server))
			{
				printf("Recieved File match with Client \n" );	
			}
			else
			{
				printf("Retry ,Files dont match!!\n");		
			}
		}
		//Acknowledge  
	}
	//printf("Completed Rcvxd Routine\n");
	free(MD5_Client);
	free(MD5_Server);


	bzero(recv_pack,sizeof(recv_pack));
	bzero(packet_no,sizeof(packet_no));
	//bzero(prev_pack_no,sizeof(prev_pack_no));
	bzero(rcvd_packet_no,sizeof(rcvd_packet_no));
	bzero(rcvd_size,sizeof(rcvd_size));
	bzero(recv_frame,sizeof(recv_frame));
	rcvd_bytes=0;
	pos_data=0;
	pos_size=0;

	return rcount;
}

*/


/*************************************************************
// Send a file from Server to Client
I/p : filename - File name to be pused to client 
o/p : find and file is pushed to client 
	  return number of status
Reference for understanding :
https://lms.ksu.edu.sa/bbcswebdav/users/mdahshan/Courses/CEN463/Course-Notes/07-file_transfer_ex.pdf	  
**************************************************************/
/*
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
		sendtoClient(err_indication,sizeof(err_indication),NOACK);
		sendtoClient(err_message,sizeof(err_message),NOACK);
		return -1;
	}
	else // open file successfully ,read file 
	{
		//debug 
		//printf("Sending %s\n",filename);
		//read bytes and send 
		while((file_bytes = read(fd,send_pack,MAXPACKSIZE)) != 0 ){//read data from file 			
			//Append packet no to packet --Frame -- PACKETNO|PACKETSIZE|PACKETDATA
			sprintf(send_frame,"%06d|%06d|",scount,(int)file_bytes);
			memcpy(send_frame+temp_displace,send_pack,file_bytes);//copy the send pack to send frame (** use memset as image files an issue)
			//printf("frame :%s\n",send_frame );

			#ifndef RELIABILITY
				recvd_ack_no=sendtoClient(send_frame,sizeof(send_frame),ACK);	//send frame
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
		    			recvd_ack_no=sendtoClient(send_frame,sizeof(send_frame),ACK);	//send frame
		    			//printf("ack no%d\n",recvd_ack_no );
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
	sendtoClient(comp_indication,(ssize_t)sizeof(comp_indication),NOACK);	//send Completion of File
	MD5Cal(filename,MD5_message);
	//printf("MD5 Message%s\n",MD5_message );
	//Send MD5 Hash value 
	sendtoClient(MD5_message,2*MD5_DIGEST_LENGTH,NOACK);

	
	//printf("No of packets send %d\n",scount );
	return scount;
}
*/

//Client connection for each client 
/*
void *client_connections(void *client_sock_id){

	DEBUG_PRINT("In %d",client_sock_id);
}
*/

char (*action)[MAXCOLSIZE];

void *client_connections(void *client_sock_id)
{
	
	
	int total_attr_commands=0,i=0;
	int thread_sock = (int*)(client_sock_id);
	ssize_t read_bytes=0;
	char message_client[MAXPACKSIZE];//store message from client 
	char message_bkp[MAXPACKSIZE];//store message from client 
	char command[MAXPACKSIZE];
	char (*packet)[MAXCOLSIZE];
	char directory[MAXCOLSIZE]=".";
	DEBUG_PRINT("passed Client connection %d\n",(int)thread_sock);
	DEBUG_PRINT("in client connections dir : %s",directory);
		//Clear the command and request to Client 
		bzero(message_client,sizeof(message_client));

		

		// Recieve the message from client  and reurn back to client 
		if((read_bytes =recv(thread_sock,message_client,MAXPACKSIZE,0))>0){

			//printf("request from client %s\n",message_client );
			strcpy(message_bkp,message_client);//backup of orginal message 
			DEBUG_PRINT("Check DFS=> %s \n",config.DFSdirectory );
			DEBUG_PRINT("Message from Client => %s \n",message_client );

			if ((strlen(message_client)>0) && (message_client[strlen(message_client)-1]=='\n')){
					message_client[strlen(message_client)-1]='\0';
			}

			//
				if ((packet=malloc(sizeof(packet)*MAXCOLSIZE))){	
				total_attr_commands=0;
				if((total_attr_commands=splitString(message_client,"/",packet,5)>0))
				{
	   				DEBUG_PRINT("User %s => length %d",packet[DFCUserloc]);
	   				DEBUG_PRINT("Password %s",packet[DFCPassloc]);
	   				//DEBUG_PRINT("Total Commands %d",total_attr_commands);
					
					if ((strncmp(packet[DFCCommandloc],"LIST",strlen("LIST"))==0)){
						//send command
						DEBUG_PRINT("Inside LIST");
						//bzero(directory,strlen(directory));
						strncat(directory,config.DFSdirectory,strlen(config.DFSdirectory));
						strcat(directory,"/");
						strncat(directory,packet[DFCUserloc],strlen(packet[DFCUserloc]));
						strcat(directory,"/");
						DEBUG_PRINT("directory : %s",directory);
						//strncpy(directory,packet[DFCUserloc]);
						list(directory,thread_sock);
						//strcpy(requesttoserver.DFCRequestCommand,"LIST");
						//sendcommandToDFS(requesttoserver);
						//sendtoServer(command,strlen(command),NOACK);//send command to server
						//sendCommand();				
						//listRcv();								
					}
					else if ((strncmp(packet[DFCCommandloc],"GET",strlen("GET")))==0){
							DEBUG_PRINT("Inside GET");
							DEBUG_PRINT("File Name %s",packet[DFCFileloc]);
							//strcpy(requesttoserver.DFCRequestFile,action[file_location]);
							//send command
							//strcpy(requesttoserver.DFCRequestCommand,"GET");
							//sendcommandToDFS(requesttoserver);

								//if(rcvFile(action[file_location]) <0){	
								//	printf("Error in get!!!\n");
									
								//} v
						
			  		}

					else if ((strncmp(packet[DFCCommandloc],"PUT",strlen("PUT")))==0){
							DEBUG_PRINT("Inside PUT");
							DEBUG_PRINT("File Name %s",packet[DFCFileloc]);
							//strcpy(requesttoserver.DFCRequestFile,action[file_location]);
							//send command
							//strcpy(requesttoserver.DFCRequestCommand,"PUT");
							//sendcommandToDFS(requesttoserver);

								//if(rcvFile(action[file_location]) <0){	
								//	printf("Error in get!!!\n");
									
								//} v
						
			  		}
			  	}	
				else
				{
					printf("Error in Command Split\n");
					exit(-1);	
				}
			}
			else
			{
				perror("Allocation for command ");
			}




			/*
			action = &temp;
			bzero(command,sizeof(command));
			bzero(action,sizeof(action));	
			
			//waits for an incoming message
			nbytes = recvfrom(sock,command,sizeof(command),0,(struct sockaddr*)&sin,&remote_length);
			if ((strlen(command)>0) && (command[strlen(command)-1]=='\n')){
					command[strlen(command)-1]='\0';
			}
			//memcpy(command,command,sizeof(command)-1);			
			if(garbage_count)
			{
				check_bytes=0;
			}
			sendtoClient(ack_message,sizeof(ack_message),NOACK);
			//Messaged received successfully
			//printf("\nRxcv command : %s bytes %d \n",command,nbytes );
			strcat(command,"\0");
			if (nbytes >0){
					total_attr_commands=splitString(command," ",action);	
						if(total_attr_commands>=0)
						{
								//printf("Command not supported\n");
								
							int count=0;
							action = &temp;
							garbage_count++;
							//printf("Action : %s\n",**action );
							//Check type of command
							if (strcmp(**action,"ls")==0){							
										**action++;//gr
										//printf("value after ls :%s\n", **action);
										if (strlen(**action)>check_bytes){
										//	printf("Command not supported\n");
											sendtoClient("Command not supported",sizeof("Command not supported"),NOACK);							
										}	
										else if(list()<0){
										//		printf("Error in listing!!!\n");
												sendtoClient("Error",sizeof("Error"),NOACK);
											}		
									}
							else if (strcmp(**action,"get")==0){
										//printf("in get\n");
										**action++;//gr
										if(sendFile(**action) <0){	
											//printf("Error in get!!!\n");
											sendtoClient("Error",sizeof("Error"),NOACK);
										}
									}
							else if (strcmp(**action,"put")==0){
										//printf("in put\n");
										**action++;
										//printf("File %s\n",**action);
										if(rcvFile(**action) <0){	
											//printf("Error in put!!!\n");
											//sendtoClient("Error");
										}
									}
							else if (strcmp(**action,"exit")==0){
										printf("Exiting server .........\n");
										close(sock);
										break;
									}
							else 	{
										sendtoClient(command,sizeof(command),NOACK);
										//printf("Command not supported\n");
									}
							//clearing the split value 		
							bzero(action,sizeof(action));		
						}	
							
			}

			//sendtoClient(msg);
		}
			
		//close the socket 
		close(sock);
	*/























			
			/*			
			if ((split_attr=malloc(sizeof(split_attr)*MAXCOLSIZE))){	
				strcpy(split_attr[HttpVersion],"HTTP/1.1");//Default
				strcpy(split_attr[HttpMethod],"GET");//Default
				strcpy(split_attr[HttpURL],"index.html");//No data 
	
				if((total_attr_commands=splitString(message_client," ",split_attr,4))<0)
				{
					DEBUG_PRINT("Error in split\n");

					//printf("%s\n", );
					bzero(message_client,sizeof(message_client));	
					bzero(split_attr,sizeof(split_attr));	
					return NULL;
				}
				else
				{
					
					bzero(message_client,sizeof(message_client));	
					bzero(split_attr,sizeof(split_attr));				
					DEBUG_PRINT("Cannot split input request");
				}
				//print the split value 
				
				for(i=0;i<total_attr_commands;i++){
					DEBUG_PRINT("%d %s\n",i,split_attr[i]);
				}
				
				//printf("in client connections%s\n",message_bkp);
				
				responsetoClient(split_attr,thread_sock,message_bkp);
							
				//free alloaction of memory 
				for(i=0;i<total_attr_commands;i++){
					free((*split_attr)[i]);
				}
				
				free(split_attr);//clear  the request recieved 
						
			}
			else 
			{
					error_response("500 Internal Server Error",split_attr[HttpURL],thread_sock,split_attr[HttpVersion],"Invalid File Name");
					perror("alloacte 2d pointer");
					exit(-1);
			}		

		}
		if (read_bytes < 0){
			perror("recv from client failed ");
			return NULL;

		}
	*/
	}
		
	DEBUG_PRINT("Completed \n");
	//Closing SOCKET
    shutdown (thread_sock, SHUT_RDWR);         //All further send and recieve operations are DISABLED...
    close(thread_sock);
    thread_sock=-1;
	//free(thread_sock);//free the connection 

	//free(client_sock_id);//free the memory 
	return 1 ;

	
}





int main (int argc, char * argv[] ){
	//char command[MAXBUFSIZE];             //a command to store our received message
	int i=0;
	
	//pthread_t DFS_thread[MAXDFSCOUNT];
	pthread_t client_thread;
	
	/* You will have to modify the program below */

	if (argc < 3)
	{
		printf("USAGE:  <dfs_name> <server_port>\n");
		exit(1);
	}
	else
	{
		printf("USAGE IP %s :PORT %s \n",argv[1],argv[2]);
		
	}

	//Server Configuration 
	maxtypesupported=config_parse("dfs.conf");

	// Print the Configuration 
	DEBUG_PRINT("Configuration from Server");
	//Check for file support available or not 
	if (!maxtypesupported){
		printf("Zero Username and Password found !! Check Config File\n");
		exit(-1);
	}
	else
	{	
		DEBUG_PRINT("DFS Configured");
		for (i=0;i<maxtypesupported;i++){
			//DEBUG_PRINT("%d %s %s",i,config.DFSIP[i],config.DFSPortNo[i]);
			DEBUG_PRINT("DFS User:Password %s:%s",config.DFCUsername[i],config.DFCPassword[i]);
		}
	}	


	//Set up all the server
	/******************
	  This code populates the sockaddr_in struct with
	  the information about our socket
	 ******************/
	bzero(&server,sizeof(server));                    //zero the struct
	server.sin_family = AF_INET;                   //address family
	//server.sin_port = htons(atoi(argv[1]));        //htons() sets the port # to network byte order

	//config. = argv[2];
	strcpy(config.listen_port,argv[2]);


	//Check if Port is present or not 
	if (strcmp(config.listen_port,"")){
		DEBUG_PRINT("Port %s",config.listen_port);
		if(atoi(config.listen_port) <=1024){
			printf("\nPort Number less than 1024!! Check configuration file\n");
			exit(-1);
		}
		else
		{
			server.sin_port = htons(atoi(config.listen_port));        		//htons() sets the port # to network byte order
		}	
		
	}
	else
	{	
		printf("\nPort Number not found !! Check configuration file");
		exit(-1);
	}
	
	server.sin_addr.s_addr = INADDR_ANY;           //supplies the IP address of the local machine
	remote_length = sizeof(struct sockaddr_in);    //size of client packet 
	
	strcpy(config.DFSdirectory, argv[1]);

	//check if document root directory present 
	if (strcmp(config.DFSdirectory,"")){
		DEBUG_PRINT("DFSdirectory %s",config.DFSdirectory);
	}
	else
	{	
		printf(" DFS directory not found !! Check Input \n");
		exit(-1);
	}



	//Causes the system to create a generic socket of type TCP (strean)
	if ((server_sock =socket(AF_INET,SOCK_STREAM,0)) < 0){
		DEBUG_PRINT("unable to create tcp socket");
		exit(-1);
	}

	/******************
	  Once we've created a socket, we must bind that socket to the 
	  local address and port we've supplied in the sockaddr_in struct
	 ******************/
	if (bind(server_sock, (struct sockaddr *)&server, sizeof(server)) < 0){
		close(server_sock);
		printf("unable to bind socket\n");
		perror("Bind Socket");
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
			
		/*
		//as it does  have to wait for it to join thread ,
		//does not allow multiple connections 
		if(pthread_join(client_thread, NULL) == 0)
		 printf("Client Thread done\n");
		else
		 perror("Client Thread");
		 */
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
}
		

		


	/*	
		
		action = &temp;
		bzero(command,sizeof(command));
		bzero(action,sizeof(action));	
		
		//waits for an incoming message
		nbytes = recvfrom(sock,command,sizeof(command),0,(struct sockaddr*)&sin,&remote_length);
		if ((strlen(command)>0) && (command[strlen(command)-1]=='\n')){
				command[strlen(command)-1]='\0';
		}
		//memcpy(command,command,sizeof(command)-1);
		
		if(garbage_count)
		{
			check_bytes=0;
		}
		sendtoClient(ack_message,sizeof(ack_message),NOACK);
		//Messaged received successfully
		//printf("\nRxcv command : %s bytes %d \n",command,nbytes );
		strcat(command,"\0");
		if (nbytes >0){
				total_attr_commands=splitString(command," ",action);	
					if(total_attr_commands>=0)
					{
							//printf("Command not supported\n");
							
						int count=0;
						action = &temp;
						garbage_count++;
						//printf("Action : %s\n",**action );
						//Check type of command
						if (strcmp(**action,"ls")==0){							
									**action++;//gr
									//printf("value after ls :%s\n", **action);
									if (strlen(**action)>check_bytes){
									//	printf("Command not supported\n");
										sendtoClient("Command not supported",sizeof("Command not supported"),NOACK);							
									}	
									else if(list()<0){
									//		printf("Error in listing!!!\n");
											sendtoClient("Error",sizeof("Error"),NOACK);
										}		
								}
						else if (strcmp(**action,"get")==0){
									//printf("in get\n");
									**action++;//gr
									if(sendFile(**action) <0){	
										//printf("Error in get!!!\n");
										sendtoClient("Error",sizeof("Error"),NOACK);
									}
								}
						else if (strcmp(**action,"put")==0){
									//printf("in put\n");
									**action++;
									//printf("File %s\n",**action);
									if(rcvFile(**action) <0){	
										//printf("Error in put!!!\n");
										//sendtoClient("Error");
									}
								}
						else if (strcmp(**action,"exit")==0){
									printf("Exiting server .........\n");
									close(sock);
									break;
								}
						else 	{
									sendtoClient(command,sizeof(command),NOACK);
									//printf("Command not supported\n");
								}
						//clearing the split value 		
						bzero(action,sizeof(action));		
					}	
						
		}

		//sendtoClient(msg);
	}
		
	//close the socket 
	close(sock);
	*/

