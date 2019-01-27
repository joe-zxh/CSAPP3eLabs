#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <zconf.h>
#include <ctype.h>
#include "cachelab.h"


//void printArgs(int argc,char* argv[]);
//void parseArgs(int argc,char* argv[],
//               int *verbose, int *s, int *E, int*b, char traceFile[BUFSIZE]);
//void printHelp();


struct Block{
    //int setNum;//这个好像没用到
    long tag;
    //int lineNum;//这个可以算的
    int valid;
    //int count;//使用次数
    int timestamp;//用来判断哪个"更旧"，旧的要被evict掉
};

#define BUFSIZE 256
#define LONGSIZE 17 //字符串所以最后会有一个"\0"
#define LENSIZE 9 //数据不会超过8个字节的。

int hits,misses,evictions;

void processLine(char line[BUFSIZE], struct Block *blocks, int *firstEmpLine, int s, int E, int b, int verbose, int currentTime);
void line2Addr(char line[BUFSIZE], unsigned long *address, int *dataLen);
void init(struct Block *blocks, int S, int E, int *firstEmpLine);
void printArgs(int argc,char* argv[]);
void parseArgs(int argc,char* argv[], int *verbose, int *s, int *E, int*b, char traceFile[BUFSIZE]);
void printHelp();

int main(int argc,char* argv[])
{
    //printArgs(argc, argv);
    int verbose, s, E, b;
    char traceFile[BUFSIZE]="\0";//tracefile的位置
    char line[BUFSIZE];
    hits=misses=evictions=0;

    parseArgs(argc, argv, &verbose, &s, &E, &b, traceFile);


    int S = (1<<(s));
//    int B = (1<<(b));
//    int t = 64-s-b;//tag的长度


    struct Block *blocks = NULL;
    blocks = (struct Block*)malloc(S*E*sizeof(struct Block));

    int *firstEmpLine = (int *)malloc(S*sizeof(int));//对于集合i中的 第一个空的line的位置。
    //当firstEmpLine[i]==E时，说明 这个set中的所有line都用完了。

    init(blocks, S, E, firstEmpLine);


    FILE *fp = fopen(traceFile, "r");
    if(NULL == fp)
    {
        printf("failed to open dos.txt\n");
        return 1;
    }
    int currentTime = 1;

    while(!feof(fp))
    {
        memset(line, 0, sizeof(line));
        fgets(line, sizeof(line) - 1, fp); // 包含了换行符
        char *find = strchr(line, '\n');     //查找换行符
        if(find)                            //如果find不为空指针
          *find = '\0';                    //就把一个空字符放在这里
        //printf("%s", line);

        processLine(line, blocks, firstEmpLine, s, E, b, verbose, currentTime);
        currentTime++;
    }

    free(blocks);
    free(firstEmpLine);

    fclose(fp);

    printSummary(hits, misses, evictions);

    return 0;
}

void processLine(char line[BUFSIZE], struct Block *blocks, int *firstEmpLine, int s, int E, int b, int verbose, int currentTime){

    if(!line || line[0]!=' ') { //忽略读指令的操作
        return ;
    }
    if(verbose!=0){
        printf("%s", line+1);
    }

    unsigned long address;
    int dataLen;

    line2Addr(line, &address, &dataLen);

    int t=64-s-b;
//    int S = (1<<s);
//    int B = (1<<b);

    unsigned setNumber = ((address<<t)>>(t+b));
    long tag = (address>>(s+b)); //tag值

    int startInd = setNumber*E;
    int endIndx = startInd+E;
    int firstEmpInd = -1;//如果没有hit，找到第一个是空的 的block。
    // 如果循环出来还是0，那么 这个set是满的，需要evict一下

    int hitIndex = -1;

    int leastAccessInd = startInd;//如果这个set是满的，那么要找到那个 访问最少的那个block。
    //int leastCount = (1<<30);//最少的访问次数
    int oldest = 1<<30;//最旧的要被 替换掉

    //先检查有没有 hit
    for (int i = startInd;i<endIndx;i++){

        if(firstEmpInd==-1 && !blocks[i].valid){ //还没找到一个空的
            firstEmpInd = i; //找到第一个空的
        }

        if(blocks[i].tag==tag && blocks[i].valid) {
            //hit
            blocks[i].timestamp=currentTime;
            hitIndex = i;
            if(verbose!=0){
                printf(" hit");
            }

            hits++;
            break;
        }

        if(blocks[i].valid && blocks[i].timestamp<oldest){
            //更新leastCount
            leastAccessInd = i;
            oldest = blocks[i].timestamp;
        }
    }

    if(hitIndex==-1){ //没有hit，需要evict一下

        if(verbose!=0){
            printf(" miss ");
        }


        misses++;

        if(firstEmpInd==-1){ //这个set已经满了，需要evict
            //printf("eviction pull：%d",blocks[leastAccessInd].tag);
            if(verbose!=0){
                printf("eviction ");
            }

            blocks[leastAccessInd].timestamp=currentTime;
            blocks[leastAccessInd].tag=tag;

            evictions++;
        }else{//这个set还没满，从第一个空的地方开始加入
            blocks[firstEmpInd].timestamp=currentTime;
            blocks[firstEmpInd].valid=1;
            blocks[firstEmpInd].tag=tag;
        }
    }

    if(line[1]=='M'){
        if(verbose!=0){
            printf(" hit ");
        }
        hits++;
    }

    if(verbose!=0){
        printf("\n");
    }

    return ;
}

//int caleFromE(int E){
//    int e = 0;
//    while (E>1){
//        E>>=1;
//    }
//
//    return e;
//}

void line2Addr(char line[BUFSIZE], unsigned long *address, int *dataLen){

    char addrStr[LONGSIZE];
    char lenStr[LENSIZE];

    int commaPos = 3;
    while(line[commaPos]!=','){
        commaPos++;
    }
    memset(addrStr, 0, sizeof(addrStr));
    memset(lenStr, 0, sizeof(lenStr));

    strncpy(addrStr,line+3,commaPos-3);
    strncpy(lenStr, line+commaPos+1,1);//一个长度，因为不会超过8

    *address=0;
    int i = 0;
    while(1){
        addrStr[i] = tolower(addrStr[i]);

        if((addrStr[i]>='0'&&addrStr[i]<='9') || (addrStr[i]>='a'&&addrStr[i]<='f')
                                                || (addrStr[i]>='A'&&addrStr[i]<='F')){

            if(addrStr[i]>'9'){
                *address=((*address)<<4)+(10+addrStr[i]-'a');
           } else{
                *address=((*address)<<4)+(addrStr[i]-'0');
           }

        }else{
            break;
        }
        i++;
    }
    *dataLen = atoi(lenStr);//这个值不会超过9，所以直接转就行。


//    *address = atoi(addrStr); //这个地方应该不会有问题，因为不可能用满64位的地址的，所以左边第一位应该不会是1
//    *dataLen = atoi(lenStr);
}


void init(struct Block *blocks, int S, int E, int *firstEmpLine){
    int t = S*E;
    for (int i = 0;i<t;i++){
        blocks[i].valid = 0;
        blocks[i].timestamp = 0;
    }
    for(int i = 0;i<S;i++){
        firstEmpLine[i]=0;
    }
}

void printArgs(int argc,char* argv[]){
    for (int i = 0;i<argc;i++){
        printf("%d: %s\n",i, argv[i]);
    }
}

void parseArgs(int argc,char* argv[],
               int *verbose, int *s, int *E, int*b, char traceFile[BUFSIZE]){
    char *_h = "-h";
    char *_v = "-v"; *verbose = 0;//显示trace的信息
    char *_s = "-s"; *s=-1;
    char *_E = "-E"; *E=-1;
    char *_b = "-b"; *b=-1;
    char *_t = "-t";//traceFile name

    for (int i = 1;i<argc;i++){
        if(strcmp(argv[i], _h)==0){
            printHelp();
            continue;
        }
        if(strcmp(argv[i], _v)==0){
            *verbose = 1;
            continue;
        }
        if(strcmp(argv[i], _s)==0){
            *s=atoi(argv[i+1]);
            i +=1;
            continue;
        }
        if(strcmp(argv[i], _E)==0){
            *E=atoi(argv[i+1]);
            i +=1;
            continue;
        }
        if(strcmp(argv[i], _b)==0){
            *b=atoi(argv[i+1]);
            i +=1;
            continue;
        }
        if(strcmp(argv[i], _t)==0){
            getcwd(traceFile,BUFSIZE);
            //printf("%s\n",traceFile);
            strcat(traceFile,"/");
            strcat(traceFile,argv[i+1]);
            i +=1;
            //printf("%s\n",traceFile);
            continue;
        }
    }

    //检查一下
    if(*s==-1||*E==-1||*b==-1){
        printf("input with s, E or b error!\n");
    }
    if(!traceFile){
        printf("input with tracefile name error!\n");
    }

}

void printHelp(){
    printf("This is help!\n");
}
