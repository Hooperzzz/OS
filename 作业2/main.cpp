#include<iostream>
#include<string>
#include<vector>
#include<sstream>

using namespace std;
typedef unsigned char B;
typedef unsigned short B2;

#pragma pack(1)  //设置按字节对齐
struct Boot{
    B BS_jmpBoot[3];   //跳转指令
    B BS_OEMName[8];   //厂商名
    B2 BPB_BytsPerSec;  //每扇区字节数
    B BPB_SecPerClus; //每簇扇区数
    B2 BPB_RsvdSecCnt;  //Boot记录占用多少扇区
    B BPB_NumFATs;    //共有多少FAT表
    B2 BPB_RootEntCnt; //根目录文件数最大值
    B2 BPB_TotSec16;   //扇区总数
    B BPB_Media;      //介质描述符
    B2 BPB_FATSz16;    //每FAT扇区数
    B2 BPB_SecPerTrk;  //每磁道扇区数
    B BPB_NumHeads[2];   //磁头数（面数）
    B BPB_HiddSec[4];    //隐藏扇区数
    B BPB_TotSec32[4];   //如果BPB_TotSec16是0,由这个值记录扇区数
    B BS_DrvNum[1];      //中断13的驱动器号
    B BS_Reservedl[1];    //未使用
    B BS_BootSig[1];     //扩展引导标记（29h）
    B BS_VolID[4];       //卷序列号
    B BS_VolLab[11];    //卷标
    B BS_FileSysType[8];  //文件系统类型
    B others[448];       //其他
    B2 finishTag;     //0xAA55
};
struct FileTable{
    B DIR_Name[0xB];
    B DIR_Attr;
    B unused[10];
    B2 DIR_WrtTime;
    B2 DIR_WrtDate;
    B2 DIR_FstClus;
    B DIR_FileSize[4];
};
#pragma pack()
struct FileTree{
    FileTree* neighbor=NULL;
    FileTree* children=NULL;
    bool isFile=false;
    unsigned int clusNum;
    string fileName="";
};
struct Path_input{
    string fileName;
    Path_input* childFile=NULL;
};

extern "C" void* my_print(const char* output_str,int length,int color_num);    //nasm对应输出

int initBoot(FILE*file_ptr,Boot*boot);
int initFileTree(FileTree* filetree_ptr);
FileTree* fillTreeByChildrenFile(FileTree* current,FILE* currentFile_ptr,FileTable*filetable_ptr,unsigned int current_clusNum);
FileTree* getFileByPath(FileTree* filetree_ptr,string path);
void initBasicData(Boot*boot_ptr);
void handleInputCommand(string input_command,FILE*file_ptr,FileTree*filetree_ptr);
void printAllFilesFromRoot(FileTree*filetree_ptr,string parentPath);
void printFilesByPath(FileTree*filetree_ptr,string path);
void catFileContextByPath(FileTree*filetree_ptr,string path);
void countFilesByPath(FileTree*filetree_ptr,string path);
void printCountResult(FileTree* fileTree,string head_space,string parentName);
Path_input* getFilePath(string path);

// void my_print(string* output_str,int length,int color_num){
//     cout<<*output_str;
// }


unsigned int bytesPerSec;   //每扇区字节数
unsigned int rsvdSecCnt;    //Boot记录占用多少扇区
unsigned int numFATs;       //FAT表数目
unsigned int FATSz16;       //每FAT扇区数
unsigned int rootEntCnt;    //根目录文件数最大值
unsigned int secPerClus;    //每簇扇区数
unsigned int rootFileStart;  //根目录文件开始处
unsigned int childFileStart;      //数据区开始处

const char* space=" "; //输出用空格

int main(){
    FILE *file_ptr=fopen("a.img","rb");  //获取引导文件
    Boot* boot_ptr=new Boot;
    FileTree*filetree_ptr=new FileTree;
    int initBoot_result=initBoot(file_ptr,boot_ptr);
    initBasicData(boot_ptr);
    int initFileTree_result=initFileTree(filetree_ptr);
    
    if(initBoot_result==1){
        cout<<"Load boot failed!"<<endl;
    }else{
        cout<<"Load boot success!"<<endl;
    }
    if(initFileTree_result==1){
        cout<<"Init FileTree failed!"<<endl;
    }else{
        cout<<"Init FileTree success!"<<endl;
    }

    string operation_command;
    getline(cin,operation_command);
    while(operation_command!="exit"){
        handleInputCommand(operation_command,file_ptr,filetree_ptr);
        getline(cin,operation_command);
    }

    delete(boot_ptr);
    delete(filetree_ptr);
    fclose(file_ptr);
    return 0;
}
void initBasicData(Boot*boot_ptr){
    bytesPerSec=boot_ptr->BPB_BytsPerSec;
    rsvdSecCnt=boot_ptr->BPB_RsvdSecCnt;
    numFATs=boot_ptr->BPB_NumFATs;   
    rootEntCnt=boot_ptr->BPB_RootEntCnt;  
    secPerClus=boot_ptr->BPB_SecPerClus;    
    FATSz16=boot_ptr->BPB_FATSz16;
    rootFileStart=bytesPerSec*(numFATs*FATSz16+rsvdSecCnt);
    childFileStart=bytesPerSec*(numFATs*FATSz16+rsvdSecCnt+(rootEntCnt*32+bytesPerSec-1)/bytesPerSec);
}
int initBoot(FILE*file_ptr,Boot*boot){
    fread(boot,1,0x200,file_ptr);
    return 0;
}
void getFilePath(string path,Path_input* path_ptr){
    int length=path.length();
    int index=0;
    if(path[0]=='/'){
        //从根目录开始的绝对路径
        index++;
        int filename_start;
        while(index<length){
            filename_start=index;
            while(path[index]!='/'&&index<length){
                index++;
            }
            path_ptr=path_ptr->childFile=new Path_input;
            path_ptr->fileName=path.substr(filename_start,index-filename_start);
            index++;
        }
    }
    else{
        string message="Invalid path";
        const char*c_message=message.c_str();
        my_print(c_message,message.length(),0);
        path_ptr->childFile=NULL;
    }
}
void handleInputCommand(string input_command,FILE*file_ptr,FileTree* filetree_ptr){
    int length=input_command.length();
    int count;
    if(input_command=="ls"){
        printAllFilesFromRoot(filetree_ptr,"/HOME");
    }
    else if(input_command.substr(0,2)=="ls"){
        count=2;
        while(input_command[count]==' '){
            count++;
        }
        string path=input_command.substr(count,length-count);
        printFilesByPath(filetree_ptr,path);
    }
    else if(input_command.substr(0,3)=="cat"){
        count=3;
        while(input_command[count]==' '){
            count++;
        }
        string path=input_command.substr(count,length-count);
        catFileContextByPath(filetree_ptr,path);
    }
    else if(input_command.substr(0,5)=="count"){
        count=5;
        while(input_command[count]==' '){
            count++;
        }
        string path=input_command.substr(count,length-count);
        countFilesByPath(filetree_ptr,path);
    }
    else{
        string message="Invalid input";
        const char* c_message=message.c_str();
        my_print(c_message,message.length(),0);
        cout<<endl;
    }
}
int initFileTree(FileTree* filetree_ptr){
    FileTree* current=filetree_ptr;
    FILE*rootFile_ptr=fopen("a.img","rb");    //根目录指针
    FILE*childFile_ptr=fopen("a.img","rb");   //根目录下子目录指针
    fseek(rootFile_ptr,rootFileStart,SEEK_SET);
    FileTable* filetable_ptr=new FileTable;
    string filename="";
    int count=0;
    fread(filetable_ptr,1,32,rootFile_ptr);
    while(filetable_ptr->DIR_Name[0]!=0x0&&ftell(rootFile_ptr)<childFileStart){
        if(filetable_ptr->DIR_Name[0]==0xe5){
            fread(filetable_ptr,1,32,rootFile_ptr);
            continue;
        }
        current=current->neighbor=new FileTree;
        filename="";
        for(count=0;count<0xB;count++){
            if(filetable_ptr->DIR_Name[count]!=0x20){
                filename+=filetable_ptr->DIR_Name[count];
            }
            if(count==7&&filetable_ptr->DIR_Attr!=0x10){
                filename+=".";
            }
        }
        current->fileName=filename;
        current->clusNum=filetable_ptr->DIR_FstClus;
        if(filetable_ptr->DIR_Attr==0x10){
            fseek(childFile_ptr,childFileStart+(current->clusNum-2)*secPerClus*bytesPerSec,SEEK_SET);
            current->children=fillTreeByChildrenFile(current->children,childFile_ptr,filetable_ptr,current->clusNum);
        }
        else{
            current->isFile=true;
        }
        
        fread(filetable_ptr,1,32,rootFile_ptr);
    }
    fclose(rootFile_ptr);
    fclose(childFile_ptr);
    delete(filetable_ptr);
    return 0;
}
FileTree* fillTreeByChildrenFile(FileTree* current,FILE* currentFile_ptr,FileTable* filetable_ptr,unsigned int current_clusNum){
    FILE*tempLoopIndex=fopen("a.img","rb");
    fseek(tempLoopIndex,ftell(currentFile_ptr),SEEK_SET);
    fread(filetable_ptr,1,32,tempLoopIndex);
    current=new FileTree;
    FileTree*head=current;
    while(filetable_ptr->DIR_Name[0]!=0&&ftell(tempLoopIndex)<childFileStart+(current_clusNum-1)*secPerClus*bytesPerSec){
        if(filetable_ptr->DIR_Name[0]==0xe5||filetable_ptr->DIR_Name[0]==0x2e){
            fread(filetable_ptr,1,32,tempLoopIndex);
            continue;
        }
        current=current->neighbor=new FileTree;
        for(int count=0;count<0xB;count++){
            if(filetable_ptr->DIR_Name[count]!=0x20){
                current->fileName+=filetable_ptr->DIR_Name[count];
            }
            if(count==7&&filetable_ptr->DIR_Attr!=0x10){
                current->fileName+=".";
            }
        }
        current->clusNum=filetable_ptr->DIR_FstClus;
        if(filetable_ptr->DIR_Attr==0x10){
            fseek(currentFile_ptr,childFileStart+(current->clusNum-2)*secPerClus*bytesPerSec,SEEK_SET);
            current->children=fillTreeByChildrenFile(current->children,currentFile_ptr,filetable_ptr,current->clusNum);
        }
        else{
            current->isFile=true;
        }
        fread(filetable_ptr,1,32,tempLoopIndex);
    }
    fclose(tempLoopIndex);
    return head;
}
void printAllFilesFromRoot(FileTree*filetree_ptr,string parentPath){
    //从根目录打印所有文件名
    FileTree*current=filetree_ptr;
    FileTree*head=filetree_ptr;
    my_print(parentPath.c_str(),parentPath.length(),0);
    cout<<endl;
    while(current->neighbor!=NULL){
        current=current->neighbor;
        const char* c_name=current->fileName.c_str();
        if(current->isFile){
            my_print(c_name,(current->fileName).length(),0);
        }else{
            my_print(c_name,(current->fileName).length(),1);
        }
        my_print(space,1,0);
    }
    cout<<endl;
    current=head;
    while(current->neighbor!=NULL){
        current=current->neighbor;
        if(current->children!=NULL){
            printAllFilesFromRoot(current->children,parentPath+"/"+current->fileName);
        }
    }
}
FileTree* getFileByPath(FileTree*filetree_ptr,string path){
    FileTree*current=filetree_ptr;
    Path_input*path_ptr=new Path_input;
    getFilePath(path,path_ptr);
    if(path_ptr->childFile==NULL){
        current=NULL;
    }else{
        path_ptr=path_ptr->childFile;    //第一个为HOME去掉
        while(path_ptr->childFile!=NULL){
            path_ptr=path_ptr->childFile;
            //my_print(path_ptr->fileName.c_str(),path_ptr->fileName.length(),0);
            bool noFound=true;
            while(current!=NULL&&current->neighbor!=NULL){
                current=current->neighbor;
                if(current->fileName==path_ptr->fileName){
                    noFound=false;
                    break;
                }
            }
            if(noFound){
                string message="Not Found!";
                my_print(message.c_str(),message.length(),0);
                cout<<endl;
                current=NULL;
                break;
            }else{
                if(path_ptr->childFile!=NULL){
                    current=current->children;
                }
            }
        }
    }
    return current;
}
void printFilesByPath(FileTree*filetree_ptr,string path){
    FileTree*current=getFileByPath(filetree_ptr,path);
    if(current!=NULL){
        if(current->isFile){
            string message="Is file";
            my_print(message.c_str(),message.length(),0);
            cout<<endl;
        }else{
            current=current->children;
            while(current->neighbor!=NULL){
                current=current->neighbor;
                if(current->isFile){
                    my_print(current->fileName.c_str(),(current->fileName).length(),0);
                }else{
                    my_print(current->fileName.c_str(),(current->fileName).length(),1);
                }
                my_print(space,1,0);
        }
        cout<<endl;
        }
    }
}
void catFileContextByPath(FileTree*filetree_ptr,string path){
    FileTree*current=getFileByPath(filetree_ptr,path);
    if(current!=NULL){
        if(current->isFile){
            FILE* file=fopen("a.img","rb");
            string context="";
            fseek(file,childFileStart+bytesPerSec*secPerClus*(current->clusNum-2),SEEK_SET);
            char oneByte[1];
            fread(oneByte,1,1,file);
            while(oneByte[0]!=0&&ftell(file)<childFileStart+bytesPerSec*secPerClus*(current->clusNum-1)){
                context+=oneByte[0];
                fread(oneByte,1,1,file);
            }
            my_print(context.c_str(),context.length(),0);
            cout<<endl;
            fclose(file);
        }else{
            string message="Error: is not a file!";
            my_print(message.c_str(),message.length(),0);
        }
    }
}
void countFilesByPath(FileTree*filetree_ptr,string path){
    FileTree*current=getFileByPath(filetree_ptr,path);
    if(current==NULL){
        string message="Error: Cannot find File!";
        my_print(message.c_str(),message.length(),0);
    }
    else{
        printCountResult(current,"","HOUSE");
    }
}
void printCountResult(FileTree*fileTree,string head_space,string parentName){
    int fileNum=0;
    int dirNum=0;
    FileTree*current=fileTree;
    FileTree*head=current;
    while(current->neighbor!=NULL){
        current=current->neighbor;
        if(current->isFile){
            fileNum++;
        }else{
            dirNum++;
        }
    }
    string message=head_space+parentName+": "+to_string(fileNum)+" file(s), "+to_string(dirNum)+" dir(s)";
    my_print(message.c_str(),message.length(),0);
    cout<<endl;
    current=head;
    while(current->neighbor!=NULL){
        current=current->neighbor;
        if(current->children!=NULL){
            printCountResult(current->children,head_space+"  ",current->fileName);
        }
    }
}

