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
//int sock;                               //this will be our socket
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
#define MAXFILESPLITSIZE MAXFILESIZE/4


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
Split Files into multiple files 
i/p - Filename i/p to 
	-   	
Ref for understanding :http://stackoverflow.com/questions/20759244/c-code-to-split-files
***************************************************************************************/
int mergeFile(char *sourcefilename,int parts)
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


//For Socket
int sock[MAXDFSCOUNT];//No(Sockets) =DFS Server availables
struct sockaddr_in server[MAXDFSCOUNT];

int main (int argc, char * argv[] ){
    int i=0;
    char message[1000] , server_reply[2000];
	char request[MAXPACKSIZE];             //a request to store our received message
	
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
	}	

	//Create multiple Socket and connect them (TCP)
	for (i=0;i<MAXDFSCOUNT;i++){
	    if((sock[i] = socket(AF_INET , SOCK_STREAM , 0))<0)//create socket
	    {
	        
	        printf("Issue in Creating Socket,Try Again !! %d\n",sock[i]);
	        perror("Socket --> Exit ");
	    	exit(-1);
	    }
	    DEBUG_PRINT("counter %d created SOCKet %d ",i,sock[i]);
		
	    //Connect the multiple sockets 
	    server[i].sin_addr.s_addr = inet_addr(config.DFSIP[i]);
	    server[i].sin_family = AF_INET;
	    //server[i].sin_port = htons( config.DFSPortNo[i] );
	    server[i].sin_port = htons(atoi(config.DFSPortNo[i]));        		//htons() sets the port # to network byte order
	    
	    //Connect to remote server
	    if (connect(sock[i] , (struct sockaddr *)&server[i] , sizeof(server[i])) < 0)
	    {
	        perror("Connect failed. Error");
	        exit(-1);
	    }
 		DEBUG_PRINT("connected SOCKet %d",sock[i]);
	   
    }
    DEBUG_PRINT("All Sockets created");
    DEBUG_PRINT("All Sockets connected");
    i=0;

    DEBUG_PRINT("Split File Name");

    splitFile("test.txt",4);

	DEBUG_PRINT("Completed splitting File name");     
    //keep communicating with server depending on command entered 
    while(1)
    {
    	        
        bzero(message,sizeof(message));
        sprintf(message,"Message to %s counter %d",config.DFSName[i],i); 
        DEBUG_PRINT("message to send : %s",message);
        //Send some data
        
        if( send(sock[i] , message , strlen(message) , 0) < 0)
        {
            DEBUG_PRINT("Send failed");
            return 1;
        }
         
        //Receive a reply from the server
		/*	        
        if( recv(sock[i%MAXDFSCOUNT] , server_reply , strlen(server_reply), 0) < 0)
        {
            DEBUG_PRINT("recv failed");
            break;
        }
         
        DEBUG_PRINT("Server reply :");
        DEBUG_PRINT("%s",server_reply);
        */
        if (i++ >= (MAXDFSCOUNT-1) ){
        	i=0;
        }
        
        
    }
     
    //Create multiple Socket and connect them (TCP)
	for (i=0;i<MAXDFSCOUNT;i++){ 
    	close(sock[i]);
	}

	DEBUG_PRINT("All Sockets are closed");
    return 0;
}


