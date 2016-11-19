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
#include <glob.h>

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
							DFCFile2loc,//Command Location
							DFCData2loc,//Data Location
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
		char DFCData[MAXBUFSIZE];
		char DFCRequestFile2[MAXCOLSIZE];
		char DFCData2[MAXBUFSIZE];
		int socket;
};

struct DataFmt datafromClient;// for data from client

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

enum FILE_STATE {FILE_NOTFOUND,FILE_FOUND,FILE_NOTOPEN};

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
		//DEBUG_PRINT	("%d : %s",sizeofip,splitop[sizeofip]);
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
		struct  dirent *listfiles;
		//ref: http://nxmnpg.lemoda.net/3/readdir
		listfiles= readdir(d);
		if (!listfiles){
			break;
		}
		
		if(!(strcmp(listfiles->d_name,".") == 0 || strcmp(listfiles->d_name,"..") ==0)){//excluding present and previous dir
			DEBUG_PRINT("File %s",listfiles->d_name);
			strcat(listtosend,listfiles->d_name);
			strcat(listtosend," ");
			
		}
	}
	//close the directory 
	if(closedir(d)){
		fprintf(stderr,"Error in closing directory %s\n",strerror(errno));
		//sendtoClient(strerror(errno),(ssize_t)sizeof(strerror(errno)),NOACK);
		return -1;
	}
	

	DEBUG_PRINT(" socket => %d ",listsock);
	DEBUG_PRINT(" directory=> %s send => %s,",directory,listtosend);
	//Send the files to client 
	//Rectify the 
	strcat(listtosend,"#");// end of packet 
	DEBUG_PRINT("LIST send %s",listtosend);
	if((send(listsock,listtosend,strlen(listtosend),0))<0)		
	{
		fprintf(stderr,"Error in sending to clinet in lsit send %s\n",strerror(errno));
	}
	
	//sendtoClient(listtosend,(ssize_t)sizeof(listtosend));
	if(listtosend){
		free(listtosend);
	}	
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

int rcvFile (char *filename,char *dataInput){
	
	int fd; // File decsriptor 
	ssize_t file_bytes=0;
	int file_size=0;
	int rcount=0;//count for send count 
	char tempFile[MAXCOLSIZE];	
	int  rcvd_bytes;
	
	//switch directory according to user name 
	char cwd[1024];

	//Append a "." for indication of piece file  
	bzero(tempFile,sizeof(tempFile));
	
	sprintf(tempFile,".%s",filename);
	strcpy(filename,tempFile);
    bzero(cwd,sizeof(cwd));
    DEBUG_PRINT("Before cwd : %s\n", cwd);

    getcwd(cwd, sizeof(cwd));
    DEBUG_PRINT("Current working dir: %s\n", cwd);
    DEBUG_PRINT("Config dir: %s\n", config.DFSdirectory);
    DEBUG_PRINT("Request User %s\n", datafromClient.DFCRequestUser);
    DEBUG_PRINT("Request File %s\n",filename);
    sprintf(cwd,"%s%s/%s/%s",cwd,config.DFSdirectory,datafromClient.DFCRequestUser,filename);
    DEBUG_PRINT("Change working dir: %s\n", cwd);
    chdir(cwd);
	strcpy(filename,cwd);    
    //getcwd(cwd, sizeof(cwd));
    //DEBUG_PRINT("New working dir: %s\n", cwd);
    //DEBUG_PRINT("%s",)
	if((fd= open(filename,O_CREAT|O_RDWR|O_TRUNC,0666)) <0){//open a file to store the data in file
		perror(filename);//if can't open
		return -1;
	}
	else // open file successfully ,read file 
	{
		//debug 
		DEBUG_PRINT("Data %s",dataInput);
		if(write(fd,dataInput,strlen(dataInput))<0){//write to file , with strlen as sizeof has larger value 
					perror("Error for writing to file");
					rcount =-1;	
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

	bzero(tempFile,sizeof(tempFile));
	bzero(filename,sizeof(filename));
	return rcount;
}

//Send data to different DFS
int sendDataToClient (struct DataFmt sendData)
{

	char sendMessage[MAXBUFSIZE];
	sprintf(sendMessage,"%s/%s/%s/%s/%s/%s/%s",sendData.DFCRequestUser,sendData.DFCRequestPass,sendData.DFCRequestCommand,sendData.DFCRequestFile,sendData.DFCData,sendData.DFCRequestFile2,sendData.DFCData2);
	//DEBUG_PRINT("Data to File => %s => dest %s => %s",sendData.DFCRequestFile,config.DFSName[sendData.DFServerId],sendData.DFCData,sendData.DFCData2);
	DEBUG_PRINT("Message to Server =>%s",sendMessage);
	write(sendData.socket,sendMessage,sizeof(sendMessage));		

}


/*************************************************************
// Send a file from Server to Client
I/p : filename - File name to be pused to client 
o/p : find and file is pushed to client 
	  return number of status
Reference for understanding :
https://lms.ksu.edu.sa/bbcswebdav/users/mdahshan/Courses/CEN463/Course-Notes/07-file_transfer_ex.pdf	  
**************************************************************/

int sendFile (char *filename,int socket){
	
	int fd; // File decsriptor 
	ssize_t file_bytes,file_size;
	char data[MAXDFSCOUNT][MAXBUFSIZE];
	int fileStatus=FILE_NOTFOUND,i=0,j=0;
	char cwd[MAXPACKSIZE];
	char filetemp[MAXPACKSIZE];
	char filetoSend[MAXDFSCOUNT][MAXCOLSIZE];
	char filetoRead[MAXDFSCOUNT][MAXCOLSIZE];
	char *ret=NULL;
	int countFiles=0;
	int fileRet=0;


	struct DataFmt datatoClient;


	bzero(datatoClient.DFCRequestUser,sizeof(datatoClient.DFCRequestUser));
	bzero(datatoClient.DFCRequestPass,sizeof(datatoClient.DFCRequestUser));
	bzero(datatoClient.DFCRequestCommand,sizeof(datatoClient.DFCRequestUser));
	bzero(datatoClient.DFCRequestFile,sizeof(datatoClient.DFCRequestUser));
	bzero(datatoClient.DFCData,sizeof(datatoClient.DFCRequestUser));
	bzero(datatoClient.DFCRequestFile2,sizeof(datatoClient.DFCRequestUser));
	bzero(datatoClient.DFCData2,sizeof(datatoClient.DFCRequestUser));

	DEBUG_PRINT("Before updating (Should be blank)");
	DEBUG_PRINT("User %s ",datatoClient.DFCRequestUser);
	DEBUG_PRINT("Password %s",datatoClient.DFCRequestPass);
	DEBUG_PRINT("Command %s",datatoClient.DFCRequestCommand);
	DEBUG_PRINT("File 1 %s",datatoClient.DFCRequestFile);
	DEBUG_PRINT("Data 1 %s ",datatoClient.DFCData);
	DEBUG_PRINT("File 2 %s",datatoClient.DFCRequestFile2);	   				
	DEBUG_PRINT("Data 2 %s ",datatoClient.DFCData2);


	glob_t  globbuf;
	//
	DEBUG_PRINT("File to Find %s",filename);
	sprintf(filetemp,".%s.",filename);

	DEBUG_PRINT("Append piece format %s",filename);
	strcpy(filename,filetemp);



	getcwd(cwd, sizeof(cwd));
    DEBUG_PRINT("Current working dir: %s\n", cwd);
    DEBUG_PRINT("Config dir: %s\n", config.DFSdirectory);
    DEBUG_PRINT("Request User %s\n", datafromClient.DFCRequestUser);
    DEBUG_PRINT("Request Part File %s\n",filename);
    sprintf(cwd,"%s%s/%s/%s*",cwd,config.DFSdirectory,datafromClient.DFCRequestUser,filename);
    DEBUG_PRINT("Search in working dir: %s\n", cwd);
    //chdir(cwd);

	strcpy(filename,cwd);    
    //getcwd(cwd, sizeof(cwd))

	//seraching for partial file 
		DEBUG_PRINT("Var i=>%d, countFiles=>%d\n",i,countFiles);

    glob( filename, 0, NULL, &globbuf);
    if ( globbuf.gl_pathc == 0 ){
        DEBUG_PRINT("there were no matching files\n");
    	return FILE_NOTFOUND;
    }
    else{
        DEBUG_PRINT("the part FIles found  : 0=>%s\n 1=>%s\n", globbuf.gl_pathv[0],globbuf.gl_pathv[1]);
        while(i<globbuf.gl_pathc && countFiles <(MAXDFSCOUNT/2) ){		        
	        strcpy(filetoRead[countFiles],globbuf.gl_pathv[i]);
	        //sscanf(filetoSend[countFiles],"*.%s.txt.*",filetoRead[countFiles]);		        		        		       
	        ret = strstr(filetoRead[countFiles], ".");
	        strcpy(filetoSend[countFiles],ret);
	        DEBUG_PRINT("File Location =>%s,FIle to send =>%s\n", filetoRead[countFiles],filetoSend[countFiles]);
	        countFiles++;
	        i++;
	   	 }
	}   
	globfree(&globbuf);
    //Complete search for files 
	if(countFiles){

		strcpy(datatoClient.DFCRequestUser,datafromClient.DFCRequestUser);
		strcpy(datatoClient.DFCRequestPass,datafromClient.DFCRequestPass);
		strcpy(datatoClient.DFCRequestCommand,datafromClient.DFCRequestCommand);
		

		//open the file
		while(j<countFiles){		
			if((fd= open(filetoRead[j],O_RDONLY)) <0 ){//open read only file 
				perror(filename);//if can't open
				//send to client the error 
				DEBUG_PRINT("Cant Open File %s",filetoRead[j]);
				//sendtoClient(err_indication,sizeof(err_indication),NOACK);
				fileRet = FILE_NOTOPEN;
				//return -1;
			}
			else // open file successfully ,read file 
			{
				//debug 
				//printf("Sending %s\n",filename);
				//read bytes and send 
				fileRet = FILE_FOUND;
				if (file_bytes = read(fd,data[j],MAXPACKSIZE) >0 ){//read data from file 							
					file_bytes =0;

				}
				
			}
			if (fd){
				close(fd);//close file if opened 
			}
			DEBUG_PRINT("Data %d %s",j,data[j]);
			j++;
		}	
		strcpy(datatoClient.DFCRequestFile,filetoSend[0]);	
		strcpy(datatoClient.DFCRequestFile2,filetoSend[1]);
		strcpy(datatoClient.DFCData,data[0]);	
		strcpy(datatoClient.DFCData2,data[1]);
	}
	else{
		fileRet = FILE_NOTFOUND;
	}	

	DEBUG_PRINT("After updating (Should be Complete)");
	DEBUG_PRINT("User %s ",datatoClient.DFCRequestUser);
	DEBUG_PRINT("Password %s",datatoClient.DFCRequestPass);
	DEBUG_PRINT("Command %s",datatoClient.DFCRequestCommand);
	DEBUG_PRINT("File 1 %s",datatoClient.DFCRequestFile);
	DEBUG_PRINT("Data 1 %s ",datatoClient.DFCData);
	DEBUG_PRINT("File 2 %s",datatoClient.DFCRequestFile2);	   				
	DEBUG_PRINT("Data 2 %s ",datatoClient.DFCData2);

	//send  data requested back to client 
	if(fileRet==FILE_FOUND){
		DEBUG_PRINT("Send Data to Client");
		datatoClient.socket=socket;
		sendDataToClient(datatoClient);
	}


	//printf("No of packets send %d\n",scount );
	return fileRet;
	
}


//Client connection for each client 
/*
void *client_connections(void *client_sock_id){

	DEBUG_PRINT("In %d",client_sock_id);
}
*/

char (*action)[MAXCOLSIZE];

enum ERR_STATE {CRED_ERROR,CRED_FAIL,CRED_PASS};

int credCheck(){

	int passCred=CRED_ERROR;//Inital Value 
	int userid=-1,i=0;//location for user 

	//Check the list from config if user is available according data from Client
	while(i<maxtypesupported){//check the values within max supported 
		if(!(strcmp(datafromClient.DFCRequestUser,config.DFCUsername[i]))) {//check username one by one 
			userid=i;
			DEBUG_PRINT("Found Username @%d",userid);
			break;
		}
		else
		{
			DEBUG_PRINT("Username Not Found @%d",i++);
		}
	
	}

	if(userid>=0){//check if username location found
		if(!(strcmp(datafromClient.DFCRequestPass,config.DFCPassword[userid]))){
			passCred = CRED_PASS;				//user and password both matched 
			DEBUG_PRINT("Password  Macthed");
		}
		else
		{
			passCred = CRED_FAIL;					//user name matched , nut password didnt match 
			DEBUG_PRINT("Password NOt Macthed");
		}	
	}
	else{
		passCred = CRED_FAIL;			//Didnt find the user 
		DEBUG_PRINT("Didnt FInd User");
	}

	return passCred;

}






void *client_connections(void *client_sock_id)
//void client_connections (int client_sock_id)
{
	
	
	int total_attr_commands=0,i=0;
	int thread_sock = (int*)(client_sock_id);
	ssize_t read_bytes=0;
	char message_client[MAXPACKSIZE];//store message from client 
	char message_client_file2[MAXPACKSIZE];//store message from client 
	char message_bkp[MAXPACKSIZE];//store message from client 
	char command[MAXPACKSIZE];
	char (*packet)[MAXCOLSIZE];
	char directory[MAXCOLSIZE]=".";
	DEBUG_PRINT("passed Client connection %d\n",(int)thread_sock);
	DEBUG_PRINT("in client connections dir : %s",directory);
		//Clear the command and request to Client 
		bzero(message_client,sizeof(message_client));

		datafromClient.socket=0;	
		datafromClient.socket=thread_sock;
		// Recieve the message from client  and reurn back to client 
		if((read_bytes =recv(thread_sock,message_client,sizeof(message_client),0))>0){

			DEBUG_PRINT("Read Bytes %d",read_bytes);	
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


				if((total_attr_commands=splitString(message_client,"/",packet,9)>0))
				{
					//copy contents to data structure of data struture 
					strcpy(datafromClient.DFCRequestUser,packet[DFCUserloc]);
					strcpy(datafromClient.DFCRequestPass,packet[DFCPassloc]);
					strcpy(datafromClient.DFCRequestCommand,packet[DFCCommandloc]);
					strcpy(datafromClient.DFCRequestFile,packet[DFCFileloc]);
					strcpy(datafromClient.DFCData,packet[DFCDataloc]);
					strcpy(datafromClient.DFCRequestFile2,packet[DFCFile2loc]);
					strcpy(datafromClient.DFCData2,packet[DFCData2loc]);

	   				DEBUG_PRINT("User %s ",datafromClient.DFCRequestUser);
	   				DEBUG_PRINT("Password %s",datafromClient.DFCRequestPass);
	   				DEBUG_PRINT("Command %s",datafromClient.DFCRequestCommand);
	   				DEBUG_PRINT("File 1 %s",datafromClient.DFCRequestFile);
	   				DEBUG_PRINT("Data 1 %s ",datafromClient.DFCData);
	   				DEBUG_PRINT("File 2 %s",datafromClient.DFCRequestFile2);	   				
	   				DEBUG_PRINT("Data 2 %s ",datafromClient.DFCData2);
	   				
	   				//DEBUG_PRINT("Total Commands %d",total_attr_commands);
					
					if ((strncmp(datafromClient.DFCRequestCommand,"LIST",strlen("LIST"))==0)){
						//send command
						DEBUG_PRINT("Inside LIST");
						//bzero(directory,strlen(directory));
						strncat(directory,config.DFSdirectory,strlen(config.DFSdirectory));
						strcat(directory,"/");
						strncat(directory,packet[DFCUserloc],strlen(packet[DFCUserloc]));
						strcat(directory,"/");
						DEBUG_PRINT("directory : %s",directory);
						//strncpy(directory,packet[DFCUserloc]);

						//Check for Credentials
						if(credCheck()==CRED_PASS){
							DEBUG_PRINT("Send to LIST fundtion");
							list(directory,thread_sock);

						}
						else							
						{
							DEBUG_PRINT("Send to Invalid Erro");
							if((send(thread_sock,"Invalid Username/Password.Please try Again",strlen("Invalid Username/Password.Please try Again"),0))<0)		
							{
								fprintf(stderr,"Error in sending to clinet in lsit send %s\n",strerror(errno));
							}
	
						}	
										
					}
					else if ((strncmp(datafromClient.DFCRequestCommand,"GET",strlen("GET")))==0){
							DEBUG_PRINT("Inside GET");
							DEBUG_PRINT("File Name %s",packet[DFCFileloc]);

							if(credCheck()==CRED_PASS){
								DEBUG_PRINT("receive Files");
								sendFile(datafromClient.DFCRequestFile,thread_sock);//Recieve the part files 										
								if((send(thread_sock,"OK",strlen("OK"),0))<0)		
								{
									fprintf(stderr,"Error in sending to clinet in lsit send %s\n",strerror(errno));
								}
							}
							else							
							{
								DEBUG_PRINT("Username and Password Didnt match,send error");
								if((write(thread_sock,"Invalid Username/Password.Please try Again",strlen("Invalid Username/Password.Please try Again")))<0)		
								{
									fprintf(stderr,"Error in sending to clinet in lsit send %s\n",strerror(errno));
								}
		
							}
			  		}

					else if ((strncmp(datafromClient.DFCRequestCommand,"PUT",strlen("PUT")))==0){
							DEBUG_PRINT("Inside PUT 1st time");
							DEBUG_PRINT("File Name %s",datafromClient.DFCRequestFile);
							if(credCheck()==CRED_PASS){
								DEBUG_PRINT("receive Files");
								rcvFile(datafromClient.DFCRequestFile,datafromClient.DFCData);//Recieve the part files 		
								rcvFile(datafromClient.DFCRequestFile2,datafromClient.DFCData2);	
								if((send(thread_sock,"OK",strlen("OK"),0))<0)		
								{
									fprintf(stderr,"Error in sending to clinet in lsit send %s\n",strerror(errno));
								}
							}
							else							
							{
								DEBUG_PRINT("Username and Password Didnt match,send error");
								if((write(thread_sock,"Invalid Username/Password.Please try Again",strlen("Invalid Username/Password.Please try Again")))<0)		
								{
									fprintf(stderr,"Error in sending to clinet in lsit send %s\n",strerror(errno));
								}
		
							}	
												   		
			  		}
			  		else
			  		{
			  			printf("Incorrect Command  \n");
		  				if((send(thread_sock,"Incorrect Command",strlen("Incorrect Command"),0))<0)		
						{
							fprintf(stderr,"Error in sending to clinet in lsit send %s\n",strerror(errno));
						}



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
	while(1){
		//Accept incoming connections 
		datafromClient.socket=0;
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

