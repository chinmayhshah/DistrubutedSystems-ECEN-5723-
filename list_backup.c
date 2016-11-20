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
	char *filename_temp;
	
	int nofiles=0;					
	int duplicate_flag=0,final_list=0, partflags[MAXFILESCOUNT][MAXDFSCOUNT];

	DEBUG_PRINT("Complete List RCVD");	
	DEBUG_PRINT("In main 1");

	//clear the arrays 
	//bzero(display_list,strlen(display_list));
	/*
	for (i=0;i<MAXFILESCOUNT;i++){	
		bzero(partflags[i],strlen(partflags[i]));
		bzero(partfilename[i],strlen(partfilename[i]));
		bzero(display_list[i],strlen(display_list[i]));
		bzero(filename[i],strlen(filename[i]));
		bzero(list_temp[i],strlen(list_temp[i]));
	}
	*/


	DEBUG_PRINT("In main 2");
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

	//Issue from here 
	//check unique and merge file names 
	j=i=k=f=0;

	DEBUG_PRINT("temp var values %d  %d %d",i,j,k);	
	while(j<total_files_rxcv)
	{	
				DEBUG_PRINT("temp var values %d  %d %d",i,j,k);	
				if ((filename_temp=(char*)calloc(MAXLISTSIZE,sizeof(char)))==NULL)
				{
					DEBUG_PRINT("Cant Allocate space\n");
					break;
				}
				bzero(filename_temp,sizeof(filename_temp));
				strncpy(filename_temp,&list_temp[j][1],strlen(&list_temp[j][1]));
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

/*
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
*/	
	return 0;
	

}
