#include<stdio.h>
#include<stdlib.h>
#include<string.h>
//#include<unistd.h> //for linux only
#include<io.h> //for windows only

//Macro's
#define MAXINODE 50 //we can create 50 files

#define READ 1
#define WRITE 2  //rw 3

#define MAXFILESIZE 2048 //maximum file size

//type of file
#define REGULAR 1
#define SPECIAL 2

//offset eg.lseek
#define START 0
#define CURRENT 1
#define END 2

// structure of superblock
typedef struct superblock
{
	int TotalInodes; // TotalInodes = 50
	int FreeInodes;
}SUPERBLOCK , *PSUPERBLOCK;

//structure of inode
typedef struct inode // size 86
{
	char FileName[50];
	int InodeNumber;
	int FileSize;		//2048kb
	int FileActualSize; //actual bytes present inside file
	int FileType;       //either regular or special
	char *Buffer;		//actual data goes inside buffer
	int LinkCount;		// linkcount always 1
	int ReferenceCount;	
	int Permission; 	// 1 = r , 2 = w , 3 = rw
	struct inode *next;	//self referncial structure
}INODE , *PINODE , **PPINODE;

//structure of FT
typedef struct filetable
{
	int readoffset;
	int writeoffset;
	int count;		//always 1
	int mode; 		// 1 2 3 
	PINODE ptrinode;  // FT structure points to inode[IIT]
} FILETABLE , *PFILETABLE;

/*
	why u created struct for single member
		we want to give name of structure as ufdt
*/
typedef struct ufdt
{
	PFILETABLE ptrfiletable;  //ufdt points to FT
}UFDT;

//Global
UFDT UFDTArr[MAXINODE];

SUPERBLOCK SUPERBLOCKobj; //object creation
PINODE head = NULL;		  // pointer creation global


void man( char *name )
{
	if( name == NULL ) return;

	if( strcmp(name , "create") == 0)
	{
		printf("Description : Used to create new regular file.\n");
		printf("Usage : create File_name Permission\n");
	}
	else if( strcmp( name , "read" ) == 0)
	{
		printf("Description : Used to read data from regular file.\n");
		printf("Usage : read File_name No_Of_Bytes_To_Read\n");	
	}
	else if( strcmp( name , "write" ) == 0)
	{
		printf("Description : Used to write data into regular file.\n");
		printf("Usage : write File_name\n");	
	}
	else if( strcmp( name , "copy" ) == 0)
	{
		printf("Description : Used to copy data from oldfile to newfile.\n");
		printf("Usage : copy newfile_name oldfile_name\n");	
	}
	else if(strcmp( name , "ls") == 0)
	{
		printf("Description : Used to list all information of files.\n");
		printf("Usage : ls\n");
	}
	else if(strcmp( name , "stat") == 0)
	{
		printf("Description : Used to display information file.\n");
		printf("Usage : stat File_name\n");
	}
	else if(strcmp( name , "fstat") == 0)
	{
		printf("Description : Used to display information file.\n");
		printf("Usage : fstat File_Descriptor\n");
	}
	else if(strcmp( name , "truncate") == 0)
	{
		printf("Description : Used to remove data from file.\n");
		printf("Usage : truncate File_name\n");
	}
	else if(strcmp(name , "open") == 0)
	{
		printf("Description : Used to open existing file.\n");
		printf("Usage : open File_name mode\n");
	}
	else if(strcmp(name , "close") == 0)
	{
		printf("Description : Used to close opened file.\n");
		printf("Usage : close File_name\n");
	}
	else if(strcmp( name , "closeall") == 0)
	{
		printf("Description : Used to close all opened file.\n");
		printf("Usage : closeall\n");
	}
	else if(strcmp( name , "lseek") == 0)
	{
		printf("Description : Used to change file offset.\n");
		printf("Usage : lseek File_name ChangeInOffset StartPoint\n");
	}
	else if(strcmp( name , "rm") == 0)
	{
		printf("Description : Used to delete the particular file.\n");
		printf("Usage : rm File_name\n");
	}
	else if(strcmp( name , "rm -r") == 0)
	{
		printf("Description : Used to delete all the files.\n");
		printf("Usage : rm -r\n");
	}
	else
	{
		printf("ERROR : No manual entry available.\n");
	}
}


void DisplayHelp()
{
	printf("\nls 	 : To List out all files.\n");
	printf("clear 	 : To clear console.\n");
	printf("open 	 : To open the file.\n");
	printf("close 	 : To close the file.\n");
	printf("closeall : To close all opened files.\n");
	printf("read     : To Read the contents from file.\n");
	printf("copy     : To copy the data from oldfile to newfile.\n");
	printf("write 	 : To Write Contents into file.\n");
	printf("exit 	 : To Terminate file system.\n");
	printf("stat 	 : To Display information of file using name.\n");
	printf("fstat 	 : To Display information of file using file Descriptor.\n");
	printf("truncate : To Remove all data from file.\n");
	printf("rm 		 : To Delete the file.\n");
	printf("rm -r 	 : To Delete all the files.\n");	
}


int GetFDFromName( char *name)
{
	int i = 0;

	while( i<MAXINODE )
	{
		if(UFDTArr[i].ptrfiletable != NULL)
		{
			if(strcmp((UFDTArr[i].ptrfiletable->ptrinode->FileName),name)==0)
				break;
		}
		i++;
	}

	if( i == MAXINODE )
	{
		return -1;
	} 
	else
	{
		return i;
	}

} 


PINODE Get_Inode( char *name)
{
	PINODE temp = head;
	int i = 0;

	if( name == NULL)
	{
		return NULL;
	}

	while( temp != NULL)
	{
		if(strcmp(name , temp->FileName) == 0)
			break;

		temp = temp -> next;
	}

	return temp;
} 


void CreateDILB() // its like InsertLast of LL
{	
	//creates inode structure LL
	 int i = 1;
	 PINODE newn = NULL;
	 PINODE temp = head;

	 while( i<=MAXINODE)
	 {
	 	newn = (PINODE)malloc(sizeof(INODE)); // 86 bytes allocates

	 	newn -> LinkCount = 0;
	 	newn -> ReferenceCount = 0;
	 	newn -> FileType = 0;
	 	newn -> FileSize = 0;

		 	newn -> Buffer = NULL;
		 	newn -> next = NULL;

	 	newn -> InodeNumber = i;
	 	
	 		if( temp == NULL )
	 		{
	 			head = newn;
	 			temp = head;
	 		}
	 		else
	 		{
	 			temp -> next = newn;
	 			temp = temp -> next;
	 		}
	 		i++;
	 }

	 printf("\nDILB created Successfully.\n");
}


void InitialiseSuperBlock() // set ufdt and super block
{
	int i = 0;

	// necessary to NULL every ele. of array
	while( i<MAXINODE )
	{
		UFDTArr[i].ptrfiletable = NULL;
		i++;
	}

	//set value
	SUPERBLOCKobj.TotalInodes = MAXINODE; 
	SUPERBLOCKobj.FreeInodes = MAXINODE;

}


int CreateFile( char *name , int Permission )
{
	int i = 0;

	PINODE temp = head;

	if((name==NULL) || (Permission==0) || (Permission>3))
		return -1; //error codes

	if( SUPERBLOCKobj.FreeInodes == 0)
	{
		return -2;
	}

	//we are using one inode so deduct 1 from FreeInodes
	(SUPERBLOCKobj.FreeInodes)--;

	/*
		we are checking if the name file is already exist or not
	*/
	if(Get_Inode(name) != NULL)
		return -3;

	while( temp != NULL)
	{	
		/*
			FileType either 1 or 0
			if 0 means inode is unused else 1 means used 
		*/
		if( temp -> FileType == 0)
		{
			break;
		}

		temp = temp -> next;
	} 

	while( i<MAXINODE )
	{
		if( UFDTArr[i].ptrfiletable == NULL)
			break;

		i++;
	} 

	UFDTArr[i].ptrfiletable = (PFILETABLE)malloc(sizeof(FILETABLE));

	UFDTArr[i].ptrfiletable -> count = 1;
	UFDTArr[i].ptrfiletable -> mode = Permission;
	UFDTArr[i].ptrfiletable -> readoffset = 0;
	UFDTArr[i].ptrfiletable -> writeoffset = 0;

	UFDTArr[i].ptrfiletable -> ptrinode = temp;

	strcpy( UFDTArr[i].ptrfiletable->ptrinode->FileName,name);
	UFDTArr[i].ptrfiletable->ptrinode->FileType = REGULAR;
	UFDTArr[i].ptrfiletable->ptrinode->ReferenceCount = 1;
	UFDTArr[i].ptrfiletable->ptrinode->LinkCount = 1;
	UFDTArr[i].ptrfiletable->ptrinode->FileSize = MAXFILESIZE;
	UFDTArr[i].ptrfiletable->ptrinode->FileActualSize = 0;
	UFDTArr[i].ptrfiletable->ptrinode->Permission = Permission;
	UFDTArr[i].ptrfiletable->ptrinode->Buffer =(char*)malloc(MAXFILESIZE);//2048kb

	return i;
}


// similar to unlink
int rm_File( char *name)
{
	int fd = 0;

	fd = GetFDFromName(name);

	if( fd == -1)
		return -1;

	(UFDTArr[fd].ptrfiletable->ptrinode->LinkCount)--;

	if(UFDTArr[fd].ptrfiletable->ptrinode->LinkCount == 0)
	{
		UFDTArr[fd].ptrfiletable->ptrinode->FileType = 0;
		free(UFDTArr[fd].ptrfiletable->ptrinode->Buffer);
		strcpy(UFDTArr[fd].ptrfiletable->ptrinode->FileName,"");
		UFDTArr[fd].ptrfiletable->ptrinode->ReferenceCount = 0;
		UFDTArr[fd].ptrfiletable->ptrinode->Permission = 0;
		UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize = 0;
		free(UFDTArr[fd].ptrfiletable);
	}

	UFDTArr[fd].ptrfiletable = NULL;
	(SUPERBLOCKobj.FreeInodes)++;

	return 0;
} 

int CountInode()
{
	return (SUPERBLOCKobj.TotalInodes-SUPERBLOCKobj.FreeInodes);
}

int rm_all()
{	
	int i = 0;
	int Cnt = 0;
	Cnt = CountInode();

	if(!CountInode())
		return -1;

	while( Cnt != 0)
	{
		if(UFDTArr[i].ptrfiletable != NULL)
		{	
			//deleting file
			(UFDTArr[i].ptrfiletable->ptrinode->LinkCount)--;

			if(UFDTArr[i].ptrfiletable->ptrinode->LinkCount == 0)
			{	
				//removing metadata of file
				UFDTArr[i].ptrfiletable->ptrinode->FileType = 0;
				free(UFDTArr[i].ptrfiletable->ptrinode->Buffer);
				strcpy(UFDTArr[i].ptrfiletable->ptrinode->FileName,"");
				UFDTArr[i].ptrfiletable->ptrinode->ReferenceCount = 0;
				UFDTArr[i].ptrfiletable->ptrinode->Permission = 0;
				UFDTArr[i].ptrfiletable->ptrinode->FileActualSize = 0;
				free(UFDTArr[i].ptrfiletable);
			}

			UFDTArr[i].ptrfiletable = NULL;
			(SUPERBLOCKobj.FreeInodes)++;
			Cnt--;
		}
		i++;
	}

	return 0;
}

int ReadFile( int fd , char *arr , int isize)
{
	int read_size = 0;

	if( UFDTArr[fd].ptrfiletable == NULL)
		return -1;

	if(UFDTArr[fd].ptrfiletable->mode != READ && UFDTArr[fd].ptrfiletable->mode != READ+WRITE)
		return -2;

	if(((UFDTArr[fd].ptrfiletable->ptrinode->Permission)!=WRITE)&&((UFDTArr[fd].ptrfiletable->ptrinode->Permission)!=READ+WRITE))
		return -2;

	if(UFDTArr[fd].ptrfiletable->readoffset == UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize)
		return -3;

	if(UFDTArr[fd].ptrfiletable->ptrinode->FileType != REGULAR)
		return -4;

	read_size = (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize)-(UFDTArr[fd].ptrfiletable->readoffset);

	if(read_size<isize)
	{
		strncpy(arr,(UFDTArr[fd].ptrfiletable->ptrinode->Buffer)+(UFDTArr[fd].ptrfiletable->readoffset),read_size);
	
		UFDTArr[fd].ptrfiletable->readoffset = UFDTArr[fd].ptrfiletable->readoffset+read_size;
	}
	else
	{
		strncpy(arr,(UFDTArr[fd].ptrfiletable->ptrinode->Buffer)+(UFDTArr[fd].ptrfiletable->readoffset),isize);
	
		(UFDTArr[fd].ptrfiletable->readoffset) = (UFDTArr[fd].ptrfiletable->readoffset) + isize;
	}

	return isize;

}


int WriteFile( int fd , char *arr , int isize )
{

	if(((UFDTArr[fd].ptrfiletable->mode)!=WRITE)&&((UFDTArr[fd].ptrfiletable->mode)!=READ+WRITE))
		return -1;

	if(((UFDTArr[fd].ptrfiletable->ptrinode->Permission)!=WRITE)&&((UFDTArr[fd].ptrfiletable->ptrinode->Permission)!=READ+WRITE))
		return -1;

	//if File is Full
	if((UFDTArr[fd].ptrfiletable->writeoffset)==MAXFILESIZE)
		return -2;

	if((UFDTArr[fd].ptrfiletable->ptrinode->FileType)!=REGULAR)
		return -3;

	//if there is not sufficient space in file for writing data of size in [isize]
	if(((UFDTArr[fd].ptrfiletable->ptrinode->FileSize)-(UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize))<isize)
		return -4;

	strncpy((UFDTArr[fd].ptrfiletable->ptrinode->Buffer)+(UFDTArr[fd].ptrfiletable->writeoffset),arr,isize);

	(UFDTArr[fd].ptrfiletable->writeoffset)=(UFDTArr[fd].ptrfiletable->writeoffset)+isize;

	(UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize)=(UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize)+isize;

	return isize;
}


int OpenFile( char *name , int mode)
{
	int i = 0;

	PINODE temp = NULL;

	if(name==NULL || mode<=0 )
		return -1;

	temp = Get_Inode(name);
	if(temp==NULL)
		return -2;

	if(temp->Permission<mode)
		return -3;

	while(i<MAXINODE)
	{
		if(UFDTArr[i].ptrfiletable==NULL)
			break;

		i++;
	}

	UFDTArr[i].ptrfiletable=(PFILETABLE)malloc(sizeof(FILETABLE));

	if(UFDTArr[i].ptrfiletable==NULL)
		return -1;

	UFDTArr[i].ptrfiletable->count = 1;
	UFDTArr[i].ptrfiletable->mode = mode;

	if(mode==READ + WRITE)
	{
		UFDTArr[i].ptrfiletable->readoffset = 0;
	}
	else if(mode == WRITE)
	{
		UFDTArr[i].ptrfiletable->writeoffset = 0;
	}

	UFDTArr[i].ptrfiletable->ptrinode = temp;
	(UFDTArr[i].ptrfiletable->ptrinode->ReferenceCount)++;

	return i;

}


void CloseFileByFD(int fd)
{
	UFDTArr[fd].ptrfiletable->readoffset = 0;
	UFDTArr[fd].ptrfiletable->writeoffset = 0;

	if(UFDTArr[fd].ptrfiletable->ptrinode->ReferenceCount > 0)
	{	
		(UFDTArr[fd].ptrfiletable->ptrinode->ReferenceCount)--;
	}
	else
	{
		UFDTArr[fd].ptrfiletable->ptrinode->ReferenceCount = 0;
	}
}


int CloseFileByName( char *name)
{
	int i = 0;

	i = GetFDFromName(name);

	if( i==-1)
		return -1;

	UFDTArr[i].ptrfiletable->readoffset = 0;
	UFDTArr[i].ptrfiletable->writeoffset = 0;

	if(UFDTArr[i].ptrfiletable->ptrinode->ReferenceCount > 0)
	{	
		(UFDTArr[i].ptrfiletable->ptrinode->ReferenceCount)--;
	}
	else
	{
		UFDTArr[i].ptrfiletable->ptrinode->ReferenceCount = 0;
	}

	/*if u do LinkCount -- then file will be delete*/

	return 0;
}


void CloseAllFile()
{
	int i = 0;

	while( i<MAXINODE )
	{
		if(UFDTArr[i].ptrfiletable != NULL)
		{
			UFDTArr[i].ptrfiletable->readoffset = 0;
			UFDTArr[i].ptrfiletable->writeoffset = 0;
			//(UFDTArr[i].ptrfiletable->ptrinode->ReferenceCount)--;
			UFDTArr[i].ptrfiletable->ptrinode->ReferenceCount = 0;
		}
		i++;
	}
}


int LseekFile( int fd , int size , int from)
{
	if((from<0) || (from>2))
		return -1;

	if(UFDTArr[fd].ptrfiletable == NULL)
		return -1;

	if((UFDTArr[fd].ptrfiletable->mode==READ)||(UFDTArr[fd].ptrfiletable->mode==READ+WRITE))
	{

		if(from==CURRENT)
		{
			if(((UFDTArr[fd].ptrfiletable->readoffset)+size)>UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize)
				return -1;

			if(((UFDTArr[fd].ptrfiletable->readoffset)+size)<0)
				return -1;

			(UFDTArr[fd].ptrfiletable->readoffset)=(UFDTArr[fd].ptrfiletable->readoffset)+size;
		}
		else if(from==START)
		{
			if(size>(UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize))
				return -1;

			if(size<0)
				return -1;

			(UFDTArr[fd].ptrfiletable->readoffset) = size;
		}
		else if(from==END)
		{
			if((UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize)+size>MAXFILESIZE)
				return -1;

			if(((UFDTArr[fd].ptrfiletable->readoffset)+size)<0)
				return -1;

			(UFDTArr[fd].ptrfiletable->readoffset)=(UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize)+size;
		}
	}
	else if(UFDTArr[fd].ptrfiletable->mode == WRITE)
	{
		if(from==CURRENT)
		{
			if((UFDTArr[fd].ptrfiletable->writeoffset)+size>MAXFILESIZE)
				return -1;

			if(((UFDTArr[fd].ptrfiletable->writeoffset)+size)<0)
				return -1;

			if(((UFDTArr[fd].ptrfiletable->writeoffset)+size)>(UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize));
			{	
				(UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize)=(UFDTArr[fd].ptrfiletable->writeoffset)+size;
			}

			(UFDTArr[fd].ptrfiletable->writeoffset)=(UFDTArr[fd].ptrfiletable->writeoffset)+size;
		}
		else if(from==START)
		{
			if(size>MAXFILESIZE)
				return -1;

			if(size<0)
				return -1;

			if(size>(UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize))
				(UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize)=size;

			(UFDTArr[fd].ptrfiletable->writeoffset)=size;
		}
		else if(from==END)
		{
			if((UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize)+size>MAXFILESIZE)
				return -1;

			if(((UFDTArr[fd].ptrfiletable->writeoffset)+size)<0)
				return -1;

			(UFDTArr[fd].ptrfiletable->writeoffset)=(UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize)+size;
		}
	}
}


void ls_file()
{
	int i = 0;
	PINODE temp = head;

	if(SUPERBLOCKobj.FreeInodes==MAXINODE)
	{
		printf("ERROR : There are no files.\n");
		return;
	}

	printf("\nFile Name\tInode number\tFile size\tFile Actual Size\tLink count\n");
	printf("--------------------------------------------------------------------------------------\n");

	while(temp!=NULL)
	{	
		/*
			FileType is used to check file is present or not
			if FileType is 0 then that file doesn't exist
			if FileType is non-zero then that file exist
		*/
		if(temp->FileType!=0)
		{
			printf("%s\t\t%d\t\t%d\t\t%d\t\t\t%d\n",temp->FileName,temp->InodeNumber,temp->FileSize,temp->FileActualSize,temp->LinkCount);
			printf("--------------------------------------------------------------------------------------\n");
		}

		temp = temp -> next;
	}

}


int fstat_file(int fd)// fstat requires fd
{
	PINODE temp = head;
	int i = 0;

	if( fd < 0)
		return -1;

	if(UFDTArr[fd].ptrfiletable==NULL)
		return -2;

	temp = UFDTArr[fd].ptrfiletable->ptrinode;

	if( temp->FileType == 0) //Filter
		return -2;
	

	printf("\n--------Statistical Information about file--------\n");
	printf("File name : %s\n",temp->FileName);
	printf("Inode Number : %d\n",temp->InodeNumber);
	printf("File size : %d\n",temp->FileSize);
	printf("Actual File size : %d\n",temp->FileActualSize);
	printf("Link count : %d\n",temp->LinkCount);
	printf("Reference count : %d\n",temp->ReferenceCount);

	if(temp->Permission==1)
		printf("File Permission : Read Only\n");
	else if(temp->Permission==2)
		printf("File Permission : Write\n");
	else if(temp->Permission==3)
		printf("File Permission : Read & Write\n");

	printf("-------------------------------------------------------\n");	

	return 0;
}


int stat_file(char *name) //stat requires name of file
{
	PINODE temp = head;
	//int i = 0;

	if(name == NULL)
		return -1;

	//if there is not a single file
	if( SUPERBLOCKobj.FreeInodes == SUPERBLOCKobj.TotalInodes)
		return -2;

	while( temp != NULL)
	{
		if(strcmp(name,temp->FileName) == 0)
			break;

		temp = temp -> next;
	}

	if( temp == NULL)
		return -2;
	
	printf("\n--------Statistical Information about file--------\n");
	printf("File name : %s\n",temp->FileName);
	printf("Inode Number : %d\n",temp->InodeNumber);
	printf("File size : %d\n",temp->FileSize);
	printf("Actual File size : %d\n",temp->FileActualSize);
	printf("Link count : %d\n",temp->LinkCount);
	printf("Reference count : %d\n",temp->ReferenceCount);

	if(temp->Permission==1)
		printf("File Permission : Read Only\n");
	else if(temp->Permission==2)
		printf("File Permission : Write\n");
	else if(temp->Permission==3)
		printf("File Permission : Read & Write\n");

	printf("---------------------------------------------------\n");	

	return 0;
}

//it will remove data present inside file
int truncate_File( char *name)
{
	int fd = GetFDFromName(name);

	if(fd==-1)
		return -1;

	memset(UFDTArr[fd].ptrfiletable->ptrinode->Buffer,0,MAXFILESIZE);
	UFDTArr[fd].ptrfiletable->readoffset = 0;
	UFDTArr[fd].ptrfiletable->writeoffset = 0;
	UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize = 0;

	return 0;
}

//Deallocate resources
void DeallocateResources()
{
	int i = 0;
	int Cnt = 0;
	Cnt = CountInode();

	if(SUPERBLOCKobj.TotalInodes == SUPERBLOCKobj.FreeInodes)
	{
		return;
	}

	//Freeing Buffers
	while( Cnt != 0)
	{
		if(UFDTArr[i].ptrfiletable != NULL)
		{
			if(strcmp(UFDTArr[i].ptrfiletable->ptrinode->FileName,"")!=0)
			{
				if(UFDTArr[i].ptrfiletable->ptrinode->FileActualSize!=0)
				{
					free(UFDTArr[i].ptrfiletable->ptrinode->Buffer);
				}
			}
			free(UFDTArr[i].ptrfiletable->ptrinode);
			UFDTArr[i].ptrfiletable->ptrinode = NULL;
			free(UFDTArr[i].ptrfiletable);
			UFDTArr[i].ptrfiletable = NULL;
			Cnt--;
		}
		i++;
	}
} 


int CopyFile( char* newname , char* oldname)
{
	int ofd = 0 , nfd = 0;

	//if there is not a single file in File System
	if(SUPERBLOCKobj.FreeInodes==MAXINODE)
		return -1;

	//if inodes are not avilable
	if(SUPERBLOCKobj.FreeInodes == 0)
		return -2;

	//Getting fd
	ofd = GetFDFromName(oldname);
	nfd = GetFDFromName(newname);

	if(ofd == -1)
		return -1;

	//if new file is not created
	if(nfd == -1)
	{
		nfd = CreateFile(newname,UFDTArr[ofd].ptrfiletable->ptrinode->Permission);
		UFDTArr[nfd].ptrfiletable->ptrinode->FileType = SPECIAL;
	}

	//copying data from oldfile to newfile
	if(UFDTArr[nfd].ptrfiletable->ptrinode->FileType == SPECIAL)
	{
		strcpy(UFDTArr[nfd].ptrfiletable->ptrinode->Buffer,UFDTArr[ofd].ptrfiletable->ptrinode->Buffer);
		UFDTArr[nfd].ptrfiletable->ptrinode->FileActualSize = UFDTArr[ofd].ptrfiletable->ptrinode->FileActualSize;
	}

	return 0;
}

/*
 we use word virtual bcoz it is in RAM
 we customised inbuilt data structures eg. inode struct 
 we created structures as per our use 
*/
int main()
{
	char *ptr = NULL;
	int ret = 0 , fd = 0 , count = 0;
	char command[4][80] , str[80] , arr[MAXFILESIZE];

	// if u use c++ then this functions will be inside constr.
	//starter function
	InitialiseSuperBlock(); 
	CreateDILB();

	while(1)
	{
		fflush(stdin);
		strcpy(str ,"");

		printf("\nMarvellous CustomisedVFS : > ");

		fgets(str , 80 , stdin);//scanf("%[^'\n']s",str);

		count=sscanf(str,"%s%s%s%s",command[0],command[1],command[2],command[3]);

		if( count == 1 ) //1 word 
		{
			if(strcmp(command[0],"ls")==0)
			{
				ls_file();
			}
			else if(strcmp(command[0],"closeall")==0)
			{
				CloseAllFile();
				printf("All files closed Successfully.\n");
				continue;
			}
			else if(strcmp(command[0],"clear")==0)
			{
				system("cls");
				continue;
			}
			else if(strcmp(command[0],"help")==0)
			{
				DisplayHelp();
				continue;
			}
			else if(strcmp(command[0],"exit")==0)
			{
				printf("Terminating the Virtual File System.\n");
				DeallocateResources();
				break;
			}
			else
			{
				printf("\nERROR : command not Fount!!!\n");
				continue;
			}
		}
		else if( count == 2 )
		{
			if(strcmp(command[0],"stat")==0)//statistics
			{
				ret = stat_file(command[1]);

				if(ret==-1)
					printf("ERROR : Incorrect parameters.\n");

				if(ret==-2)
					printf("ERROR : There is no such file.\n");

				continue;
			}
			else if(strcmp(command[0],"fstat")==0)//file statistics
			{
				ret = fstat_file(atoi(command[1]));

				if(ret==-1)
					printf("ERROR : Incorrect parameters.\n");

				if(ret==-2)
					printf("ERROR : There is no such file.\n");

				continue;
			}
			else if(strcmp(command[0],"close")==0)
			{
				
				ret = CloseFileByName(command[1]);

				if(ret==-1)
					printf("ERROR : There is no such file.\n");
				else if( ret == 0)
					printf("File Closed Successfully.\n");

				continue;
			}
			else if(strcmp(command[0],"closefd")==0)
			{
				
				CloseFileByFD(atoi(command[1]));

				continue;
			}
			else if(strcmp(command[0],"rm")==0)//Remove[delete] file
			{
				//unlink is system call rm is command
				//unlink is used to remove file

				if(strcmp(command[1],"-r")==0)
				{
					ret = rm_all();

					if(ret==-1)
						printf("There are no files.\n");
					else if( ret == 0)
						printf("All files deleted Successfully.\n");

				}
				else
				{
					ret = rm_File(command[1]);

					if(ret==-1)
						printf("ERROR : There is no such file.\n");
					else if( ret == 0)
						printf("File deleted Successfully.\n");

				}

				continue;
			}
			else if(strcmp(command[0],"man")==0)//manual
			{
				man(command[1]);
			}
			else if(strcmp(command[0],"write")==0)//write File_name
			{
				fd = GetFDFromName(command[1]);

				if(fd==-1)
				{
					printf("ERROR : Incorrect parameters.\n");
					continue;
				}

				printf("Enter the data : \n");
				fflush(stdin);
				scanf("%[^'\n']",arr); //abcd

				ret = strlen(arr); //ret = 4

				if(ret == 0)
				{
					printf("ERROR : Incorrect parameter.\n");
					continue;
				}

				ret = WriteFile(fd,arr,ret);

				if(ret==-1)
					printf("ERROR : Permission denied.\n");

				if(ret==-2)
					printf("ERROR : There is no sufficient memory to write.\n");
				
				if(ret==-3)
					printf("ERROR : It is not regular file.\n");

				if(ret==-4)
					printf("ERROR : There is no sufficient memory available to write.\n");

				if( ret > 0)
				{
					printf("\nSuccess : %dbytes Successfully written.",ret);
				}
			}
			else if(strcmp(command[0],"truncate")==0)
			{
				ret = truncate_File(command[1]);

				if(ret==-1)
					printf("ERROR : Incorrect parameter.\n");
				else if(ret==0)
					printf("Data inside file removed Successfully.\n");
			}
			else
			{
				printf("\nERROR : Command not Found!!!\n");
				continue;
			}
		}
		else if(count==3)
		{
			if(strcmp(command[0],"create")==0)
			{	
				// atoi = ascii to integer
				ret = CreateFile(command[1],atoi(command[2]));
					//CreateFile( "Demo.txt" , 3 );
				if(ret>=0)
					printf("File is Successfully created with file Descriptor : %d\n",ret);

				if(ret==-1) //error codes
					printf("ERROR : Incorrect parameters.\n");

				if(ret==-2)
					printf("ERROR : There is no inodes.\n");

				if(ret==-3)
					printf("ERROR : File already exists.\n");

				if(ret==-4)
					printf("ERROR : Memory allocation Failure.\n");

				continue;
			}
			else if(strcmp(command[0],"open")==0)
			{
				ret = OpenFile(command[1],atoi(command[2]));

				if(ret>=0)
					printf("File is Successfully opened with file Descriptor : %d\n",ret);

				if(ret==-1)
					printf("ERROR : Incorrect parameters.\n");

				if(ret==-2)
					printf("ERROR : File not present.\n");

				if(ret==-3)
					printf("ERROR : Permission denied.\n");

				continue;
			}
			else if(strcmp(command[0],"copy")==0)
			{
				ret = CopyFile(command[1],command[2]);

				if(ret==-1)
					printf("%s File doesn't exists.\n",command[1]);

				if(ret==-2)
					printf("There are no inodes.\n");

				if(ret==0)
					printf("File copied Successfully.\n");
			}
			else if(strcmp(command[0],"read")==0)
			{
				ret = GetFDFromName(command[1]);

				if(fd==-1)
				{
					printf("ERROR : Incorrect parameter.\n");
					continue;
				}

				ptr = (char *)malloc(sizeof(atoi(command[2]))+1);

				if( ptr == NULL)
				{
					printf("ERROR : Memory allocation Failure.\n");
					continue;
				}

				ret = ReadFile(fd,ptr,atoi(command[2]));

				if(ret==-1)
					printf("ERROR : File not existing.\n");

				if(ret==-2)
					printf("ERROR : Permission denied.\n");

				if(ret==-3)
					printf("ERROR : Reached at end of file.\n");

				if(ret==-4)
					printf("ERROR : It is not regular file.\n");

				if( ret == 0)
					printf("ERROR : File empty.\n");

				if( ret > 0 )
				{	
					/*
					Here we are using write sys call than printf
					bcoz data comes inside ptr doesn't contain '\0'
					Here 1 means = stdout;
					*/
					write(1,ptr,ret);//system call
				}

				continue;
			}
			else
			{
				printf("\nERROR : Command not Found!!!");
				continue;
			}
		} 
		else if( count == 4)
		{
			if(strcmp(command[0],"lseek")==0)
			{
				fd = GetFDFromName(command[1]);

				if(fd==-1)
				{
					printf("ERROR : Incorrect parameter.\n");
					continue;
				}

				ret = LseekFile(fd,atoi(command[2]),atoi(command[3]));

				if(ret==-1)
				{
					printf("ERROR : Unable to perform lseek.\n");
				}
			}
			else
			{
				printf("\nERROR : Command not Found!!!\n");
				continue;
			}
		}
		else
		{
			printf("\nERROR : Command not Found!!!\n");
			continue;
		}
	}

	return 0;
}