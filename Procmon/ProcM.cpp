#include<windows.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<iostream>
#include<string.h>
#include<tlhelp32.h>
#include<stdio.h>
#include<io.h>

using namespace std;
#pragma warning(disable : 4996)

typedef unsigned int UINT; 

//structure to create LogFile with desired information
typedef struct LogFile 
{
    char ProcessName[100];  
    UINT pid;
    UINT ppid;
    UINT thread_cnt;
}LOGFILE; //size of structure = 112 bytes

class ThreadInfo
{
    private :
            DWORD PID;
            HANDLE hThreadSnap;
            THREADENTRY32 te32;
    
    public :    
            ThreadInfo(DWORD);
            BOOL ThreadsDisplay(DWORD);
};


/////////////////////////////////////////////////////////////////////////////////////////////
//
//	Name				:ThreadInfo(constructor)
//	Input				:DWORD
//	Description			:Initialise resources for ThreadInfo class such as taking snapshot
//						 of all threads of Process passesed as Parameter
//	Author				:Rushikesh Godase
//	Date				:18 Aug 2020
//
/////////////////////////////////////////////////////////////////////////////////////////////
ThreadInfo :: ThreadInfo( DWORD no)
{
    PID = no;

	// Take a snapshot of all Threads in the system by giving PID of Process.
    hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD,PID);

    if(hThreadSnap == INVALID_HANDLE_VALUE)
    {
        cout<<"Unable to create the snapshot of current thread pool"<<"\n";
        return;
    }

	//  Set the size of the structure before using it.
    te32.dwSize = sizeof(THREADENTRY32);
}


/////////////////////////////////////////////////////////////////////////////////////////////
//
//	Name				:ThreadsDisplay
//	Input				:DWORD
//	Output				:BOOL
//	Description			:Displays all thread's PID associated with Process
//	Author				:Rushikesh Godase	
//	Date				:18 Aug 2020
//
/////////////////////////////////////////////////////////////////////////////////////////////
BOOL ThreadInfo :: ThreadsDisplay( DWORD tcnt)
{
	//  Retrieve information about the first thread of process , 
	//  and exit if unsuccessful 
    if(!Thread32First(hThreadSnap,&te32))
    {
        cout<<"Error: In Getting the first thread"<<"\n";
        CloseHandle(hThreadSnap);
        return FALSE;
    }

    cout<<endl<<"THREADS OF THIS PROCESS:"<<endl;
	//  Now walk the Thread list of the process, 
	//  and display ThreadID each Process 
    do
    {
        if(te32.th32OwnerProcessID == PID )
        {
            cout<<"\tTHREAD ID :"<< te32.th32ThreadID<<endl;
			tcnt--;
        }

    } while (Thread32Next(hThreadSnap,&te32) && tcnt != 0);
    
    CloseHandle(hThreadSnap);  

    return TRUE;
}

class DLLInfo
{
    private :   
        DWORD PID;
        MODULEENTRY32 me32;
        HANDLE hProcessSnap;

    public :    
        DLLInfo(DWORD);
        BOOL DependentDLLDisplay();
};


/////////////////////////////////////////////////////////////////////////////////////////////
//
//	Name				:DLLInfo(constructor)
//	Input				:DWORD
//	Output				:  -
//	Description			:Take snapshot of all DLL of passed process
//	Author				:Rushikesh Godase	
//	Date				:18 Aug 2020
//
/////////////////////////////////////////////////////////////////////////////////////////////
DLLInfo :: DLLInfo(DWORD no)
{
    this->PID = no;

	// Take a snapshot of all DLL in the system by giving PID of process.
    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE,PID);

    if(hProcessSnap == INVALID_HANDLE_VALUE)
    {
        cout<<"ERROR: UNABLE to Create the snapshot of current thread pool"<<endl;
        return;
    }

	//  Set the size of the structure before using it.
    me32.dwSize = sizeof(MODULEENTRY32);
}


/////////////////////////////////////////////////////////////////////////////////////////////
//
//	Name				:DependentDLLDisplay
//	Input				:void
//	Output				:BOOL
//	Description			:Displays all DLL associated with Process
//	Author				:Rushikesh Godase	
//	Date				:18 Aug 2020
//
/////////////////////////////////////////////////////////////////////////////////////////////
BOOL DLLInfo :: DependentDLLDisplay()
{
    char arr[200];

	//  Retrieve information about the first module, 
	//  and exit if unsuccessful
    if(!Module32First(hProcessSnap,&me32))
    {
        cout<<"FAILED to get DLL information. "<<endl;
        CloseHandle(hProcessSnap);	//clean the snapshot object
        return FALSE;
    }

    cout<<"\nDEPENDENT DLL OF THIS PROCESS :"<<endl;
	//  Now walk the DLL list of the process, 
	//  and display each DLL
    do
    {
        wcstombs_s(NULL,arr,200,me32.szModule,200);
        cout<<'\t'<<arr<<endl;

    }while(Module32Next(hProcessSnap , &me32));
    

    CloseHandle(hProcessSnap);	//clean the snapshot object
    return TRUE;
}

class ProcessInfo
{
	private:

        DWORD PID;			//DWORD = UINTeger[windows typedef]  size = 4
        DLLInfo *pdobj;		//pointer size = 4
        ThreadInfo *ptobj;  //pointer size = 4
        HANDLE hProcessSnap;   //handle is UINTeger size = 4
        PROCESSENTRY32 pe32;	/*PROCESSENTRY32 is structure and pe32 is object of that structure
								it is present inside windows.h(declarations) size = 28*/
		/* Inside windows.h file
			typedef UINT UINT;
			typedef UINT Handle;
		*/

    public:

            ProcessInfo();		//default constructor
            BOOL ProcessDisplay(const char *);
            BOOL ProcessLog();
            BOOL ReadLog(DWORD,DWORD,DWORD,DWORD);
            BOOL ProcessSearch(char *);
            BOOL KillProcess(char *);

};


/////////////////////////////////////////////////////////////////////////////////////////////
//	
//	Name				:ProcessInfo(constructor)
//	Input				:void
//	Output				:-
//	Description			:Take snapshot of all processes run at time of execution
//	Author				:Rushikesh Godase	
//	Date				:18 Aug 2020
//
/////////////////////////////////////////////////////////////////////////////////////////////
ProcessInfo::ProcessInfo()
{
    ptobj = NULL;  //process thread object
    pdobj = NULL;	//process dll object

	/*
		taking snapshot
		TH32CS_SNAPPROCESS is value which tells us to take snapshot of process.
	*/
	// Take a snapshot of all processes in the system by giving second parameter as 0.
    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);//handle

    if(hProcessSnap == INVALID_HANDLE_VALUE) //INVALID_HANDLE_VALUE == NULL
    {
        cout<<"Error:Unable to create the snapshot of running processes"<<endl;
        return;
    }

	//  Set the size of the structure before using it. 
    pe32.dwSize = sizeof(PROCESSENTRY32);
	/* 
		windows style
		PROCESSENTRY32 is structure and pe32 is object of that structure (inbuilt)
		Here we are assingning size of our structure inside structure.
	*/

}


/////////////////////////////////////////////////////////////////////////////////////////////
//
//	Name				:ProcessDisplay
//	Input				:const char*
//	Output				:BOOL
//	Description			:Displays all Process executing in RAM
//	Author				:Rushikesh Godase	
//	Date				:18 Aug 2020
//
/////////////////////////////////////////////////////////////////////////////////////////////
BOOL ProcessInfo::ProcessDisplay(const char* option) //option will be -a , -t , -d
{
    char arr[200];
	/* 
		Handle is use to create a snapshot.for receiving integer return 
		value of snapshot handle is created
	
	*/
	//  Retrieve information about the first process, 
	//  and exit if unsuccessful 
    if( !Process32First(hProcessSnap,&pe32)) //if snapshot of first process is not takes place
    {
        cout<<"Error : In finding the process."<<endl;
        CloseHandle(hProcessSnap);//clean the snapshot object
        return FALSE;
    }
	/*
		Process32First & Process32Next used for iteration. Retrieve the snapshot.
	*/


	//  Now walk the Process list, 
	//  and display information about each process
    do
    {
        cout<<endl<<"-------------------------------------------------------";

		//arr is going empty and comes with data that we display on console
        wcstombs_s(NULL,arr,200,pe32.szExeFile,200);
		/*
		 szExeFile
			  sz is string terminated with '\0;
		 wcstombs_s() this function converts szExeFile(string) into arr from unicode to asccii
			data comes inside szExeFile and we are collecting it into arr
			suppose,
				szExeFile = | m | \0 | y | \0 | . | \0 | e | \0 | x | \0 | e | \0 | (UNICODE 2Byte)
					here printf will only print m bcoz of \0. so convert it into ASCII
				converted
				arr = | m | y | . | e | x | e | ( ASCII 1Byte)
		*/

		//ProcessInfo structure present in windows.h
        cout<<endl<<"PROCESS NAME:"<<arr;
        cout<<endl<<"PID:"<<pe32.th32ProcessID;				//Process ID
        cout<<endl<<"PPID:"<<pe32.th32ParentProcessID;		//Parent Process ID
        cout<<endl<<"No of Thread:"<<pe32.cntThreads;		//count of threads

		/* th32ProcessID , th32ParentProcessID , cntThreads , dwSize is members in obj. pe32 */

		//-a = all , -t = only threads  , -d = only DLL
        if( (_stricmp(option,"-a") == 0) || (_stricmp(option,"-d")==0) || (_stricmp(option,"-t")==0) )
        {
            if( (_stricmp(option,"-t") == 0) || (_stricmp(option,"-a") == 0)) //only for thread
            {
                ptobj = new ThreadInfo(pe32.th32ProcessID);
                ptobj->ThreadsDisplay( pe32.cntThreads );
                delete ptobj;
            }
            if( (_stricmp(option,"-d") == 0) || (_stricmp(option,"-a") == 0) )//only for dll
            {
                pdobj = new DLLInfo(pe32.th32ProcessID);
                pdobj->DependentDLLDisplay();
                delete pdobj;
            }
        }

        cout<<endl<<"-------------------------------------------------------";


    } while(Process32Next(hProcessSnap,&pe32)); //Process32Next != NULL

    CloseHandle(hProcessSnap);//clean the snapshot object

    return TRUE;
}


/////////////////////////////////////////////////////////////////////////////////////////////
//
//	Name				:ProcessLog
//	Input				:void
//	Output				:BOOL
//	Description			:Create a Log File of Process with Time stamp
//	Author				:Rushikesh Godase	
//	Date				:18 Aug 2020
//
/////////////////////////////////////////////////////////////////////////////////////////////
BOOL ProcessInfo::ProcessLog()
{
	//for fetching current time
    char* month[]={"JAN","FEB","MAR","APR","MAY","JUN","JUL","AUG","SEP","OCT","NOV","DEC"};
	//month is one dimensional array which contains adresses of string[char *]
	//"" by default considered as address of that ele.

    char FileName[50],arr[512];

    int ret = 0 , fd = 0, count = 0;

    SYSTEMTIME It;   //SYSTEMTIME is inbuilt structure

    LOGFILE fobj;	//LOGFILE is user defined structure

    FILE *fp;

    GetLocalTime(&It); //fetching cuurrent time

	//we fetch current time for creating name file giving name by using sprintf  
    sprintf(FileName,"G:\\ LOG%02d_%02d_%02d%s.txt",It.wHour,It.wMinute,It.wDay,month[It.wMonth-1]);    
	// "G:\\" here '\\' means '\'
	// %02d width specifier

    fp = fopen(FileName,"wb"); //opening in binary mode for writing purpose

    if(fp==NULL)
    {
        cout<<"\nUNABLE to create log File"<<endl;
        return FALSE;
    }
    else
    {
        cout<<"\nLog file successfully gets created as: "<<FileName<<endl;
        cout<<"\nTime of log file creation is->"<<It.wHour<<":"<<It.wMinute<<":"<<It.wDay<<"th"<<month[It.wMonth-1]<<endl;

    }

	//  Retrieve information about the first process, 
	//  and exit if unsuccessful 
    if(!Process32First(hProcessSnap,&pe32))
    {
        cout<<"Error : In finding the first process."<<endl;
        CloseHandle(hProcessSnap);//clean the snapshot object
        return FALSE;
    }

	//  Now walk the Process list, 
	//  and write information about each process	
    do 
    {
        wcstombs_s(NULL,arr,200,pe32.szExeFile,200);

        strcpy(fobj.ProcessName,arr);		 //copying name of file in fobj
        fobj.pid = pe32.th32ProcessID;		 //copy process id in fobj
        fobj.ppid = pe32.th32ParentProcessID;//copy parent process id in fobj
        fobj.thread_cnt = pe32.cntThreads;	 //copy thread count in fobj
        fwrite(&fobj,sizeof(fobj),1,fp);	 //writing content of fobj in file
											 //here 1 is how many object
    }while(Process32Next(hProcessSnap,&pe32));
    
    CloseHandle(hProcessSnap);//clean the snapshot object
    fclose(fp);    

    return FALSE;
}


/////////////////////////////////////////////////////////////////////////////////////////////
//
//	Name				:ReadLog
//	Input				:DWORD,DWORD,DWORD,DWORD
//	Output			    :BOOL
//	Description			:Reads Logfile's contents and display them to user
//	Author				:Rushikesh Godase	
//	Date				:18 Aug 2020
//
/////////////////////////////////////////////////////////////////////////////////////////////
BOOL ProcessInfo::ReadLog(DWORD hr,DWORD min ,DWORD date,DWORD month)
{
    char FileName[50];
	
     char* montharr[]={"JAN","FEB","MAR","APR","MAY","JUN","JUL","AUG","SEP","OCT","NOV","DEC"};
     int ret=0,count=0;
     LOGFILE fobj;
     FILE *fp;

	 sprintf(FileName,"G:\\ LOG%02d_%02d_%02d%s.txt",hr,min,date,montharr[month-1]);

    fp = fopen(FileName,"rb");  //reading in binary mode

    if(fp==NULL)
    {
        cout<<"Error:Unable to open log named as : "<<FileName<<endl;
        return FALSE;
    }

    while( (ret = fread(&fobj,1,sizeof(fobj),fp))!=0)  //data reading
    {
        cout<<"-----------------------------------------------------------------"<<endl;
        cout<<"Process Name :"<<fobj.ProcessName<<endl;
        cout<<"PID of current process:"<<fobj.pid<<endl;
        cout<<"Parent process PID:"<<fobj.ppid<<endl;
		cout << "Thread count of process:" << fobj.thread_cnt << endl;
    }

    return TRUE;
}


/////////////////////////////////////////////////////////////////////////////////////////////
//
//	Name				:ProcessSearch
//	Input				:char*
//	Output				:BOOL 
//	Description			:Search Particular Process with desired name.
//	Author				:Rushikesh Godase	
//	Date				:28 March 2020
//
/////////////////////////////////////////////////////////////////////////////////////////////
BOOL ProcessInfo::ProcessSearch(char* name)
{
    char arr[200];
    BOOL Flag = FALSE;

	//  Retrieve information about the first process, 
	//  and exit if unsuccessful 
    if(!Process32First(hProcessSnap,&pe32))
    {
        CloseHandle(hProcessSnap);
        return FALSE;
    }

	//  Now walk the Process list, 
	//  and display information about desired process
    do
    {
        wcstombs_s(NULL,arr,200,pe32.szExeFile,200);
        if(_stricmp(arr,name)==0)
        {
            cout<<endl<<"PROCESS NAME:"<<arr;
			cout<<endl<<"PID:"<<pe32.th32ProcessID;
			cout<<endl<<"PPID:"<<pe32.th32ParentProcessID;
			cout<<endl<<"No of Thread:"<<pe32.cntThreads;
			Flag = TRUE;
			break;
        }
    }while(Process32Next(hProcessSnap,&pe32));

    CloseHandle(hProcessSnap);//clean the snapshot object

    return Flag;
}


/////////////////////////////////////////////////////////////////////////////////////////////
//
//	Name				:KillProcess
//	Input				:char*
//	Output				:BOOL
//	Description			:Kill Particular Process with desired name
//	Author				:Rushikesh Godase	
//	Date				:18 Aug 2020
//
/////////////////////////////////////////////////////////////////////////////////////////////
BOOL ProcessInfo::KillProcess(char *name) //parameter : name of process
{
    char arr[200];
    int pid = -1;
    BOOL bRet;
    HANDLE hprocess; //handle of process

	//  Retrieve information about the first process, 
	//  and exit if unsuccessful 
    if(!Process32First(hProcessSnap,&pe32))
    {
        CloseHandle(hProcessSnap);//clean thesnapshot object
        return FALSE;
    }

	//  Now walk the Process list, 
	//  and find desired process to kill
    do
    {
        wcstombs_s(NULL,arr,200,pe32.szExeFile,200);

        if(_stricmp(arr,name)==0)
        {
            pid = pe32.th32ProcessID; //fetching pid of process
            break;
        }

    }while(Process32Next(hProcessSnap,&pe32));

    CloseHandle(hProcessSnap);//clean thesnapshot object

    if(pid == -1)
    {
        cout<<"Error:There is no such process"<<endl;
        return FALSE;
    }

	//API for open process for desired purpose
    hprocess = OpenProcess(PROCESS_TERMINATE,FALSE,pid);
	//OpenProcess( why u opening a process,are you parent of that proc.,pid of that process to open )

    if(hprocess == NULL) //if process is not open for eg.processes of OS[permission is not granted]
    {
        cout<<"Error : There is no access to terminate"<<endl;
        return FALSE;
    }

	//terminate process API
    bRet = TerminateProcess(hprocess,0); //Terminte a process
										//0 indicates y'r proc. + dependencies u want to kill
										//if 1 u want to delete only that process
    if(bRet==FALSE)
    {
        cout<<"Error:Unable to terminate process";
        return FALSE;
    }
	
	return TRUE;
	
}


/////////////////////////////////////////////////////////////////////////////////////////////
//
//	Name			:HardwareInfo
//	Input			:void
//	Output			:BOOL
//	Description		:Displays all Information of hardware
//	Author			:Rushikesh Godase	
//	Date			:18 Aug 2020
//
/////////////////////////////////////////////////////////////////////////////////////////////
BOOL HardwareInfo() //naked function
{
    SYSTEM_INFO siSyInfo;

    GetSystemInfo(&siSyInfo);

	cout << "Hardware Information of current system is:" << endl;
	cout <<"\n----------------------------------------------" << endl;
    cout<<"OEM ID:"<<siSyInfo.dwOemId<<endl; //OEM:Original Equipment Manufacturer
    cout<<"Number of processors :"<<siSyInfo.dwNumberOfProcessors<<endl;
    cout<<"Page size:"<<siSyInfo.dwPageSize<<endl;
	cout<<"Processor Type:"<<siSyInfo.dwProcessorType<<endl;
    cout<<"Minimum application address:"<<siSyInfo.lpMinimumApplicationAddress<<endl;
    cout<<"Maximum application address:"<<siSyInfo.lpMaximumApplicationAddress<<endl;
    cout<<"Active processor mask:"<<siSyInfo.dwActiveProcessorMask<<endl;

    return TRUE;

}


/////////////////////////////////////////////////////////////////////////////////////////////
//
//	Name			:DisplayHelp
//	Input			:void
//	Output			:void
//	Description		:Displays Help option to user
//	Author			:Rushikesh Godase	
//	Date			:18 Aug 2020
//
/////////////////////////////////////////////////////////////////////////////////////////////
void DisplayHelp()//Naked function
{

	cout<<"-----Process Monitoring Tool [ProcMon]-----\n";
    cout<<"Developed and Maintained by Rushikesh Godase"<<endl;
    cout<<"ps	: Display all information of process"<<endl;
    cout<<"ps -t   : Display all information about threads"<<endl;
	cout<<"ps -d   : Display all information about DLL"<<endl;
    cout<<"cls	: Clear the contents on console"<<endl;
    cout<<"log	: Creates log current running process on G drive"<<endl;
	cout<<"readlog :Display the information from specified log file"<<endl;
    cout<<"sysinfo : Display the current hardware configuration "<<endl;
    cout<<"search  : Search and display information of specific running process"<<endl;
	cout<<"kill    :Terminate the specific process"<<endl;
    cout<<"exit    : Terminate ProcMon "<<endl;
	cout<<"help    :Display Commands for ProcMon"<<endl;

}


/////////////////////////////////////////////////////////////////////////////////////////////
//
//	Name			:manpage
//	Input			:char*
//	Output			:void
//	Description		:displays how to execute particular command
//	Author			:Rushikesh Godase	
//	Date			:18 Aug 2020
//
/////////////////////////////////////////////////////////////////////////////////////////////
void man(char* cname)
{
	if(cname == NULL)
		return;

	if(strcmpi(cname, "ps") == 0)
	{
		cout << "DESCRIPTION:\n\t Process status\n";
		cout << "USAGE:\n\tps,\tDisplays information about process\n";
		cout << "\tps -t,\tDisplays Threads associated with processes\n";
		cout << "\tps -d,\tDisplays DLLs associated with processes\n";
	}
	else if(strcmpi(cname, "log") == 0)
	{
		cout << "DESCRIPTION:\n\t Creates log file\n";
		cout << "USAGE:\n\t log\n";
	}
	else if(strcmpi(cname, "readlog") == 0)
	{
		cout << "DESCRIPTION:\n\t reads log file and display its contents\n";
		cout << "USAGE:\n\t readlog\n";
	}
	else if(strcmpi(cname, "search") == 0)
	{
		cout << "DESCRIPTION:\n\t searches particular process \n";
		cout << "USAGE:\n\t search Process_Name\n";
	}
	else if(strcmpi(cname, "kill") == 0)
	{
		cout << "DESCRIPTION:\n\t kills particular process\n";
		cout << "USAGE:\n\t kill Process_Name\n";
	}
	else if(strcmpi(cname, "cls") == 0)
	{
		cout << "DESCRIPTION:\n\t clears onsole\n";
		cout << "USAGE:\n\t cls\n";
	}
	else if(strcmpi(cname, "exit") == 0)
	{
		cout << "DESCRIPTION:\n\t terminates from shell\n";
		cout << "USAGE:\n\t exit\n";
	}
	else if(strcmpi(cname, "help") == 0)
	{
		cout << "DESCRIPTION:\n\t displays help menu\n";
		cout << "USAGE:\n\t help\n";
	}
	else if(strcmpi(cname, "sysinfo") == 0)
	{
		cout << "DESCRIPTION:\n\t displays hardware information of running system\n";
		cout << "USAGE:\n\t sysinfo\n";
	}
	else
	{
		cout << "ERROR : Manual entry not found!!!\n";
	}

}

//Entry-Point function
int main(int argc , char*argv[])
{
    BOOL bRet = FALSE , bRunning = TRUE;
    char *ptr = NULL;
    ProcessInfo *ppobj = NULL; //class pointer created
    char command[4][80] , str[80]; //horizontal length of console is 80 [max buffer length in VS]
    int count , min , date , hr,month;

    while(bRunning)
    {
        fflush(stdin);	//clean input [keyboard] buffer
        strcpy(str,"");	// clear or clean str

        cout<<endl<<"\nMarvellous ProcMon : >";  //Name of shell
        fgets(str,80,stdin);  /*taking data from keyboard to buffer str(in what,how many bytes,from where)
								stdin is also treated as file.
								largest data that we can receive from user should be less than 80Bytes.
								you can also use special scanf.
								cin or scanf can't control how much data sould receive.
								*/
		/*
		string scanf(sscanf) : it accept the input from string.
		we are spliting data into words using sscanf [space is delimitor]
		if it founds space its going to split it into different word.
		*/
        count = sscanf(str,"%s%s%s%s",command[0],command[1],command[2],command[3]);//tokennise
		/* eg.command = ps -t 
			command[0] = ps , command[1] = -t , command[2] = 0 , command[3] = 0
		*/

		//count will be 2 according to above example[depends on how many fragments(words) are there]
        if( count==1 )// no. of words is 1
        {	
			/* _stricmp();
				string comparison without considering a case.
				i indicates ignore case.
			*/
            if(_stricmp(command[0],"ps")==0) 
            {
                ppobj = new ProcessInfo();		//object created of class ProcessInfo() [composition]

				//if user only enters command as "ps" then we are passing it as "ps -a"
                bRet = ppobj->ProcessDisplay("-a");

                if(bRet == FALSE )
				{
                    cout<<"Error : Unable to display process"<<endl;
				}

                delete ppobj;
                
            }
            else if(_stricmp(command[0],"log")==0)
            {

                ppobj = new ProcessInfo(); 
                bRet  = ppobj->ProcessLog();

                if(bRet==FALSE)
				{
                    cout<<"Error  : Unable to create log file"<<endl;
				}

                delete ppobj;    
            }
            else if(_stricmp(command[0],"sysinfo")==0)
            {
                bRet = HardwareInfo();

                if(bRet ==FALSE)
				{
                    cout<<"Error : Unable to get hardware information"<<endl;
				}
                    
            }
            else if(_stricmp(command[0],"readlog")==0)
            {
                //ProcessInfo *ppobj;
                ppobj = new ProcessInfo();

                cout<<"Enter log file details as :"<<endl;

                cout<<"Hour : ";
                cin>>hr;

                cout<<endl<<"Minute : ";
                cin>>min;

                cout<<endl<<"Date : ";
                cin>>date;

                cout<<endl<<"Month : ";
                cin>>month;

                bRet = ppobj->ReadLog(hr,min,date,month);

                if( bRet == FALSE)
				{
                    cout<<"Error : Unable to read specified log file "<<endl;
				}

				delete ppobj;
            }
            else if(_stricmp(command[0],"clear")==0)
            {
                system("cls");  //we are command sending to system prompt
                continue;
            }
            else if(_stricmp(command[0],"help")==0)
            {
                DisplayHelp();
                continue;
            }
            else if(_stricmp(command[0],"exit")==0)
            {
                cout<<endl<<"Terminating the Marvellous ProcMon"<<endl;
                bRunning = FALSE;
            }
            else
            {
                cout<<endl<<"Error  : Command not Found !!"<<endl;
                continue;
            }
        }
        else if(count==2)// words in command are 2
        {
                if(_stricmp(command[0],"ps")==0)
                {
                    ppobj = new ProcessInfo();

					if( (_stricmp(command[1] ,"-d") != 0) &&  (_stricmp(command[1] ,"-t") != 0) && (_stricmp(command[1] ,"-a") != 0))
					{
						cout<<"\nPlease Enter Valid Flag.[-t,-d,-a]";
						continue;
					}

                    bRet = ppobj->ProcessDisplay(command[1]);

                    if(bRet==FALSE)
					{
                        cout<<"Error : Unable to display process information"<<endl;
					}

					delete ppobj;
                }
                else if(_stricmp(command[0],"search")==0)
                {
                    ppobj = new ProcessInfo();

                    bRet = ppobj->ProcessSearch(command[1]);

                    if(bRet==FALSE)
					{
                        cout<<"Error : There is no such process"<<endl;
					}          
	
					delete ppobj;

                    continue;
                }
                else if(_stricmp(command[0],"kill")==0)
                {
                    ppobj = new ProcessInfo();  //snapshot inside constructor
												//we are taking snapshot everytime
                    bRet = ppobj->KillProcess(command[1]);


                    if(bRet==FALSE)
					{
                        cout<<"Error : There is no such process"<<endl;
					}
                    else
					{
						cout<<command[1]<<"\tTerminated successfully"<<endl;
					}

					delete ppobj;

                    continue;
                    
                }
				else if(_stricmp(command[0], "man") == 0)
				{
					man(command[1]);
				}
        }
        else
        {
            cout<<endl<<"Error : Command not Found!!"<<endl;
            continue;
        }
        
    }//end of while

    return 0;
}

/*
what is meant by shell?
	Shell is infinite reading loop.
*/
