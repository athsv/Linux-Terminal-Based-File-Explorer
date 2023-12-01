#include <unistd.h>
#include<bits/stdc++.h>
#include <iostream>
#include <iomanip>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <pwd.h>
#include <grp.h>
#include<stdio.h>

using namespace std;

#define MOVE_CURSER printf("%c[%d;%dH",27,posx,posy)

int directory_Listing(const char*);
void display(const char*);
int FilesToPrint();
bool isRegularFile(const char*path);
void split_char_to_string();
string SplitFilename(string);
bool searchFileorDir(string cur,string toFind);
void clearLastLine();
bool searchCommand(vector<string> cmdL);
int commandMode();
void split_command();
string create_absolute_path(string);
void update_list();
bool isDirectory(string);
int isFileExist(string);
void DeleteSingleFile(string path);
void removeFiles(vector<string> list);
void DeleteSingleDir(string dirToDel);
void removeDirectories(vector<string> cmdList);
void copySingleFile(string fromFile, string toFile);
void CopySingleDirectory(string from, string to);
bool my_copy(vector<string> cmdList);
void createFile(vector<string> cmdList);
void createDirectory(vector<string> cmdList);
void renameFiles(vector<string> list);
void movecommand(vector<string> list);
void enableRawMode();



int g_argc; 
char** g_argv;
char root[4096]; 
char cur_directory[4096]; 
vector<string> directoryList; 
stack<string> back_stack; 
stack<string> forw_stack; 
int totalFiles;
vector<char> command_string; 
char homepath[4096];
unsigned int term_row_num;
unsigned int term_col_num; 
unsigned int posx; 
unsigned int posy; 
int cur_window; 
struct winsize terminalWindow; 
struct termios raw, newraw;
vector<string> my_command; 


//This function will make single or multiple directories
void createDirectory(vector<string> commandlist)
{    
    unsigned int leng=commandlist.size();
    if(leng<=2)
    {                          
        printf("less parameters than expected:\n");
        return;
    }
    else{
        string dest_dir=create_absolute_path(commandlist[leng-1]);       
        //check if destination is directory or not
        if(!isDirectory(dest_dir)){
            cout<<endl;
            cout<<"\033[0;31m"<<"In the command The destination is not valid directory "<<dest_dir<<endl;
            cout<<"\033[0m";
            cout<<":";       
            return;
        }

        int i=1;

        while(i<leng-1){    
            string Dir=dest_dir+"/"+commandlist[i];
            if(mkdir(Dir.c_str(),S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)==-1){      
                cout<<endl;
                cout<<"\033[0;31m"<<" Error in Creating Directory in path "<<dest_dir<<endl;
                cout<<"\033[0m";
                cout<<":";               
            }
            i++;  
        }
    }
    return;
}


//listing directories
int directory_Listing(const char* path){
   DIR *pwdir ;
   struct dirent *entity; 
   pwdir=opendir(path);
   if(pwdir == NULL){
        perror("opendir");
        return -1;
   }
   
   directoryList.clear();
   while ((entity=readdir(pwdir))){
        if((string(entity->d_name)=="..")&&(strcmp(path,root)==0)){
            continue;
        }
        else{
            directoryList.push_back(string(entity->d_name));
        }
   }
        
   posx = 1;
   MOVE_CURSER;
   sort(directoryList.begin(),directoryList.end());
   cout<<"\033c";
   cout<<"\u001b[33m";
   update_list();
   posy=80;
   closedir(pwdir);
   return 0;
}

void update_list()
{
    cout<<"\033c";
    posy=1;
    MOVE_CURSER;
    totalFiles=directoryList.size();
    unsigned int len=FilesToPrint();
    for (unsigned int i=0,itr=1;(i<totalFiles)&&(itr <= len);i++,itr++){
        string filename=directoryList[i];
        display(filename.c_str());
    }
    return;
}

void display(const char* dirname)
{
    struct stat thestat;
    struct passwd *tf; 
    struct group *gf;
    string finalpath;
    string dir=string(cur_directory) ;
    string current=string(dirname);
    finalpath=dir+"/"+current ;
    char *path=new char[finalpath.length()+1];
    strcpy(path,finalpath.c_str());
    
    if (stat(path,&thestat)==-1){
        perror("lstat");
    }
       
    printf((S_ISDIR(thestat.st_mode)) ? "d" : "-");
    cout<<( (thestat.st_mode & S_IRUSR) ? "r" : "-");
    cout<<( (thestat.st_mode & S_IWUSR) ? "w" : "-");
    cout<<( (thestat.st_mode & S_IXUSR) ? "x" : "-");
    cout<<( (thestat.st_mode & S_IRGRP) ? "r" : "-");
    cout<<( (thestat.st_mode & S_IWGRP) ? "w" : "-");
    cout<<( (thestat.st_mode & S_IXGRP) ? "x" : "-");
    cout<<( (thestat.st_mode & S_IROTH) ? "r" : "-");
    cout<<( (thestat.st_mode & S_IWOTH) ? "w" : "-");       
    cout<<string(1,'\t');

    
    tf = getpwuid(thestat.st_uid);
    printf("%10s ",tf->pw_name);     
    cout<<string(1,'\t');

    gf = getgrgid(thestat.st_gid);
    printf("%10s ", gf->gr_name);

    //if else construct for converting file size from bytes to KB,MB,GB
    long long file_size=thestat.st_size;
    if(file_size>=(1<<30)){
        printf("%4lldG ",file_size/(1<<30));
    }
    else if(file_size>=(1<<20)){
        printf("%4lldM ",file_size/(1<<20));
    }
    else if(file_size>=(1<<10)){
        printf("%4lldK ",file_size/(1<<10));
    }
    else{
        printf("%4lldB ",file_size);
    }
        
    char *tt=(ctime(&thestat.st_mtime));
    tt[strlen(tt)-1]='\0';
    printf("%-30s",tt);
    if(S_ISDIR(thestat.st_mode)){
        printf("\033[1;32m");
        printf("\t%-20s\n",dirname);
        printf("\033[0m");
    }
    else{
        printf("\t%-20s\n",dirname);
    }
}

//Enabling non-canonical mode

void enableRawMode()
{
    //getting the initial terminal settings
    tcgetattr(STDIN_FILENO,&raw);
    newraw=raw;
    //changing the flags for entering Non Canonical mode.
    newraw.c_lflag&=~(ICANON | ECHO | IEXTEN | ISIG);
    newraw.c_iflag&=~(BRKINT);
    

    if(tcsetattr(STDIN_FILENO,TCSAFLUSH,&newraw)!=0){
        fprintf(stderr, "Failed in  setting  attributes\n");
    }
    else{
        char inputKey[3];
        memset(inputKey,0,3*sizeof(inputKey[0]));

        posx=1;
        posy=80;
        while(true){
            unsigned int lastPos=terminalWindow.ws_row-1;
            printf("%c[%d;%dH", 27, lastPos ,1);
            cout<<"*****NORMAL MODE*****";
            printf("%c[%d;%dH",27,posx,posy);
            unsigned int x=posx+cur_window;
            fflush(0);
            if(read(STDIN_FILENO, inputKey, 3)==0)
                //0 bytes are read
                continue;
            else if(inputKey[2]=='A'){
                  if(posx+cur_window>1){
                        posx--;
                        if(posx>=1){
                            printf("%c[%d;%dH",27,posx,posy);
                        }
                        else if((x>= 1) && (posx <= 0)){
                            printf("%c[2J",27);
                            if(cur_window>0){
                                cur_window--;
                            }
                           
                            printf("%c[%d;%dH",27,1,1);
                            unsigned int i;
                            i=cur_window;
                            while(i<=term_row_num+cur_window-1){
                                string fName=directoryList[i];
                                display(fName.c_str());
                                i++;
                            }
                            posx++;
                            printf("%c[%d;%dH",27,posx,posy);
                        }
                    }
            }
            else if(inputKey[2]=='B'){
               int lenRecord;
                    if(x<(totalFiles)){
                        posx++;
                        if(posx<=term_row_num){
                            printf("%c[%d;%dH",27,posx,posy);
                        }
                        else if((posx>term_row_num)&&(x <= totalFiles)){
                            printf("%c[2J",27);
                            lenRecord = FilesToPrint()-1;
                            if(totalFiles>term_row_num){
                                cur_window++;
                            }
                            
                            printf("%c[%d;%dH",27,1,1);
                            unsigned int i;
                            for (i=cur_window;i<=lenRecord+cur_window;i++)
                            {
                                string fName=directoryList[i];
                                display(fName.c_str());
                            }
                            posx--;
                        }
                        printf("%c[%d;%dH",27,posx,posy);
                    }
            }
            else if(inputKey[2]=='C'){
                //if forward stack contains someting then we should go to top directory
                if(!forw_stack.empty()){
                    back_stack.push(cur_directory);
                    string gotoD=forw_stack.top();
                     strcpy(cur_directory,gotoD.c_str());
                    forw_stack.pop();
                    
                    directory_Listing(cur_directory);
                }
            }
            else if(inputKey[2]=='D'){
               //if forward stack contains someting then we should go to top directory
                int s=back_stack.size() ;
                if(s>=2){
                    string gotoD=back_stack.top();
                    back_stack.pop();
                    forw_stack.push(string(cur_directory));
                    strcpy(cur_directory,gotoD.c_str());
                    directory_Listing(cur_directory);
                 }
            }
            else if(inputKey[0]=='H' || inputKey[0]=='h'){
                back_stack.push(cur_directory);
                forw_stack.push(homepath);
                directory_Listing(homepath);
            }
            //backspace key
            else if(inputKey[0]==127){
                string s=string(cur_directory);
                if(strcmp(cur_directory,homepath)==0){
                    continue;
                }
                int posoflastslash=0;
                unsigned int i=0;
                while(i<s.size()){
                //for(int i=0;i<s.length();i++){
                    if(s[i]=='/'){
                        posoflastslash=i;
                    }
                    i++;
                }

                string parent=s.substr(0,posoflastslash);
                back_stack.push(cur_directory);
                forw_stack.push(parent);
                strcpy(cur_directory,parent.c_str());
                directory_Listing(cur_directory);
            }
            //enter key
            else if(inputKey[0]==10){
                string parent="..";
                string current=".";
                string myDir=directoryList[cur_window+posx-1];
                if(myDir==current){
                    continue;
                }
                else if(myDir==parent){
                    if(!(cur_directory==homepath)){
                        string s=string(cur_directory);
                        int posoflastslash=0;
                        int i=0;
                        while(i<s.size()){
                            if(s[i]=='/'){
                                posoflastslash=i;
                            }
                        }

                        string parent=s.substr(0,posoflastslash);
                        back_stack.push(cur_directory);
                        forw_stack.push(parent);
                        strcpy(cur_directory,parent.c_str());
                        directory_Listing(cur_directory);
                    }                        
                }
                else{
                    //get the full path 
                    string fullP=string(cur_directory)+"/"+myDir;
                    char *fpath = new char[fullP.length()+1];
                    strcpy(fpath,fullP.c_str());

                if(isDirectory(fpath)){
                        back_stack.push(string(cur_directory));
                        strcpy(cur_directory,fpath);
                    // back_stack.push(string(cur_directory));
                    while (!forw_stack.empty()){
                        forw_stack.pop();
                    }
                    forw_stack.push(string(cur_directory));
                    printf("%c[%d;%dH",27,1,1);
                    directory_Listing(cur_directory);
                }
                else if(isRegularFile(fpath)){
                    pid_t pid=fork();
                    if(pid==0){
                        char* parameters[3] ={"vi",fpath,NULL };
                        execvp("vi",parameters);
                    }
                    else{
                        int *c;
                        wait(c);
                    }  
                }
            }
        }
            else if(inputKey[0]==':'){
                printf("%c[%d;%dH",27,lastPos,1);
                printf("%c[2K",27);
                cout << ":";
                // going into command mode
                int result=commandMode();
                posx=1;
                printf("%c[%d;%dH",27,posx,posy);
                if (result==1){
                    directory_Listing(cur_directory);
                    //cout<<"goto out : ";
                }
                else if(result==2){
                    //cout<<"search out : ";
                    //doing nothing in this case
                }
                else
                {
                   directory_Listing(cur_directory);
                }
                
            }
            else if(inputKey[0]=='q') {
                write(STDOUT_FILENO,"\x1b[2J",4);
                tcsetattr(STDIN_FILENO,TCSAFLUSH,&raw);
                posx=1;
                posy=1;
                MOVE_CURSER;
                exit(1);
            }
            fflush(0);
            memset(inputKey,0,3*sizeof(inputKey[0]));
        }
    }
}


//Command Mode
int commandMode(){
        posx = terminalWindow.ws_row-1;
        posy = 1;
        MOVE_CURSER;
        printf("\x1b[0K");
        printf(":");
        fflush(0);
        posy++;
         
        char inp[3];
        memset(inp,0,3*sizeof(inp[0]));
        command_string.clear();
        my_command.clear();
        while(true){
            if(read(STDIN_FILENO,inp,3)==0)
                continue;
            if((inp[0]==27)&&(inp[1]==0)&&(inp[2]==0)){
                posx=1;
                posy=80;
                MOVE_CURSER;
                return 0;
            }
            else if((inp[0]==27)&&(inp[1] == '[')&&(inp[2]=='A' || inp[2] == 'B'|| inp[2] == 'C' || inp[2] == 'D')) {
                continue;
            }
            else if(inp[0]==10){
                command_string.push_back('\n');
                split_char_to_string();
                if(my_command.size()>=1){
                string s=my_command[0];
                if(s=="copy"){
                    my_copy(my_command);
                    // cout<<"my_copy()"<<endl;
                    posy=2;
                    command_string.clear();
                    clearLastLine();
                }
                else if(s=="move"){
                    movecommand(my_command);
                    posy=2;
                    command_string.clear();
                    clearLastLine();
                 }
                else if(s=="rename"){
                    renameFiles(my_command);
                    posy=2;
                    command_string.clear();
                    clearLastLine();
                }
                else if(s=="create_file"){
                    createFile(my_command);
                    posy=2;
                    command_string.clear();
                    clearLastLine();
                }
                else if(s=="create_dir"){
                    createDirectory(my_command);
                    posy=2;
                    command_string.clear();
                    clearLastLine();
                }
                else if(s=="delete_file"){
                    removeFiles(my_command);
                    posy=2;
                    command_string.clear();
                    clearLastLine();
                }
                else if(s=="delete_dir"){
                    removeDirectories(my_command);
                    posy=2;
                    command_string.clear();
                    clearLastLine();
                }
                else if(s=="goto"){
                    string my_path ="";
                    if(my_command.size()!=2){
                        cout<<endl;
                        cout<<"\033[0;31m"<<" Invalid command "<<endl;
                        cout<<"\033[0m";
                        cout<<":";
                     }
                     else{
                        my_path = create_absolute_path(my_command[1]);
                     }

                    back_stack.push(string(cur_directory));
                    while(!forw_stack.empty()){
                        forw_stack.pop();
                    }
                    strcpy(cur_directory, my_path.c_str());
                    clearLastLine();
                    return 1;
                }
                else if(s=="search"){
                   // cout<<"here  "<<endl;
                   bool found=searchCommand(my_command);
                   unsigned int last=term_row_num ;
                   if(found == true){
                   printf("%c[%d;%dH",27,last,1);
                   printf("%c[2K",27);
                   cout<<"Last Search Result:True";
                   }
                   else{
                    printf("%c[%d;%dH",27,last,1);
                    printf("%c[2K",27);
                    cout<<"Last Search Result : False "<<endl;
                   }
                   posy=2;
                   command_string.clear();
                   printf("%c[%d;%dH",27,last+1,1);
                   printf("%c[2K",27);
                   cout<<":";
                }
                else{
                    cout<<endl;
                    cout<<"\033[0;31m"<<" Command not found  " <<endl;
                    cout<<"\033[0m";
                    cout<<":";
                    clearLastLine();
                }
            }
            else{
                clearLastLine(); 
            }
               
        }
            else if(inp[0]==127){
                if (posy>2) {
                    posy--;
                    MOVE_CURSER;
                    printf("\x1b[0K");
                    command_string.pop_back();
                }
            }
            else{
                cout<<inp[0];
                posy++;
                MOVE_CURSER;
                command_string.push_back(inp[0]);
            }
            fflush(0);
            memset(inp,0,3*sizeof(inp[0]));
        }
    return 0;
}
 

//Creating files

//This function makes single or multiple files
void createFile(vector<string> cmdList)
{
    unsigned int len=cmdList.size();
    if(len<3)
    {
        cout<<"\033[0;31m"<<" Few parameters in the command given than expected"<<endl;
        cout<<"\033[0m";
        cout<<":";
    }
    else{
        string destDir= create_absolute_path(cmdList[len-1]);
        if (!isDirectory(destDir)) {
            cout<<"The destination given is not a valid directory " << endl;
            return;
        }
        

        unsigned int i=1;
        while(i<cmdList.size()-1){
        //for (unsigned int i = 1; i < cmdList.size()-1; i++) {
            string filePath= destDir +"/"+cmdList[i];
            int status=open(filePath.c_str(),O_RDONLY | O_CREAT,S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH );   
            if (status == -1)
            {
                cout<<endl;
                cout<<"\033[0;31m"<<" Error creating file "<<endl;
                cout<<"\033[0m";
                cout<<":";          
            }
            i++;
        }
    }
    return;
}

//This function will delete many directories

void removeDirectories(vector<string> cmdList)
{
    if(cmdList.size()<2)
    {
        cout<<endl;
        cout<<"\033[0;31m"<<" Less parameters than expected"<<endl;
        cout<<"\033[0m";
        cout<<":";  
    }
    unsigned int i=1;
    while(i<cmdList.size()){
    //for(unsigned int i=1;i<cmdList.size();i++)
        DeleteSingleDir(create_absolute_path( cmdList[i]));
    }    
}

//This function will delete a single directory
 
void DeleteSingleFile(string path)
{
    int status=remove(path.c_str());
    if(status!=0)
     {
        perror("");
     }   
}


//This function will delete many files
void removeFiles(vector<string> list)
{   
    if(list.size()<2)
    {
        perror("");
    }

    unsigned int i=1;
    while(i<list.size()){
    //for(unsigned int i=1;i<list.size();i++)
        string fileToDelete = create_absolute_path(list[i]);
        DeleteSingleFile(fileToDelete);
        i++;
    }    
}

//This function will delete a single file
 void DeleteSingleDir(string dirToDel)
{
    struct dirent* entity;
    DIR* folder;
    folder = opendir(dirToDel.c_str());
   
    if(folder==NULL){
        perror("opendir");
        return;
    }
    
    string curr=".";
    string parent="..";

    while((entity=readdir(folder))){
        string dname=string(entity->d_name);
        string dirToDel_path = dirToDel+"/"+dname;
           
        if(dname == curr || dname == parent)
            continue;
        else {
            if (isDirectory(dirToDel_path)) {
                DeleteSingleDir(dirToDel_path);
            }
            else {
                DeleteSingleFile(dirToDel_path);
            }
        }
    }
    closedir(folder);
    remove(dirToDel.c_str());
}

//Moving and Deleting files

void copySingleFile(string fromFile, string toFile)
{
    char ch;
    FILE *fromFile_f,*toFile_f;
    fromFile_f=fopen(fromFile.c_str(),"r");
    toFile_f=fopen(toFile.c_str(),"w");
    if(fromFile_f==NULL) {
        perror("");
        return;
    }
    if(toFile_f==NULL) {
        perror("");
        return;
    }
    while((ch=getc(fromFile_f))!=EOF){
        putc(ch,toFile_f);
    }

    struct stat fromFile_stat;
    stat(fromFile.c_str(),&fromFile_stat);
    chown(toFile.c_str(),fromFile_stat.st_uid, fromFile_stat.st_gid);
    chmod(toFile.c_str(),fromFile_stat.st_mode);
    fclose(fromFile_f);
    fclose(toFile_f); 
}

void CopySingleDirectory(string from, string to)
{
    struct dirent* entity;
    DIR* folder;
    folder=opendir(from.c_str());
   
    if (folder==NULL) {
        perror("opendir");
        return;
    }
    
    string curr=".";
    string parent="..";

    while ((entity=readdir(folder))) {
        string dname=string(entity->d_name);
        string from_path=from+"/"+dname;
        string to_path=to+"/"+dname;
        if(dname == curr || dname == parent)
            continue;
        else {
            if(isDirectory(from_path)){
                if(mkdir(to_path.c_str(),0755)==-1) {
                    perror("");
                    return;
                }
                else{
                    CopySingleDirectory(from_path,to_path);
                }
            }
            else{
                copySingleFile(from_path,to_path);
            }
        }
    }
    closedir(folder);
    return;
}

bool my_copy(vector<string> cmdList)
{
    if(cmdList.size()<3){
            cout<<"\033[0;31m"<<" Less parameters given in the command than expected "<<endl;
            cout<<"\033[0m";
            cout<<":";
            return false;
    }
    else{
        unsigned int leng=cmdList.size();
        string destination=create_absolute_path(cmdList[leng-1]);
        if(!isDirectory(destination)){          
            return false;
        }
        for(unsigned int i = 1;i<leng-1;i++){
            string from_path=create_absolute_path(cmdList[i]);
            int posoflastslash=0;
            for(int i=0;i<from_path.length();i++){
                if(from_path[i]=='/'){
                    posoflastslash=i;
                }
            }

            string to_path = destination+"/"+from_path.substr(posoflastslash+1,from_path.length()-posoflastslash);
            if(isDirectory(from_path)) {
                if (mkdir(to_path.c_str(),0755)!= 0) {
                    perror("");
                    return false;
                }
                CopySingleDirectory(from_path,to_path);
            }
            else{
                copySingleFile(from_path,to_path);
            }
        }
    }
    return true;
}


void movecommand(vector<string> list)
{
    unsigned int leng = list.size();
    if(leng<3)
    {
       perror("");
    }
    else{
        for(unsigned int i=1;i<leng-1;i++){           
            string newData=create_absolute_path(list[i]);
            string s=newData;
            int posoflastslash=0;
            for(int i=0;i<s.length();i++){
                   if(s[i]=='/'){
                    posoflastslash=i;
                    }
            }
            
            string name;
            name=s.substr(posoflastslash+1,s.length());
            string destpath= create_absolute_path( list[leng-1]);
            destpath=destpath+"/"+name;
            if(isDirectory(newData)){
                if(mkdir(destpath.c_str(),0755)==-1) {
                    perror("");
                    return;
                }
                else{
                    //for moving directory, first copy directory then delete 
                    CopySingleDirectory(newData,destpath);
                    DeleteSingleDir(newData);
                }
            }
            else{
                //for moving file, first copy file then delete 
                copySingleFile(newData,destpath);
                DeleteSingleFile(newData);
            }
        }
    }
}


//This function will convert relative path to absolute path
string create_absolute_path(string str)
{
    string abs="";
    char firstchar = str[0];
    
    string basepath = string(root);
    if(firstchar=='.'){
        abs = string(cur_directory) + str.substr(1,str.length());    
    }
    
    else if(firstchar=='~'){
        abs=basepath+str.substr(1,str.length());
    }
    else if(firstchar =='/'){
        abs=basepath+str;
    }
    else{
        abs= string(cur_directory)+ "/" + str;
    }
    return abs;
}

int FilesToPrint()
{
    int lenRecord;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &terminalWindow);
    term_row_num= terminalWindow.ws_row - 2;
    term_col_num=terminalWindow.ws_col;
    if (totalFiles <= term_row_num){
        lenRecord = totalFiles;
    }
    else{
        lenRecord = term_row_num;
    }
    return lenRecord;
}

//this function will check if directory is present or not
bool isDirectory(string path)
{
    struct stat obj;
    stat(path.c_str(), &obj);
    return ((obj.st_mode & S_IFMT) == S_IFDIR)?true:false;
}

//function will check whether path denotes a regular file
bool isRegularFile(const char*path)
{
    struct stat obj;
    stat(path, &obj);
    return ((obj.st_mode & S_IFMT)==S_IFREG)?true:false;
}

void split_char_to_string()
{
    my_command.clear();
    string temp="";
    unsigned int size=command_string.size();
    cout<<endl;
    for(unsigned int i=0;i<size;i++){
        if(command_string[i]==' ' || command_string[i]=='\n'){
            if(temp.size()>0){
                my_command.push_back(temp);
            }
            temp="";
        }
        else if(command_string[i]=='\\'){
            i++;
            temp=temp+command_string[i];
        }
        else{
            temp=temp+command_string[i];
        }
    }
}

void clearLastLine()
{
    unsigned int lastLine=term_row_num +1;
    printf("%c[%d;%dH",27,lastLine,1);
    printf("%c[2K",27);
    cout<<":";
}

void renameFiles(vector<string> list)
{
    if(list.size()!=3){
        // showError("Invalid Argument in Renaming !!!");
    }
    else{
        string initName = create_absolute_path( list[1]);
        string finalName = create_absolute_path(list[2]);
        rename(initName.c_str(),finalName.c_str());
    }
}

//Function to serch files and directories
bool searchCommand(vector<string> cmdL)
{
    if(cmdL.size()!=2){
        cout<<" incorrect no of parameters"<<endl;
        return false;
    }
    string s=string(cur_directory);
    return searchFileorDir(s,cmdL[1]);
}
 
bool searchFileorDir(string cur,string toFind)
{
    struct dirent* d;
    DIR* folder;
    folder = opendir(cur.c_str());
    if(folder == NULL) {
        perror("opendir");
        return false;
    }

    string curr=".";
    string parent="..";
    while((d = readdir(folder))){
        string dname=string(d->d_name);
        string nextDir=cur +"/"+dname;
        if(dname == curr || dname == parent){
            continue;
        }
        else{           
            if(isDirectory(nextDir)){
                if(dname == toFind){
                    return true; 
                }
                else{
                    bool found =searchFileorDir(nextDir,toFind);
                    if(found == true){
                        return true;
                    }
                }
            }
            else{
                if(dname == toFind){
                    return true;     
                }
            }
        }
    }
    closedir(folder);
    return false;
 }

 int main(int argc,char* argv[]){
    g_argc=argc;
    g_argv=argv;
    if (g_argc==1){
        string str = ".";
        
        strcpy(root,get_current_dir_name());  
        strcat(cur_directory, root);
        strcpy(homepath,root);
        back_stack.push(root);
         //back_stack saves history of visited dir.      
        directory_Listing(root);
    }
    else if(argc==2){
        strcpy(root,g_argv[1]);
        strcpy(homepath,root);
        strcat(cur_directory,root);
        
        back_stack.push(root);
        directory_Listing(root);
    }
    else{
        cout<<"Incorrect parameters"<<endl;
    }
    enableRawMode();
}
