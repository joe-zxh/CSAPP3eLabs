#include<iostream>
#include <string.h>

using namespace std;

void bomb(){
  cout<<"bomb！！！"<<endl;
}

bool checkStr(char *input, char *secret){//判断输入 和 密文之间是否相同。
    int lenInput = strlen(input);
    int lenSecret = strlen(secret);

    if(lenInput!=lenSecret){
        return false;
    }

    for(int i = 0;i<lenSecret;i++){
        if(input[i]!=secret[i]){
            return false;
        }       
    }

    return true;
}

char secreteCode[] = "maduiersnfotvbylSo you think you can stop the bomb with ctrl-c, do you?";

//注意大小端的问题，左边的元素是小的。
//rsp数组 靠左边的元素是低位。而别的8字节的数组，靠右边的是低位
bool phase5(char *rdi, int n){//8个字节的内容，但只有5个字节的字符串
    // rdi = new char[8]; 记得要delete
    // 寄存器 %rdi 的值是一个指针(而不是一个整数)，即
    // %rdi = rdi;
    // (%rdi) = *(long int*)rdi

    //n表示现在搜的是第n个字符是否正确，如果n=-1，表示搜的是整个字符串

    bool retB = false;

    char tmpch;
    long int tmpint;
    
    char *rsp = new char[32];//401063 一开始就分配了 32个字节
    // 寄存器 %rsp的值是一个指针，即：
    // %rsp = rsp
    // (%rsp) = *(long int*)rsp

    char *rbx = new char[8];//401067
    // 寄存器 %rbx的值是一个指针，即：
    // %rbx = rbx;
    // (%rbx) = *(long int*)rbx

    rbx=rdi;//401067

    char *rax=new char[8];
    // 寄存器 %rax的值 是一个数，即
    // %rax = *(long int*)rax

    char *rcx=new char[8];//先定义在40108b出现的 ecx  存的是数
    // %rcx = *(long int*)rcx

    char *rdx=new char[8];//先定义在401092出现的 rdx  存的是 数
    // %rdx = *(long int*)rdx 
    
    char *rsi=new char[8];//先定义在4010b3出现的 rsi  存的是 地址
    // %rsi = rsi
    // (%rsi) = *(long int*)rsi

    //40106a位置处 放了一个金丝雀的值，即fs:0x28位置上的值。应该是随机的。不用管
    strcpy(rax, "canary!");//40106a 

    *(long int*)(rsp+24) = *(long int*)rax;
    //401073: rsp+24字节的位置存放 金丝雀的值，最后再检查一下溢出情况。


    *(long int*)rax = 0;//401078 #清空一下rax

    *(int *)(rax) = strlen(rdi);//eax记录一下输入的字符串的长度 40107a

    if (*(int *)(rax)==6){ //要求输入的字符串长度为6，否则爆炸

        //4010d2 #清空一下rax
        *((long int*)rax) = 0;


    gt40108b:
        //40108b  注意rbx是指针，而rax是值，加完是一个指针，再取值，的出来就是一个值
        *(int*)(rcx) = int(*(char*)(rbx+(*(long int*)rax)));               

        tmpint = *(long int*)(rcx);

        //40108f
        *rsp = *(rcx);

        //401092
        *(long int*)rdx = *(long int*)rsp;

        tmpint = *(long int*)rdx;

        //401096
        *(long int*)rdx = *(int*)(rdx) & 0xf;

        tmpint = *(long int*)rdx;

        //0x4024b0~0x4024fa
        //maduiersnfotvbylSo you think you can stop the bomb with ctrl-c, do you?

        char *p = secreteCode;

        //401099
        *((int *)(rdx)) = (int)*(p+*(long int*)rdx);

        tmpint = *((int *)(rdx));

        //4010a0        
        *(rsp+*(long int*)rax+0x10)=*(rdx);

        //4010a4         
        *(long int*)rax += 1;
        

        if(*(long int*)rax==6){//继续执行
            
            //4010ae
            *(rsp+0x16)=char(0);

            //4010b3
            strcpy(rsi, "flyers");//地址0x40245e里面的字符串 就是这个

            //4010b8
            rdi =rsp+0x10;

            if(n==-1){
                retB =checkStr(rdi, rsi); //比较整个字符串（对比正确答案时用）
            }else{
                return (rsi[n]==rdi[n]); //比较第n个字符（遍历时用）
            }                  

        }else{//跳转到40108b
            goto gt40108b;
        }     

    } else {
        bomb();
    }

    delete[] rsp;
    //delete[] rbx; 因为它指向的是rsp的内容，不必 重复delete了。
    delete[] rax;
    delete[] rcx;
    delete[] rdx;
    delete[] rsi;

    return retB;   
}

int main()
{
    char input[8]="heyhey";//随便一个初始值

    int left = 32;//设置遍历的左右边界
    int right = 126;
  
    for(int i = 0;i<=5;i++){
        for(int j=left;j<right;j++){
            input[i]=j;
            if(phase5(input,i)){
                break;
            }
        }
    }
    cout<<input[0]<<endl;
    cout<<input[1]<<endl;
    cout<<input[2]<<endl;
    cout<<input[3]<<endl;
    cout<<input[4]<<endl;
    cout<<input[5]<<endl;

    // 结果为：   )/.%&'
    // input[0]=41;
    // input[1]=47;
    // input[2]=46;
    // input[3]=37;
    // input[4]=38;
    // input[5]=39;

  return 0;
}
