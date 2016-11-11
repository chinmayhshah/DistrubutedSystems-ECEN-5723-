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
#include <pthread.h> // For threading , require change in Makefile -lpthread
#include <semaphore.h> // For using semaphore

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
//int sock;                               //this will be our socket
char buffer[MAXBUFSIZE];

struct sockaddr_in remote;              //"Internet socket address structure"
struct sockaddr *remoteaddr;
//struct hostent *server;
struct sockaddr_in from_addr;
int addr_length = sizeof(struct sockaddr);
char ack_message[ACKMESSAGE];


typedef enum TYPEACK{NOACK,ACK,TIMEDACK}ACK_TYPE;


//Time set for Time out 
struct timeval timeout={0,0};     




//Using Mutex
//sem_t semDFS[MAXDFSCOUNT];
pthread_mutex_t thread_mutex;







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






struct configClient{
		char DFSName[MAXDFSCOUNT][MAXCOLSIZE];
		char DFSIP[MAXDFSCOUNT][MAXCOLSIZE];
		char DFSPortNo[MAXDFSCOUNT][MAXCOLSIZE];
		char DFCUsername[MAXCOLSIZE];
		char DFCPassword[MAXCOLSIZE];
 };




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


struct configClient config;
int maxtypesupported=0; //typedef enum HTTPFORMAT{RM,RU,RV}HTTP_FM;


//int server_sock,client_sock;                           //This will be our socket
//struct sockaddr_in server, client;     //"Internet socket address structure"
//unsigned int remote_length;         //length of the sockaddr_in structure
//int nbytes;                        //number of bytes we receive in our message




//fixed Root , take from configuration file 
//char * ROOT = "/home/chinmay/Desktop/5273/PA2/www";
char *configfilename ="dfc.conf"; // take input name 

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
Send message from  Client to Server
input 	
		msg   - String to be transmitted
		bytes -No of bytes transferred 
		Type  - Wait for ACK or Not 
*************************************************************/
		/*
int sendtoServer(char *msg,ACK_TYPE type_ack,int sock)
{
	ssize_t send_bytes,recv_bytes;
	int rcvd_packno=0;
	char ack[ACKMESSAGE];
	//printf("send to Server %s , %d\n",msg,(int)bytes );
	//Concat the Command to be send 


	//Send to Server 
	if ((send_bytes=write(sock,msg,bytes,sizeof(remote))) < bytes)
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

*/
//Send data to different DFS
int sendDataToDFS (struct DataFmt sendData)
{

	char sendMessage[MAXBUFSIZE];
	sprintf(sendMessage,"%s/%s/%s/%s/%s",sendData.DFCRequestUser,sendData.DFCRequestPass,sendData.DFCRequestCommand,sendData.DFCRequestFile,sendData.DFCData);
	DEBUG_PRINT("Data to Server => %s",sendMessage);

	write(sendData.socket,sendMessage,sizeof(sendMessage));		

}




int sendcommandToDFS (struct requestCommandFmt reqCommand)
{
	char sendMessage[MAXBUFSIZE];
	sprintf(sendMessage,"%s/%s/%s/%s",reqCommand.DFCRequestUser,reqCommand.DFCRequestPass,reqCommand.DFCRequestCommand,reqCommand.DFCRequestFile);
	DEBUG_PRINT("Command to Server => %s",sendMessage);

	//Send Command
	//for (int i = 0; i < MAXDFSCOUNT; ++i)
	//{
		/* code */
		//write(socket[i],sendMessage,sizeof(sendMessage));	
	//}
	write(reqCommand.socket,sendMessage,sizeof(sendMessage));	

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
int 	splitString(char *splitip,char *delimiter,char (*splitop)[MAXCOLSIZE],int maxattr)
{
	int sizeofip=1,i=1;
	char *p=NULL;//token
	char *temp_str = NULL;


	DEBUG_PRINT("Inout string %s",splitip);
	
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
		DEBUG_PRINT("successful split %d ",sizeofip);
		//DEBUG_PRINT("Here 7");
		return sizeofip;//Done split and return successful }
	}	
	return sizeofip;		
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


#define MAXFILESIZE 50000


/*************************************************************************************
Split Files into multiple files 
i/p :sourcefilename -  Filename to be split 
	:parts - No of equal size parts need to be split into 

Ref for understanding :http://www.programmingsimplified.com/c-program-merge-two-files
***************************************************************************************/
int splitFile(char *sourcefilename,int parts)
{

 	//char sourceFileName[MAXPACKSIZE], ch, 
 	char destFileName[MAXPACKSIZE];
 	int ch=0;
    long int size=0, k=0;
    int i=0;
    FILE *sourceFile,*splitFile; //file and temp file

    //printf("enter the file you want to split with full path : ");
    //scanf("%s", fn);
    //printf("enter the number of parts you want to split the file : ");
    //scanf("%ld", &n);
    DEBUG_PRINT("Filename => %s",sourcefilename);
    DEBUG_PRINT("NO of parts => %d",parts);

    sourceFile=fopen(sourcefilename, "rb");//Open source file name 
    if (sourceFile==NULL)// if file closed or not
    {
        printf("couldn't open file");
        exit(-1);
    }

    // fseek(FILE *stream, long int offset, int whence)
    fseek(sourceFile, 0, 2);//
    size = ftell(sourceFile);// Calculate Size of Filename
    DEBUG_PRINT("Size(File) => %ld\n", size);

    i = 1;
    k = (int)(size/parts);
    DEBUG_PRINT("Split Size(File) => %ld\n", k);
    
    rewind(sourceFile);// Change the pointer back to original
    sprintf(destFileName, "%s.%d", sourcefilename, i);// Format <sourcefilename>.<i> eg:#file.txt.1
    DEBUG_PRINT("Split File name %s",destFileName);
    splitFile = fopen(destFileName, "wb");//Open the destincation file
    if (splitFile==NULL)// if file closed or not
    {
        printf("couldn't open file");
        perror("Destination File Open");
        exit(-1);
    }
    DEBUG_PRINT("Here ");
    while(i<=(parts))
    {
        ch = fgetc(sourceFile);    
        //if (ch==EOF)//check for end of file 
        //    break;
        //DEBUG_PRINT("Read Value => %s\n", ch);        
        DEBUG_PRINT("Here 2");
        fputc(ch, splitFile);
        if(feof(splitFile) )
	    {
	    	DEBUG_PRINT("End of File=> File %d",i);
	        break ;
	    }
        //To check for the size of file written 
        if (ftell(sourceFile)==(i*k))//check the size of file reached 
        {
        	DEBUG_PRINT("Data of  file reached %d",(i*k));
		    i = i+1;
		    DEBUG_PRINT("Closing %s:%d",destFileName,splitFile);
			fclose(splitFile);
			if (i<=(parts)){
	            sprintf(destFileName, "%s.%d", sourcefilename, i);// Format <sourcefilename>.<i> eg:#file.txt.1
	            splitFile=fopen(destFileName, "wb");
	        }
        }
    }
    if(sourceFile){
    	fclose(sourceFile);
    }	
   
}




/*************************************************************************************
Merge  Files into single file from  multiple files 
i/p - Filename i/p to 
	-   	
***************************************************************************************/
int mergeFile(char *mergedFileName,int parts)
{
	char sourceFileName[MAXPACKSIZE];// Source file name
 	int ch=0; long int size=0, k=0;
    int i;
    FILE *sourceFile,*mergeFile; //file and temp file

    DEBUG_PRINT("Filename => %s",mergedFileName);
    DEBUG_PRINT("NO of parts => %d",parts);


    mergeFile=fopen(mergedFileName, "wb");//Open destn Merge file name in write mode 
    if (mergeFile==NULL)// if file closed or not
    {
        printf("couldn't open file %s",mergedFileName);
        exit(-1);
    }

    i = 1;   
    
    
	sprintf(sourceFileName, "%s.%d", mergedFileName, i);// Format <sourcefilename>.<i> eg:#file.txt.1
	DEBUG_PRINT(" File name to be merged %s",sourceFileName);
	sourceFile = fopen(sourceFileName, "rb");//Open the source file 
	if (sourceFile == NULL)
	{
		printf("couldn't open file %s",mergedFileName);
        exit(-1);

	}
    
    while(i<=(parts))
    {

	    if (sourceFile==NULL)// if file opened or not
	    {
	        printf("couldn't open file");
	        perror("Source File Open");
	        exit(-1);
	    }
	    //read each chacracter of file
	    ch =0;
        ch = fgetc(sourceFile);    
        
        if(feof(sourceFile))
	    {
	    
		    i = i+1;
		    DEBUG_PRINT("End of File=> File upaded %d",i);
		    DEBUG_PRINT("Closing %s:%d",sourceFileName,sourceFile);
			fclose(sourceFile);
			if (i<=(parts)){
	            sprintf(sourceFileName, "%s.%d", mergedFileName, i);// Format <sourcefilename>.<i> eg:#file.txt.1
	            DEBUG_PRINT(" File name to be merged %s",sourceFileName);
	            sourceFile=fopen(sourceFileName, "rb");
	            	if (sourceFile == NULL)
					{
						printf("couldn't open file %s",mergedFileName);
				        exit(-1);
					}
	        }
	    }
	    else
	    {
		    
	        fputc(ch, mergeFile);// put in o/p file      
	    }
	   
    }
    fputc('\n',mergeFile);// add extra new line at end of file as fputch does not add it by itself
    if(mergeFile){
    	fclose(mergeFile);
    }	
   
}

//Input from user in a while loop 
char command[MAXCOMMANDSIZE];//Local command storage 
struct DataFmt datatoserver;
struct requestCommandFmt requesttoserver;

//For Socket
int sock[MAXDFSCOUNT];//No(Sockets) =DFS Server availables
struct sockaddr_in server[MAXDFSCOUNT];
int total_attr_commands;
//type2D *action;//for splitting commands 
char (*action)[MAXCOLSIZE];
char list[MAXPACKSIZE][MAXCOLSIZE];// To store list common between all threads
int list_count=0;
int resource_mutex=1;
//Call back function for thread 
//Input is the thread id(i) - maps to the DFSIP and DFSPORT 
void *DFSThreadServed(void *Id){

			int DFSId = (int)Id;
			

			
			DEBUG_PRINT("DFS Id connection %d\n",(int)DFSId);
			//Create multiple Socket and connect them (TCP)
			//for (i=0;i<MAXDFSCOUNT;i++){
			    //if((sock[i] = socket(AF_INET , SOCK_STREAM , 0))<0)//create socket
		    //{
			if ((sock[DFSId]= socket(AF_INET , SOCK_STREAM , 0))<0){
                printf("Issue in Creating Socket,Try Again !! %d\n",sock[DFSId]);
		        perror("Socket --> Exit ");			        
		    	//exit(-1); // what needs to be done ?
	    		return -1;
	    	}
		    //}
		    DEBUG_PRINT("created SOCKet %d ",sock[DFSId]);
			
		    //Connect the multiple sockets 
		    server[DFSId].sin_addr.s_addr = inet_addr(config.DFSIP[DFSId]);
		    server[DFSId].sin_family = AF_INET;
		    //server[i].sin_port = htons( config.DFSPortNo[i] );
		    server[DFSId].sin_port = htons(atoi(config.DFSPortNo[DFSId]));        		//htons() sets the port # to network byte order
		    
		    
			pthread_mutex_lock(&thread_mutex);//lock mutex	
			DEBUG_PRINT("Locked Mutex and connect socket ");
				    //Connect to remote server
				    if (connect(sock[DFSId] , (struct sockaddr *)&server[DFSId] , sizeof(server[DFSId])) < 0)
				    {
				        perror("Connect failed. Error");
				        //exit(-1);
				        return -1;// Return as Socket is not up 
				    }
			 		DEBUG_PRINT("connected SOCKet %d",sock[DFSId]);
				    //i=0;

				    
					//copy socket 
					requesttoserver.socket=datatoserver.socket=sock[DFSId];//assigning only server one socket for now
					
					//compare the input command and perform necessary action inside thread  		
					if ((strncmp(action[command_location],"LIST",strlen("LIST"))==0)){
						//send command
						DEBUG_PRINT("Inside Thread %d LIST",(int)DFSId);
						//strcpy(requesttoserver.DFCRequestCommand,"LIST");
						strcpy(requesttoserver.DFCRequestCommand,"LIST");
						sendcommandToDFS(requesttoserver);
						//sendtoServer(command,strlen(command),NOACK);//send command to server
						//sendCommand();				
						//listRcv();								
						DEBUG_PRINT("Waiting for List");
						bzero(list[list_count],sizeof(list[list_count]));
						//#ifdef NON_BLOCKING
						if(nbytes = recv(sock[DFSId],list[list_count],sizeof(list[list_count]),0)<0){//recv from server and check for non-blocking 
							fprintf(stderr,"non-blocking socket not returning data  List %s\n",strerror(errno) );
						}
						
						//#else
							//nbytes = recvfrom(sock,buffer,sizeof(buffer),0,(struct sockaddr*)&remote,&addr_length);
						//#endif	
						DEBUG_PRINT("Last Filename in thread %d , list_count %d = >%s",(int)Id,list_count,list[list_count]);
						list_count++;
					}
					else if ((strncmp(action[command_location],"GET",strlen("GET")))==0){
								

								DEBUG_PRINT("Inside Thread %d GET",(int)DFSId);
								strcpy(requesttoserver.DFCRequestCommand,"GET");
								strcpy(requesttoserver.DFCRequestFile,action[file_location]);
								//strcpy(requesttoserver.DFCRequestFile,threadaction[file_location]);
								//send command
								//DEBUG_PRINT("File Name %s",threadaction[file_location]);
								//strcpy(requesttoserver.DFCRequestCommand,"GET");
								sendcommandToDFS(requesttoserver);

									//if(rcvFile(action[file_location]) <0){	
									//	printf("Error in get!!!\n");
										
									//} v
							
				  	}
				  	else if ((strncmp(action[command_location],"PUT",strlen("PUT")))==0){
								
								DEBUG_PRINT("Inside Thread %d PUT",(int)DFSId);
								strcpy(requesttoserver.DFCRequestCommand,"PUT");
								strcpy(requesttoserver.DFCRequestFile,action[file_location]);
								//strcpy(requesttoserver.DFCRequestFile,threadaction[file_location]);
								//send command
								//DEBUG_PRINT("File Name %s",threadaction[file_location]);
								//strcpy(requesttoserver.DFCRequestCommand,"GET");
								sendcommandToDFS(requesttoserver);

									//if(rcvFile(action[file_location]) <0){	
									//	printf("Error in get!!!\n");
										
										//} 
						
							
					}
		pthread_mutex_unlock(&thread_mutex);	
		DEBUG_PRINT("UnLOcked Mutex");						
		//Close multiple Socket 
		shutdown (sock[DFSId], SHUT_RDWR);         //All further send and recieve operations are DISABLED...
    	close(sock[DFSId]);
    	sock[DFSId]=-1;
    
}

		
int main (int argc, char * argv[] ){
    int i=0,c=0,l=0;
    char message[1000] , server_reply[2000];
	char request[MAXPACKSIZE];             //a request to store our received message
	
	pthread_t DFSThread[MAXDFSCOUNT];
	
	//Input of filename for config 
	if (argc != 2){
		printf ("USAGE:  <Conf File>\n");
		exit(1);
	}
	
	//Configuration file before starting the Web Server 
	DEBUG_PRINT("Reading the config file %s",argv[1]);
	maxtypesupported=config_parse(argv[1]);

	// Print the Configuration 
	DEBUG_PRINT("Confiuration Obtain");
	//Check for file support available or not 
	if (!maxtypesupported){
		printf("Zero DFS found !! Check Config File\n");
		exit(-1);
	}
	else
	{	
		DEBUG_PRINT("DFS Configured");
		for (i=0;i<maxtypesupported;i++){
			//DEBUG_PRINT("%d %s %s",i,config.DFSIP[i],config.DFSPortNo[i]);
			DEBUG_PRINT("DFS IP:PORT %d %s - %s:%s",i,config.DFSName[i],config.DFSIP[i],config.DFSPortNo[i]);
		}
	}	
		
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
		printf("User Password not found !! Check configuration file\n");
		exit(-1);
	}//Configuratoin Complete 


    
	// assiging standard parameters of client 
	
	//copy username 
	strcpy(datatoserver.DFCRequestUser,config.DFCUsername);
	strcpy(requesttoserver.DFCRequestUser,config.DFCUsername);

	//copy password 
	strcpy(requesttoserver.DFCRequestPass,config.DFCPassword);
	strcpy(datatoserver.DFCRequestPass,config.DFCPassword);

	DEBUG_PRINT("User => Password %s=>%s", requesttoserver.DFCRequestUser,requesttoserver.DFCRequestPass);
	
	//Test  of file functions 
	//splitFile("sample.txt",4);
	//mergeFile("sample.txt",4);
	//Initiate Mutex 
  	pthread_mutex_init(&thread_mutex,NULL);


	DEBUG_PRINT("Completed splitting File name");     
    //keep communicating with server depending on command entered 
	printf("Enter Command\n");
	printf("GET <Filename> - to get file from DFS\n");
	printf("PUT <Filename> - to put file to DFS \n");
	printf("LIST -list files on  DFS server\n");
	
		
	while(1)
	{//start of while 

			//Clear the command and request to Client 
			bzero(requesttoserver.DFCRequestFile,sizeof(requesttoserver.DFCRequestFile));
			bzero(requesttoserver.DFCRequestFile,sizeof(requesttoserver.DFCRequestCommand));
			bzero(command,sizeof(command));
			//wait for input from user 
			fgets(command,MAXCOMMANDSIZE,stdin);
			if ((strlen(command)>0) && (command[strlen(command)-1]=='\n')){
					command[strlen(command)-1]='\0';
			}	


			//bzero(ack_recv,sizeof(ack_recv));	
			//ack_bytes=recvfrom(sock,ack_recv,sizeof(ack_recv),0,(struct sockaddr*)&remote,&addr_length);
			strcat(command,"\n");
			DEBUG_PRINT("Command %s",command);
			int cmpvalue=100;

			//Split the input from user 
			if ((action=malloc(sizeof(action)*MAXCOLSIZE))){	
				total_attr_commands=0;
				if((total_attr_commands=splitString(command," ",action,3)>0))
				{
	   				DEBUG_PRINT("Total Commands >0  => %d",total_attr_commands);
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

			//clear all buffers before sending the command 
			DEBUG_PRINT("Clearing buffers");
			bzero(list,strlen(list));//clear the list before evoking the threads 
			list_count=0;
			DEBUG_PRINT("List Buffer(should be empty or garbage) => %s",list);


			//*** change logic , should not create the threads if command invalid *****
			//Create the thread for all the communication with DFS Servers
			for (c=0;c<MAXDFSCOUNT;c++ ){
				if ((pthread_create(&DFSThread[c],NULL,DFSThreadServed,(void *)c))<0){
					//close(server_sock);
					perror("Thread not created");
					exit(-1);

				}	
			
			}
			//Waiting for Join of all threads
			for (c=0;c<MAXDFSCOUNT;c++){
				if(pthread_join(DFSThread[c], NULL) == 0)
				 {
				  DEBUG_PRINT("Thread %d completed\n",c);
				 }	
				 else
				 {
				   perror(" Thread Join");
				 }
			}
			DEBUG_PRINT("Thread join Completed \n");

			//IN main depending on Command 

					DEBUG_PRINT("Command %s => length %d",action[command_location]);
	   				DEBUG_PRINT("File(iF present) => %s",action[file_location]);
	   				DEBUG_PRINT("Total attributes of input  => %d",total_attr_commands);
					
					if ((strncmp(action[command_location],"LIST",strlen("LIST"))==0)){
						//send command
						//strcpy(requesttoserver.DFCRequestCommand,"LIST");
						//sendcommandToDFS(requesttoserver);
						//sendtoServer(command,strlen(command),NOACK);//send command to server
						//sendCommand();				
						//listRcv();	
						DEBUG_PRINT("Complete List RCVD");						
						for (l=0;l<list_count;l++){	
							DEBUG_PRINT(" %s",list[l]);
						}
					}
					else if ((strncmp(action[command_location],"GET",strlen("GET")))==0){
							DEBUG_PRINT("Inside GET");
							DEBUG_PRINT("File Name %s",action[file_location]);
							//strcpy(requesttoserver.DFCRequestFile,action[file_location]);
							//send command
							//strcpy(requesttoserver.DFCRequestCommand,"GET");
							//sendcommandToDFS(requesttoserver);

								//if(rcvFile(action[file_location]) <0){	
								//	printf("Error in get!!!\n");
									
								//} v
						
			  		}

					else if ((strncmp(action[command_location],"PUT",strlen("PUT")))==0){
							DEBUG_PRINT("Inside PUT");
							DEBUG_PRINT("File Name %s",action[file_location]);
							//strcpy(requesttoserver.DFCRequestFile,action[file_location]);
							//send command
							//strcpy(requesttoserver.DFCRequestCommand,"PUT");
							//sendcommandToDFS(requesttoserver);

								//if(rcvFile(action[file_location]) <0){	
								//	printf("Error in get!!!\n");
									
								//} v
						
			  		}






















			DEBUG_PRINT("Deallocate memory for command");
			//Free the command allocated 
			if (action!=NULL){
				//free alloaction of memory 
				for(i=0;i<total_attr_commands;i++){
					free((*action)[i]);
				}
				free(action);//clear  the request recieved 
			}

			
		
	}//While end
	
	return 0;

	
}
	





















	
	