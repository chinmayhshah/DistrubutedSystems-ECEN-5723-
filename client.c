/*******************************************************************************
Base Code : Provided by Prof 
Author :Chinmay Shah 
File :dfsclient.c
Last Edit : 11/17

File for implementation of a Distrubuted File Client and Server 
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

#define MAXCOMMANDSIZE 100
#define PACKETNO 7
#define SIZEMESSAGE 7
#define MAXFRAMESIZE (PACKETNO+SIZEMESSAGE+MAXPACKSIZE)
#define ACKMESSAGE 4+PACKETNO
#define MAXREPEATCOUNT 3
#define RELIABILITY
#define ERRMESSAGE 20


/* You will have to modify the program below */
#define LISTENQ 1000 
#define SERV_PORT 3000

#define MAXCOLSIZE 1000
#define MAXLISTSIZE MAXCOLSIZE*5
#define MAXFILESCOUNT 1000
#define HTTPREQ 	30

#define MAXDFSCOUNT 4
#define MAXBUFSIZE 60000
#define MAXPACKSIZE 60000
#define ERRORMSGSIZE 1000
#define MAXCOMMANDSIZE 100
#define MAXCONTENTSUPPORT 15

#define MAXFILESIZE 50000
#define MAXACKSIZE 10





#define DEBUGLEVEL

#ifdef DEBUGLEVEL
	#define DEBUG 1
#else
	#define	DEBUG 0
#endif

#define DEBUG_PRINT(fmt, args...) \
        do { if (DEBUG) fprintf(stderr, "\n %s:%d:%s(): " fmt, __FILE__, \
                                __LINE__, __FUNCTION__, ##args); } while (0)



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
char list[MAXFILESCOUNT][MAXLISTSIZE];// To store list common between all threads
int list_count=0;
int resource_mutex=1;
// Default split of parts 	 //array     0 1 2 3
								        //DFS1 2 3 4	
//int partdest1[MAXDFSCOUNT][MAXDFSCOUNT]={1,2,3,4};
//int partdest2[MAXDFSCOUNT][MAXDFSCOUNT]={2,3,4,1};

int partdest1[MAXDFSCOUNT];
int partdest2[MAXDFSCOUNT];
//Call back function for thread 
//Input is the thread id(i) - maps to the DFSIP and DFSPORT 

enum ERR_STATE {CRED_ERROR,CRED_FAIL,CRED_PASS};
enum FILE_STATE {FILE_NOTFOUND,FILE_FOUND,FILE_NOTOPEN};

int passCred=CRED_FAIL;





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
							subfolder_location,//Sub folder locatio for PUT and GET 
							
							
						}COMMAND_LC;// Resource format

// For Server Client Packet request/data Format and location 
typedef enum PACKETLOCATION{
							requestExtra,//Extra Character
							DFCUserloc,//User Location
							DFCPassloc,//Password Location
							DFCCommandloc,//Command Location
							DFCFileloc,//File1  Location
							DFCDataloc,//Data Location
							DFCFile2loc,//File 2 Location
							DFCData2loc,//Data 2 Location
							DFCSubFolderloc,//Sub folder location 
						}PACKET_LC;// Resource format


//Request/command exchnage format b/w DFS and DFC

struct requestCommandFmt{		
		char DFCRequestUser[MAXCOLSIZE];
		char DFCRequestPass[MAXCOLSIZE];
		char DFCRequestCommand[MAXCOLSIZE];
		char DFCRequestFile[MAXCOLSIZE];
		char DFCMergedFile[MAXCOLSIZE];
		char DFCRequestFolder[MAXCOLSIZE];
		int socket;
		int DFServerId;
};

//Data exchnage format b/w DFS and DFC

struct DataFmt{		
		char DFCRequestUser[MAXCOLSIZE];
		char DFCRequestPass[MAXCOLSIZE];
		char DFCRequestCommand[MAXCOLSIZE];
		char DFCRequestFile[MAXCOLSIZE];
		char DFCRequestFile2[MAXCOLSIZE];
		char DFCMergedFile[MAXCOLSIZE];
		char DFCData[MAXBUFSIZE];
		char DFCData2[MAXBUFSIZE];
		char DFCRequestFolder[MAXCOLSIZE];
		int socket;
		int DFServerId;
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
	sprintf(sendMessage,"%s|%s|%s|%s|%s|%s|%s|%s|",sendData.DFCRequestUser,sendData.DFCRequestPass,sendData.DFCRequestCommand,sendData.DFCRequestFile,sendData.DFCData,sendData.DFCRequestFile2,sendData.DFCData2,sendData.DFCRequestFolder);
	
	DEBUG_PRINT("Message to Server =>%s",sendMessage);
	write(sendData.socket,sendMessage,sizeof(sendMessage));		

}




int sendcommandToDFS (struct requestCommandFmt reqCommand)
{
	char sendMessage[MAXBUFSIZE];
	sprintf(sendMessage,"%s|%s|%s|%s|%s|",reqCommand.DFCRequestUser,reqCommand.DFCRequestPass,reqCommand.DFCRequestCommand,reqCommand.DFCMergedFile,reqCommand.DFCRequestFolder);
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
		DEBUG_PRINT	("%d : %s=>",sizeofip,splitop[sizeofip]);
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
	
	char cwd[1024];
	//chdir("/path/to/change/directory/to");
	getcwd(cwd, sizeof(cwd));
	DEBUG_PRINT("Current working dir: %s\n", cwd);


	//DEBUG_PRINT("File To be opened %s",cwd);
	//sprintf(filename,"%s%s",cwd,filename);
	//strcat(cwd,"/");
	//strcat(cwd,filename);
	DEBUG_PRINT("File=>%s=>",filename);
	FILE *fdesc = fopen(filename,"rb");
	if (fdesc ==NULL)
	{
		perror(cwd);//if can't open
		DEBUG_PRINT("File Not Found %s",filename);
		return -1;
	}

	MD5_Init(&mdCont);//Initialize 
	
	while((file_bytes = fread(tempdata,1,10,fdesc)) != 0){//Read data from files
		MD5_Update(&mdCont,tempdata,file_bytes);//Convert and update context 		
	}	
	MD5_Final(MD5_gen,&mdCont);
	fclose(fdesc);	

	for ( i=0;i<MD5_DIGEST_LENGTH;i++) {
		temp = MD5_gen[i];
		sprintf(MD5_result,"%x",((MD5_gen[i]&0xF0)>>4) );
		*MD5_result++;
		sprintf(MD5_result,"%x",((MD5_gen[i]&0x0F)) );
		*MD5_result++;
		
	   }
	DEBUG_PRINT("MD5 Gen => MD5 Result %s",MD5_gen,MD5_result);  
	return 0;
}





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
    FILE *sourceFile=NULL,*splitFile=NULL; //file and temp file

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
    bzero(destFileName,strlen(destFileName));
    sprintf(destFileName, "%s.%d", sourcefilename, i);// Format <sourcefilename>.<i> eg:#file.txt.1
    DEBUG_PRINT("Split File name %s=>",destFileName);
    splitFile = fopen(destFileName, "wb");//Open the destincation file
    if (splitFile==NULL)// if file closed or not
    {
        printf("couldn't open file");
        perror("Destination File Open");
        exit(-1);
    }
    
    while(i<=(parts))
    {
        ch = fgetc(sourceFile);    
        //if (ch==EOF)//check for end of file 
        //    break;
    
    
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
		    DEBUG_PRINT("Closing %s",destFileName);
		    if(splitFile!=NULL){
		    	
				fclose(splitFile);
				DEBUG_PRINT("Closed %s",destFileName);
			}


			if (i<=(parts)){
				DEBUG_PRINT("INto new file openeing");
	            sprintf(destFileName, "%s.%d", sourcefilename, i);// Format <sourcefilename>.<i> eg:#file.txt.1
	            splitFile=fopen(destFileName, "wb");
	        }
        }
    }
    if(sourceFile){
    	fclose(sourceFile);
    }	
   DEBUG_PRINT("after completed split %s File%s\n",action[command_location],action[file_location]); 										
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


    mergeFile=fopen(&mergedFileName[1], "wb");//Open destn Merge file name in write mode 
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
        //DEBUG_PRINT("%c",ch);	
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
	        if(ch=='\n'){
	        	fputc('\n',mergeFile);	   	//as fputc does not put a '\n' by itself
	        }	
	        //else
	        //{
	        //	fputc(ch,mergeFile);	   
	        //}
	    }
	    else
	    {
		    if(ch=='\n'){
	        	fputc('\n',mergeFile);	   	//as fputc does not put a '\n' by itself
	        }
	        else
	        {
	        	fputc(ch,mergeFile);
	        }


	    }
		
    }
    
    fputc('\n',mergeFile);// add extra new line at end of file as fputch does not add it by itself
    if(mergeFile){
    	fclose(mergeFile);
    }	
   

}




int writeFile(char *filename,char *dataInput){
	int fd =0;
	DEBUG_PRINT("Write File %s",filename);
	if((fd= open(filename,O_CREAT|O_RDWR|O_TRUNC,0666)) <0){//open a file to store the data in file
		perror(filename);//if can't open
		return FILE_NOTOPEN;
	}
	else // open file successfully ,read file 
	{
		//debug 
		DEBUG_PRINT("Data %s",dataInput);
		if(write(fd,dataInput,strlen(dataInput))<0){//write to file , with strlen as sizeof has larger value 
			perror("Error for writing to file");
			return FILE_NOTFOUND;	
		}
		else// when write has be done
		{	
			DEBUG_PRINT("Data written to %s",filename);
		}

		if(fd){
			DEBUG_PRINT("Close File");
			close(fd);
		}
	}
			
}


void rcvDFSFile(int socketID){

	char message_server[MAXBUFSIZE];
	int packetCheck=0;
	char (*packet)[MAXCOLSIZE];
	ssize_t read_bytes;
	int total_attr_commands=0,i=0;

	struct DataFmt datafromServer;
	//recieve data from each DFS Server 
	bzero(message_server,sizeof(message_server));
	if((read_bytes =recv(socketID,message_server,sizeof(message_server),0))>0){

		DEBUG_PRINT("Read Bytes %d",read_bytes);	
		//printf("request from client %s\n",message_client );
		//strcpy(message_bkp,message_server);//backup of orginal message 
		//DEBUG_PRINT("Check DFS=> %s \n",config.DFSdirectory );
		DEBUG_PRINT("Message from Client => %s \n",message_server );

		if ((strlen(message_server)>0) && (message_server[strlen(message_server)-1]=='\n')){
				message_server[strlen(message_server)-1]='\0';
		}

		//
		if ((packet=malloc(sizeof(packet)*MAXCOLSIZE))){	
			total_attr_commands=0;


			if((total_attr_commands=splitString(message_server,"|",packet,9)>0)){
					
					
					//copy contents to data structure of data struture 
					strcpy(datafromServer.DFCRequestUser,packet[DFCUserloc]);
					strcpy(datafromServer.DFCRequestPass,packet[DFCPassloc]);
					strcpy(datafromServer.DFCRequestCommand,packet[DFCCommandloc]);
					strcpy(datafromServer.DFCRequestFile,packet[DFCFileloc]);
					memcpy(datafromServer.DFCData,packet[DFCDataloc],sizeof(packet[DFCDataloc]));
					strcpy(datafromServer.DFCRequestFile2,packet[DFCFile2loc]);
					memcpy(datafromServer.DFCData2,packet[DFCData2loc],sizeof(packet[DFCData2loc]));

	   				DEBUG_PRINT("User %s ",datafromServer.DFCRequestUser);
	   				DEBUG_PRINT("Password %s",datafromServer.DFCRequestPass);
	   				DEBUG_PRINT("Command %s",datafromServer.DFCRequestCommand);
	   				DEBUG_PRINT("File 1 %s",datafromServer.DFCRequestFile);
	   				DEBUG_PRINT("Data 1 %s ",datafromServer.DFCData);
	   				DEBUG_PRINT("File 2 %s",datafromServer.DFCRequestFile2);	   				
	   				DEBUG_PRINT("Data 2 %s ",datafromServer.DFCData2);
	   				
	   				DEBUG_PRINT("Total Commands %d",total_attr_commands);
				
			}
			writeFile(datafromServer.DFCRequestFile,datafromServer.DFCData);
			writeFile(datafromServer.DFCRequestFile2,datafromServer.DFCData2);		

		}	


	}
	//Free the command allocated 
	DEBUG_PRINT("Deallocate packet ");
		if (packet!=NULL){
			//free alloaction of memory 
			for(i=0;i<total_attr_commands;i++){
				free((*packet)[i]);
			}
			free(packet);//clear  the request recieved 
		}

}




void rcvMain(){




}



void putDFSFile();

















void *DFSThreadServed(void *Id){



			int DFSId = (int)Id;
			int connect_sucess=0,i=0,k=0,temp=0;
			
			//MD5 Cal
			char * MD5file;
			char MD5_temp[MD5_DIGEST_LENGTH*2];
			
			
			MD5file = MD5_temp;
			
	
			DEBUG_PRINT("Command %s File%s\n",action[command_location],action[file_location]);	
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
		    DEBUG_PRINT("created SOCKet %d \n",sock[DFSId]);
			
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
				        perror("\nConnect failed. Error");
				        //exit(-1);
				        connect_sucess=0;
				        //return -1;// Return as Socket is not up 
				    }
				    else
				    {
				    	connect_sucess=1;
				    }
			 		
				    //i=0;
    		// for destnation 1
				DEBUG_PRINT("Check rotate dest 1");
				for (i=0 ;i<MAXDFSCOUNT;i++){
					DEBUG_PRINT("%d",partdest1[i]);
				}

			// for destnation 2 								

				DEBUG_PRINT("Check rotate dest 2");
				for (i=0 ;i<MAXDFSCOUNT;i++){
					DEBUG_PRINT("%d",partdest2[i]);
				}




			if (connect_sucess){	    
					DEBUG_PRINT("connected SOCKet %d",sock[DFSId]);
					//copy socket 
					requesttoserver.socket=datatoserver.socket=sock[DFSId];//assigning only server one socket for now
					requesttoserver.DFServerId=datatoserver.DFServerId=DFSId;
					DEBUG_PRINT("Command %s File%s\n",action[command_location],action[file_location]);
					//compare the input command and perform necessary action inside thread  		
					if ((strncmp(requesttoserver.DFCRequestCommand,"LIST",strlen("LIST"))==0)){
						//send command
						DEBUG_PRINT("Inside Thread %d LIST",(int)DFSId);
						strcpy(requesttoserver.DFCRequestCommand,"LIST");
						sendcommandToDFS(requesttoserver);
						DEBUG_PRINT("Waiting for List");
						bzero(list[list_count],sizeof(list[list_count]));
						if(nbytes = recv(sock[DFSId],list[list_count],sizeof(list[list_count]),0)<0){//recv from server and check for non-blocking 
							fprintf(stderr,"non-blocking socket not returning data  List %s\n",strerror(errno) );
						}
						
						DEBUG_PRINT("\n%s\n",list[list_count] );
						if (!strncmp(list[list_count],"Invalid",strlen("Invalid")))
						{
							printf("\n%s\n",list[list_count] );
							passCred=CRED_FAIL;
						}
						else{								
							DEBUG_PRINT("Last Filename in thread %d , list_count %d = >%s",(int)Id,list_count,list[list_count]);
							passCred=CRED_PASS;
							list_count++;
						}						

					}
					else if ((strncmp(requesttoserver.DFCRequestCommand,"GET",strlen("GET")))==0){								
						DEBUG_PRINT("Inside Thread %d GET",(int)DFSId);						
						sendcommandToDFS(requesttoserver);							
						rcvDFSFile(sock[DFSId]);
						passCred=CRED_PASS;//check if 
				  	}
				  	else if ((strncmp(requesttoserver.DFCRequestCommand,"PUT",strlen("PUT")))==0){
								
						DEBUG_PRINT("Inside Thread %d PUT",(int)DFSId);								
						putDFSFile();		

					}
					else if ((strncmp(action[command_location],"MKDIR",strlen("MKDIR"))==0))
					{

						DEBUG_PRINT("Inside Thread %d MKDIR",(int)DFSId);								
						sendcommandToDFS(requesttoserver);		
					}
					else
					{
						//should not reach 
						printf("incorrect Commnad ");
					}
				DEBUG_PRINT("UnLOcked Mutex");						
				//Close multiple Socket 
				shutdown (sock[DFSId], SHUT_RDWR);         //All further send and recieve operations are DISABLED...
		    	close(sock[DFSId]);
		    	sock[DFSId]=-1;	
			}		
			else
			{

				DEBUG_PRINT("No connection");
			}
			// Free MD5file if present 
	
		pthread_mutex_unlock(&thread_mutex);	
		
    
}


// Put files to location accodrding to MD5 has value 



void putDFSFile(){

	
	//long int MD5value=0,
	//char * MD5value;
	int k=0,i=0,tempdest;
	char tempFile1[MAXCOLSIZE];//temp file name for creating the split files 
	char tempFile2[MAXCOLSIZE];//temp file name for creating the split files 
	char tempFile3[MAXCOLSIZE];//temp file name for creating the split files 
	int fd1=0,fd2=0;
	char data[MAXPACKSIZE];
	int temp;
	ssize_t  file_bytes;
	char ACK_packet[MAXCOLSIZE];

		
	DEBUG_PRINT("Put DFS File");

// Send Files for each server according to each client 
		strcpy(tempFile3,datatoserver.DFCMergedFile); 

		bzero(tempFile1,strlen(tempFile1));
		sprintf(tempFile1,"%s.%d",tempFile3,partdest1[datatoserver.DFServerId]);
		DEBUG_PRINT("Temp File 1 %s",tempFile1);
		strcpy(datatoserver.DFCRequestFile,tempFile1); 

		

		//destication 2 
		//DEBUG_PRINT("Temp File 2 here %s=>%s",datatoserver.DFCRequestFile,tempFile2);
		bzero(tempFile2,strlen(tempFile2));
		sprintf(tempFile2,"%s.%d",tempFile3,partdest2[datatoserver.DFServerId]);
		DEBUG_PRINT("Temp File 2 %s",tempFile2);


		//open the file for this DFS dest 1
		if((fd1= open(datatoserver.DFCRequestFile,O_RDONLY)) <0 ){//open read only file 
			perror(datatoserver.DFCRequestFile);//if can't open
			//send to client the error
			DEBUG_PRINT("Error in File openeing %s",datatoserver.DFCRequestFile);
			//return -1;
		}
		else // open file successful ,read file 
		{
			//read bytes and send 
			bzero(data,sizeof(data));
			if(file_bytes = read(fd1,data,MAXPACKSIZE) > 0){//read data from file 	
				//DEBUG_PRINT("Error in File openeing =>%s=>",datatoserver.DFCRequestFile);	
				memcpy(datatoserver.DFCData,data,sizeof(data));
				//DEBUG_PRINT("Read data from%s=>%s ",datatoserver.DFCRequestFile,data);	
			}	
		}	
		
		strcpy(datatoserver.DFCRequestFile2,tempFile2);

		//open the file for this DFS dest 1
		if((fd2= open(datatoserver.DFCRequestFile2,O_RDONLY)) <0 ){//open read only file 
			perror(datatoserver.DFCRequestFile2);//if can't open
			//send to client the error
			DEBUG_PRINT("Error in File openeing %s",datatoserver.DFCRequestFile2);
			//return -1;
		}
		else // open file successful ,read file 
		{
			//read bytes and send 
			bzero(data,sizeof(data));
			if(file_bytes = read(fd2,data,MAXPACKSIZE) > 0){//read data from file 	
				//DEBUG_PRINT("Error in File openeing =>%s=>",datatoserver.DFCRequestFile);	
				memcpy(datatoserver.DFCData2,data,sizeof(data));
				//DEBUG_PRINT("Read data from%s=> ",datatoserver.DFCRequestFile,data);	
			}	
		}	

		sendDataToDFS(datatoserver);//send data to 		
		DEBUG_PRINT("Sending 1st and 2nd File to %s part %d",config.DFSName[datatoserver.DFServerId],partdest1[datatoserver.DFServerId],partdest2[datatoserver.DFServerId]);
		if(nbytes = recv(sock[datatoserver.DFServerId],ACK_packet,sizeof(ACK_packet),0)>0)	{	
			DEBUG_PRINT("\nACK Packet%s\n",ACK_packet );
			if(!strcmp(ACK_packet,"OK")){
				passCred =CRED_PASS;
			}
			else if (!strncmp(ACK_packet,"Invalid",strlen("Invalid"))){
				passCred =CRED_FAIL;	
				printf("%s\n",ACK_packet );
			}
			else
			{
				passCred =CRED_ERROR;		
				printf("%s\n",ACK_packet );
			}	
		}	
		
		/*
		
				//sleep(1);
				sendDataToDFS(datatoserver);//send data to 
				DEBUG_PRINT("Sending 2nd file to %s part %d",config.DFSName[datatoserver.DFServerId],partdest2[datatoserver.DFServerId]);
		*/
				/*
		DEBUG_PRINT("Waiting for ACK................");	
		bzero(ACK_packet,sizeof(ACK_packet));	
		//nbytes = recv(sock[datatoserver.DFServerId],ACK_packet,sizeof(ACK_packet),0);		
		do{
				if(!strncmp(ACK_packet,"ACK",strlen("ACK"))){
					DEBUG_PRINT("received ACK=> %s",ACK_packet);
					strcpy(datatoserver.DFCRequestFile,tempFile2);

					//open the file for this DFS dest 1
					if((fd2= open(datatoserver.DFCRequestFile,O_RDONLY)) <0 ){//open read only file 
						perror(datatoserver.DFCRequestFile);//if can't open
						//send to client the error
						DEBUG_PRINT("Error in File openeing %s",datatoserver.DFCRequestFile);
						//return -1;
					}
					else // open file successful ,read file 
					{
						//read bytes and send 
						bzero(data,sizeof(data));
						if(file_bytes = read(fd2,data,MAXPACKSIZE) > 0){//read data from file 	
							//DEBUG_PRINT("Error in File openeing =>%s=>",datatoserver.DFCRequestFile);	
							strcpy(datatoserver.DFCData,data);
						    DEBUG_PRINT("Read data from%s=> ",datatoserver.DFCRequestFile,data);	
						    sendDataToDFS(datatoserver);//send data to 
							DEBUG_PRINT("Sending 2nd file to %s part %d",config.DFSName[datatoserver.DFServerId],partdest2[datatoserver.DFServerId]);
							DEBUG_PRINT("Waiting for ACK");
							nbytes = recv(sock[datatoserver.DFServerId],ACK_packet,sizeof(ACK_packet),0);			
							if(!strncmp(ACK_packet,"ACK",strlen("ACK"))){
								DEBUG_PRINT("received ACK=> %s",ACK_packet);
								break;//ACK recived and next file send 	
							}	
						}	
						
					}	
					//sleep(1);
					
				}	
		}while(nbytes = recv(sock[datatoserver.DFServerId],ACK_packet,sizeof(ACK_packet),0)>0);		
		*/
			if(fd1){
				if(close(fd1)<0){
					perror("fd1");
				}
			}
			if(fd2){
				if(close(fd2)<0){
					perror("fd2");
				}
			}
			
			DEBUG_PRINT("Closed the files");

}


// A list recv function for main 
// after all threads have received the 
int listMainRcv()
{
	char list_temp[MAXFILESCOUNT][MAXLISTSIZE];
	int i=0,j=0,k=0,l=0,a=0,f=0,total_files_rxcv,temp_locn,total_disp_files=0; 
	char file_ext[MAXACKSIZE];
	char filename[MAXFILESCOUNT][MAXLISTSIZE];
	char partfilename[MAXFILESCOUNT][MAXLISTSIZE];
	char display_list[MAXFILESCOUNT][MAXLISTSIZE];
	char lastTemp[10];
	char *lastptr=&lastTemp;
	char *filename_temp=NULL;
	DEBUG_PRINT("Complete List RCVD");	
	int nofiles=0;					
	int duplicate_flag=0,final_list=0, partflags[MAXFILESCOUNT][MAXDFSCOUNT];


	//clear the arrays 
	//bzero(display_list,strlen(display_list));
	for (i=0;i<MAXFILESCOUNT;i++){	
		bzero(partflags[i],strlen(partflags[i]));
		bzero(partfilename[i],strlen(partfilename[i]));
		bzero(display_list[i],strlen(display_list[i]));
		bzero(filename[i],strlen(filename[i]));
		bzero(list_temp[i],strlen(list_temp[i]));
	}

	for (i=0;i<MAXDFSCOUNT;i++){	
		DEBUG_PRINT(" %s",list[i]);

		if(strlen(list[i])>2)
		{	
			DEBUG_PRINT("Lenght %d",strlen(list[i]));
			nofiles++;//check if list is enpty or not 
		}
	}
	DEBUG_PRINT("Check count %d",nofiles);
	//for presentation purposes five files 
	i=0;
	j=0;
	l=0;
	k=0;
	DEBUG_PRINT("Split files from differet serveres");				
	
	if(!nofiles){
		printf("NO files on DFS\n");
		return -1;

	}
	DEBUG_PRINT("Split files ");				
	while(i < MAXDFSCOUNT){						

		if (list[i][j] == ' '){			
			list_temp[k][j]='\0';					
			if (list[i][j+1] == '#'){
				i++;			
				j=0;	
				
			}
			else{				
				j++;
			}	
			l=0;
			k++;	
		}
		else
		{					
			list_temp[k][l]=list[i][j];			
			j++;
			l++;
		}		
		//DEBUG_PRINT("Check for split %d",i);
	}	

	
	total_files_rxcv =k;
	DEBUG_PRINT("total files separated %d",total_files_rxcv);	

	//Display all files rcved
	j=i=k=0;
	DEBUG_PRINT("Display list %d",total_files_rxcv);	
	for (k=0;k<total_files_rxcv;k++){
		DEBUG_PRINT("%s",&list_temp[k][1]);	
	}


	//check unique and merge file names 
	j=i=k=f=0;

	DEBUG_PRINT("temp var values %d  %d %d",i,j,k);	
	while(j<total_files_rxcv)
	{
				if ((filename_temp=(char*)malloc(sizeof(char)*MAXLISTSIZE))<0)
				{
					DEBUG_PRINT("Cant Allocate space\n");
					break;
				}
				bzero(filename_temp,sizeof(filename_temp));
				strncpy(filename_temp,&list_temp[j][1],strlen(list_temp[j]));
				//filename_temp[strlen(&list_temp[j][1])]='\0';
				DEBUG_PRINT	("File temp %s \n",filename_temp);
				

				// to extract name of file and check if matches with other files and 
				lastptr = strrchr(filename_temp,'.');
				
				if(lastptr){
					*lastptr++;
					strcpy(file_ext,lastptr);
					DEBUG_PRINT("Extension %s",file_ext);
					temp_locn=strlen(filename_temp)- strlen(file_ext) -1;
					bzero(filename[f],sizeof(filename[f]));
					strncpy( filename[f], filename_temp,temp_locn);
					filename[f][temp_locn+1] = '\0';
					//check for duplicaates of same file and keep only unique file names 
						for (a=0;a<total_files_rxcv;a++){
							if (!strcmp(filename[f],filename[a]) && a<f ){// check if filename matches with previous file namme
								duplicate_flag=1;
								DEBUG_PRINT("break up");
								break;
							}					
						}
						if (duplicate_flag){						
							duplicate_flag=0;
							DEBUG_PRINT("Found duplicate file name ");
						}
						else
						{

							f++;// update if not duplicate 
							DEBUG_PRINT("update f");
							DEBUG_PRINT("Extracted file %d=>%s",f-1,filename[f-1]);
						}
					
					//update the part flag which has been rcvd	
					//temp_locn=atoi(file_ext);
					//temp_locn-=48;
					//partflags[f-1][temp_locn]=1;
					//DEBUG_PRINT("part flag updated %d=> %d => %x , f=>%d",temp_locn,partflags[f-1][temp_locn],temp_locn,f-1);
						
					
					
				}
				else
				{
					
					perror("File not Found");
					DEBUG_PRINT("Could not find '.' ");
					//return -1;
				}
		j++;		
	}

	total_disp_files=f;

	//Debugging
	DEBUG_PRINT("All List of files");
	for (f=0;f<total_disp_files;f++){	
			 DEBUG_PRINT("%s",filename[f]);
	}

	//check if each part of file present or not 
	j=i=k=f=0;
	DEBUG_PRINT("Upating Part flags");
	while(j<total_files_rxcv){
				strncpy(filename_temp,list_temp[j],strlen(list_temp[j]));
				filename_temp[strlen(list_temp[j])]='\0';
				DEBUG_PRINT	("File temp %s \n",filename_temp);
				

				// to extract name of file and check if matches with other files and 
				lastptr = strrchr(filename_temp,'.');
				
				if(lastptr){
					*lastptr++;
					strcpy(file_ext,lastptr);
					DEBUG_PRINT("Extension %s",file_ext);
					temp_locn=strlen(filename_temp)- strlen(file_ext) -1;
					
					bzero(partfilename[f],sizeof(partfilename[f]));
					strncpy( partfilename[f], filename_temp,temp_locn);
					partfilename[f][temp_locn+1] = '\0';
					//check for duplicaates of same file and keep only unique file names 
					//	for (a=0;a<total_files_rxcv;a++){
					//		if (!strcmp(filename[f],filename[a]) && a<f ){// check if filename matches with previous file namme
					//			duplicate_flag=1;
					//			DEBUG_PRINT("break up");
					//			break;
					//			}					
					//	}
					//	if (duplicate_flag){						
					//		duplicate_flag=0;
					//		DEBUG_PRINT("Found duplicate file name ");
					//	}
					//	else
					//	{
					//
					//		f++;// update if not duplicate 

					//	}
					

					//update the part flag which has been rcvd	
					temp_locn=atoi(file_ext);
					//temp_locn-=48;
					//check which file name matches 
					for(k=0;k<total_disp_files;k++){
						DEBUG_PRINT("File =>%s , Orginal File=> %s ",partfilename[f],filename[k]);
						if (!(strncmp(&partfilename[f][1],filename[k],strlen(&partfilename[f][1])))){
							partflags[k][temp_locn]=1;
							DEBUG_PRINT("File %s part flag updated %d=> %d => %x , f=>%d",&partfilename[f][1],temp_locn,partflags[k][temp_locn],temp_locn,k);
							break;

						}else
						{
							DEBUG_PRINT("Didnt match");
						}
					}
					
				}
				else
				{
					
					perror("File not Found");
					DEBUG_PRINT("Could not find '.' ");
					//return -1;
				}
		j++;

	}


	//total_files_rxcv=k;

	


	j=0;
	DEBUG_PRINT("\nList of files\n");

	// List of files to be displayed 
	for (f=0;f<total_disp_files;f++){
			DEBUG_PRINT("Filename => %s",filename[j] );
			DEBUG_PRINT("Var f=>%d j=>%d file=>%s",f,j,filename[f]);
			 // check if all flag bits 
				for (i=1;i<=MAXDFSCOUNT;i++){
					DEBUG_PRINT("Part %d => %d ",i,partflags[f][i]);
					if (partflags[f][i]==0){
						DEBUG_PRINT("File Name %s => Part %d Not found ",filename[f],i);
						final_list=1;
						break;
					}

				}
				bzero(display_list[j],strlen(display_list[j]));
				strncpy(display_list[j],filename[f],strlen(filename[f]));
				
				if (final_list){
					final_list =0;
					strcat(display_list[j]," [incomplete]");
					DEBUG_PRINT("incomplete");
				}
				//DEBUG_PRINT("%s",display_list[j]);
				printf("\n%s\n",display_list[j] );
				j++;
	}
	if(filename_temp){
		free(filename_temp);
	}	
	DEBUG_PRINT("Return to main" );
	return 0;


}


int CheckSever(){

}

int FilePresent(char *fname){

		
       if (strlen(fname)<1){
       		return FILE_NOTFOUND;
       }


			FILE *fp = fopen (fname, "r");
			if (fp!=NULL) {
				fclose (fp);
				return FILE_FOUND ;
			}
			else{
				return FILE_NOTFOUND ;
			}	
			
}
/*
       if( access( fname, F_OK ) != -1 ) {
       		//FILE exists
       		DEBUG_PRINT("File is present %s",fname);

       } else {
       		DEBUG_PRINT("File not present %s",fname);
           return FILE_NOTFOUND ;
       }
*/


		
int main (int argc, char * argv[] ){
    int i=0,c=0,l=0;
   	int k=0,temp=0;
    char message[1000] , server_reply[2000];
	char request[MAXPACKSIZE];             //a request to store our received message
	
	long int x=0; //result of MD5 in integer 
	long int shiftvalue=0;
			
	pthread_t DFSThread[MAXDFSCOUNT];
	char *MD5file;
	char MD5_temp[MD5_DIGEST_LENGTH*2];
	char MD5filelast[MAXDFSCOUNT];
	int MD5length=0;
	MD5file = MD5_temp;
	passCred=CRED_FAIL;
	char temmpFileName[MAXCOLSIZE];
    int commandFilter=0,fileCheck=0;
    int fileFoundFlag=FILE_NOTFOUND;
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
		
	//check if document root user present 
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
	//MD5Cal("sample.txt",MD5file);
	DEBUG_PRINT("MD5 file value %s",MD5file);
	//mergeFile("sample.txt",4);
	//Initiate Mutex 
  	pthread_mutex_init(&thread_mutex,NULL);


	DEBUG_PRINT("Completed splitting File name");     
    //keep communicating with server depending on command entered 
	printf("Enter Command\n");
	printf("GET <Filename> - to get file from DFS\n");
	printf("PUT <Filename> - to put file to DFS \n");
	printf("LIST -list files on  DFS server\n");
	printf("MKDIR <folderName> -to create a directory(subfolder) on DFS\n");
	
		
	while(1)
	{//start of while 
			DEBUG_PRINT("Start oF Main");
			//Clear the command and request to Client 
			bzero(requesttoserver.DFCRequestFile,sizeof(requesttoserver.DFCRequestFile));
			bzero(requesttoserver.DFCMergedFile,sizeof(requesttoserver.DFCMergedFile));
			bzero(requesttoserver.DFCRequestFile,sizeof(requesttoserver.DFCRequestCommand));
			bzero(requesttoserver.DFCRequestFolder,sizeof(requesttoserver.DFCRequestFolder));
			bzero(requesttoserver.DFCMergedFile,sizeof(requesttoserver.DFCMergedFile));



			bzero(datatoserver.DFCRequestCommand,sizeof(datatoserver.DFCRequestCommand));
			bzero(datatoserver.DFCRequestFile,sizeof(datatoserver.DFCRequestFile));
			bzero(datatoserver.DFCData,sizeof(datatoserver.DFCData));
			bzero(datatoserver.DFCRequestFile2,sizeof(datatoserver.DFCRequestFile2));
			bzero(datatoserver.DFCData2,sizeof(datatoserver.DFCData2));
			bzero(datatoserver.DFCRequestFolder,sizeof(datatoserver.DFCRequestFolder));


			bzero(command,sizeof(command));

			partdest1[0]=1;
			partdest1[1]=2;
			partdest1[2]=3;
			partdest1[3]=4;
			partdest2[0]=2;
			partdest2[1]=3;
			partdest2[2]=4;
			partdest2[3]=1;

			//partdest2[MAXDFSCOUNT]={2,3,4,1};

			//wait for input from user 
			fgets(command,MAXCOMMANDSIZE,stdin);
			if ((strlen(command)>0) && (command[strlen(command)-1]=='\n')){
					command[strlen(command)-1]='\0';
			}	


			//bzero(ack_recv,sizeof(ack_recv));	
			//ack_bytes=recvfrom(sock,ack_recv,sizeof(ack_recv),0,(struct sockaddr*)&remote,&addr_length);
			strcat(command,"\0");
			DEBUG_PRINT("Command %s",command);
			int cmpvalue=100;

			//Split the input from user 
			if ((action=malloc(sizeof(action)*MAXCOLSIZE))){	
				total_attr_commands=0;
				if((total_attr_commands=splitString(command," ",action,4)>0)) {
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

			strcpy(datatoserver.DFCRequestCommand,action[command_location]);
			
			strcpy(requesttoserver.DFCRequestCommand,action[command_location]);



						

			//IN main depending on Command 

					DEBUG_PRINT("Command %s => length %d",action[command_location]);
	   				DEBUG_PRINT("File(iF present) => %s",action[file_location]);
	   				DEBUG_PRINT("Total attributes of input  => %d",total_attr_commands);
					
					if ((strncmp(action[command_location],"LIST",strlen("LIST"))==0)){
								DEBUG_PRINT("Inside LIST");
								fileFoundFlag = FILE_FOUND;
								if (action[file_location]!=NULL){//check if present then taken as folder name
									strcpy(requesttoserver.DFCRequestFolder,action[file_location]);
								}

					}
					else if ((strncmp(action[command_location],"GET",strlen("GET")))==0){
							DEBUG_PRINT("Inside GET");
							DEBUG_PRINT("File Name %s",action[file_location]);
							//copy the file name as check if file exist on Server side 
							strcpy(datatoserver.DFCMergedFile,action[file_location]);
							strcpy(requesttoserver.DFCMergedFile,action[file_location]);
							fileFoundFlag = FILE_FOUND;


							if (action[subfolder_location]!=NULL){//check if present then taken as folder name
								strncpy(requesttoserver.DFCRequestFolder,action[subfolder_location],sizeof(action[subfolder_location]));
								strncpy(datatoserver.DFCRequestFolder,action[subfolder_location],sizeof(action[subfolder_location]));
							}


			  		}

					else if ((strncmp(action[command_location],"PUT",strlen("PUT")))==0){
							DEBUG_PRINT("Inside PUT");
							DEBUG_PRINT("File Name %s",action[file_location]);
							fileFoundFlag=FilePresent(action[file_location]);							


							DEBUG_PRINT("File Found %d",fileFoundFlag);
							if(fileFoundFlag==FILE_FOUND){

								//copy only if file exist on Client side 	
								strcpy(datatoserver.DFCMergedFile,action[file_location]);
								strcpy(requesttoserver.DFCMergedFile,action[file_location]);


								if (action[subfolder_location]!=NULL){//check if present then taken as folder name
									strcpy(requesttoserver.DFCRequestFolder,action[subfolder_location]);
									strcpy(datatoserver.DFCRequestFolder,action[subfolder_location]);
								}


								DEBUG_PRINT("after strcpy Command=>%s File=>%s FOlder=>%s\n",action[command_location],action[file_location],datatoserver.DFCRequestFolder); 		
							
							
									MD5Cal(datatoserver.DFCMergedFile,MD5file);
									DEBUG_PRINT("MD5 Allocated");
									DEBUG_PRINT("MD5 value File=>%s , %s ",datatoserver.DFCMergedFile,MD5file);
										//x=atoi(MD5file);
									MD5length = strlen(MD5file);
									MD5length -=2; 
									DEBUG_PRINT("MD5 File length=>%d",MD5length);		
									strcpy(MD5filelast,&MD5file[MD5length])	;  
									DEBUG_PRINT("MD5 File=>%s",MD5filelast);	
									x=(int)strtol(MD5filelast, NULL, 16);
									DEBUG_PRINT("MD5 int using strol value %d",x);	
									x= x % 4;
									DEBUG_PRINT("MD5 modulo 4 value %ld",x);		
							
								//}
								DEBUG_PRINT("after MD5 Command DFC %s File%s\n",datatoserver.DFCRequestCommand, datatoserver.DFCMergedFile); 										
								DEBUG_PRINT("after MD5 Command %s File%s\n",action[command_location],action[file_location]); 										
								shiftvalue =x ;
								if (x==1){
									shiftvalue=3;
								}
								else if (x==3)
								{
									shiftvalue=1;	
								}
								DEBUG_PRINT("shift value %d",shiftvalue);		
								

								DEBUG_PRINT("original dest 1");
								for (i=0 ;i<MAXDFSCOUNT;i++){
									DEBUG_PRINT("%d",partdest1[i]);
								}

								DEBUG_PRINT("original dest 2");
								for (i=0 ;i<MAXDFSCOUNT;i++){
									DEBUG_PRINT("%d",partdest2[i]);
								}

								//run i from 0 to shiftvalue times 

								for (i=0 ;i<=shiftvalue-1;i++){
									temp=partdest1[0];
									for (k=0;k<MAXDFSCOUNT-1;k++){
										//temp=partdest1[k+1];
										partdest1[k]= partdest1[k+1];
									}
									partdest1[k]= temp;
								}

								DEBUG_PRINT("Check rotate dest 1");
								for (i=0 ;i<MAXDFSCOUNT;i++){
									DEBUG_PRINT("%d",partdest1[i]);
								}

							// for destnation 2 
								
								for (i=0 ;i<=shiftvalue-1;i++){
									temp=partdest2[0];
									for (k=0;k<MAXDFSCOUNT-1;k++){
										//temp=partdest1[k+1];
										partdest2[k]= partdest2[k+1];
									}
									partdest2[k]= temp;

								}
								

								DEBUG_PRINT("Check rotate dest 2");
								for (i=0 ;i<MAXDFSCOUNT;i++){
									DEBUG_PRINT("%d",partdest2[i]);
								}

								DEBUG_PRINT("Split file %s into %d",datatoserver.DFCMergedFile,MAXDFSCOUNT);
								//split the file into MAXDFSCOUNT parts 
								splitFile(datatoserver.DFCMergedFile,MAXDFSCOUNT);
								
								DEBUG_PRINT("Files are split\n");
								DEBUG_PRINT("after Split%s File%s\n",datatoserver.DFCRequestCommand, datatoserver.DFCMergedFile); 									
							}
							else
							{
								printf("File Not Found\n");
								fileFoundFlag = FILE_NOTFOUND;
							}
			  		}
			  		else if((strncmp(action[command_location],"MKDIR",strlen("MKDIR"))==0))
			  		{
			  			DEBUG_PRINT("Inside MKDIR");
						DEBUG_PRINT("FOlder Name %s",action[file_location]);
						if (action[subfolder_location]!=NULL){//check if present then taken as folder name
								strncpy(requesttoserver.DFCRequestFolder,action[file_location],sizeof(action[file_location]));
								strncpy(datatoserver.DFCRequestFolder,action[file_location],sizeof(action[file_location]));
						}
						fileFoundFlag = FILE_FOUND;


			  		}	
			  		else
			  		{	
			  			commandFilter=1;
			  			fileFoundFlag = FILE_NOTFOUND;
			  			DEBUG_PRINT("Input in correct \n");

			  		}	


			 DEBUG_PRINT("last Command %s File%s\n",action[command_location],action[file_location]); 		
			 
			 DEBUG_PRINT("COMMAND %d, FILE FOUND %d",commandFilter,fileFoundFlag);
			if(!commandFilter && fileFoundFlag == FILE_FOUND){
				//Create the thread for all the communication with DFS Servers
				for (c=0;c<MAXDFSCOUNT;c++ ){
					//DEBUG_PRINT("DFS Thread %d",i);
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





						DEBUG_PRINT("Command =>%s ",action[command_location]);
		   				DEBUG_PRINT("File(iF present) => %s",action[file_location]);
		   				DEBUG_PRINT("Total attributes of input  => %d",total_attr_commands);
		   				DEBUG_PRINT("PASS CRED  => %d",passCred);
					if(passCred==CRED_PASS){		
						if ((strncmp(datatoserver.DFCRequestCommand,"LIST",strlen("LIST"))==0)){			

							if(listMainRcv()<0){
								DEBUG_PRINT("NO Files found");
							}
							else
							{
								DEBUG_PRINT("Complete LIST ");	
							}	
						}												
						else if ((strncmp(datatoserver.DFCRequestCommand,"GET",strlen("GET")))==0){
							DEBUG_PRINT("Inside GET after Thread");
							DEBUG_PRINT("File Name %s",action[file_location]);
							sprintf(temmpFileName,".%s",action[file_location]);
							DEBUG_PRINT("File to be merged  %s",action[file_location]);	
							mergeFile(temmpFileName,MAXDFSCOUNT);
							
				  		}

						else if ((strncmp(datatoserver.DFCRequestCommand,"PUT",strlen("PUT")))==0){
							DEBUG_PRINT("Inside PUT after Thread");
							DEBUG_PRINT("File Name %s",action[file_location]);						

				  		}
				  		else
				  		{
				  			DEBUG_PRINT("Command not found ");

				  		}	
					}  	

				DEBUG_PRINT("Deallocate memory for command\n");
				DEBUG_PRINT("Deallocate action\n");
		}
		else
		{
            printf("Input Command incorrect.Please try again \n");
            commandFilter=0;
		}		
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
	





















	
	