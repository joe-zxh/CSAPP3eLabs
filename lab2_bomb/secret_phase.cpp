#include<iostream>
#include <string.h>
#include <map>

using namespace std;


map<long,long> simulatedMemory;

void init(){//brutal force 暴力列举所有"需要"的内存值。（通过gdb来得到的）
    simulatedMemory[6303984]=36;

    simulatedMemory[6303992]=6304016;
    simulatedMemory[6304000]=6304048;
    simulatedMemory[6304008]=0;
    simulatedMemory[6304016]=8;

    simulatedMemory[6304024]=6304144;
    simulatedMemory[6304032]=6304080;
    simulatedMemory[6304040]=0;
    simulatedMemory[6304048]=50;
    
    simulatedMemory[6304056]=6304112;
    simulatedMemory[6304064]=6304176;
    simulatedMemory[6304072]=0;
    simulatedMemory[6304080]=22;
    
    simulatedMemory[6304088]=6304368;
    simulatedMemory[6304096]=6304304;
    simulatedMemory[6304104]=0;
    simulatedMemory[6304112]=45;

    simulatedMemory[6304120]=6304208;
    simulatedMemory[6304128]=6304400;
    simulatedMemory[6304136]=0;
    simulatedMemory[6304144]=6;

    simulatedMemory[6304152]=6304240;
    simulatedMemory[6304160]=6304336;
    simulatedMemory[6304168]=0;
    simulatedMemory[6304176]=107;

    simulatedMemory[6304184]=6304272;
    simulatedMemory[6304192]=6304432;
    simulatedMemory[6304200]=0;
    simulatedMemory[6304208]=40;

    simulatedMemory[6304432]=1001;  
    simulatedMemory[6304440]=0;    
    simulatedMemory[6304400]=47; 
    
    simulatedMemory[6304240]=1; 
    simulatedMemory[6304248]=0;
    simulatedMemory[6304256]=0;

    simulatedMemory[6304336]=7;
    simulatedMemory[6304352]=0;
    simulatedMemory[6304368]=20;
    simulatedMemory[6304384]=0;

    simulatedMemory[6304304]=35;
    simulatedMemory[6304320]=0;

    simulatedMemory[6304216]=0;
    simulatedMemory[6304224]=0;
    simulatedMemory[6304408]=0;
    simulatedMemory[6304416]=0;
    simulatedMemory[6304272]=99;
    simulatedMemory[6304288]=0;  

    simulatedMemory[6304376]=0; 
    simulatedMemory[6304312]=0; 
    simulatedMemory[6304280]=0;       
}

void bomb(){
  //cout<<"bomb！！！"<<endl;
}

class RegisterJoe{

private:
    long v64;//64位的值
    int i32;//32位的值是用来debug的。

    long *p;//指针。8个字节。指向一个long值

    bool isValue;//当前是指针 还是 值。

public:
    RegisterJoe(long value64=-1, long *pointer=NULL, bool isV=true){
        if(isV){
            this->isValue = true;
            this->v64 = value64;
            seti32();
            this->p= nullptr;
        }else{
            this->isValue = false;
            this->v64 = -1;
            seti32();
            this->p = pointer;
        }
    }

    long getv64(){
        if(!this->isValue){
            cout<<"error!!! 当前存的是 地址，但 希望 获取 其值"<<endl;
        }
        return v64;
    }

    void setv64(int a){
        if(!this->isValue){
            //cout<<"waring!!! 当前存的是 地址，现在 变为 存储 值"<<endl;
            this->isValue = true;
            this->p = NULL;
        }
        this->v64 = a;
        seti32();
    }

    void setv64(long a){
        if(!this->isValue){
            //cout<<"waring!!! 当前存的是 地址，现在 变为 存储 值"<<endl;
            this->isValue = true;
            this->p = NULL;
        }
        this->v64 = a;
        seti32();
    }

    int geti32(){
        return *((int *)&v64);
    }

    void seti32(){
        this->i32 = *((int *)&v64);
    }

    void setPointer(long *pointer){
        if(this->isValue){
            //cout<<"waring!!! 从存储值，变为 存储 地址"<<endl;
            this->isValue = false;
            setv64(-1);
        }
        this->isValue = false;
        this->p = (long *)pointer;
    }

    void setPointer(char *pointer){
        if(this->isValue){
            //cout<<"waring!!! 从存储值，变为 存储 地址"<<endl;
            this->isValue = false;
            setv64(-1);
        }
        this->isValue = false;
        this->p = (long *)pointer;
    }

    long* getPointer(){
        if(this->isValue){
            //cout<<"waring!!! 当前存的是值，无法返回 地址"<<endl;
            return NULL;
        }
        return this->p;
    }

    long getv64FromPointer(){
        if(this->isValue){
            //cout<<"waring!!! 当前存的是值，无法使用 地址"<<endl;
            return -1;        
        }
        return *this->p;
    }

    long geti32FromPointer(){
        if(this->isValue){
            cout<<"error!!! 当前存的是值，无法使用 地址"<<endl;
            return -1;        
        }
        return *(int*)this->p;
    }

    void setv64FromPointer(long a){
        if(this->isValue){
            cout<<"error!!! 当前存的是值，无法使用 地址"<<endl;
        }
        *this->p=a;
    }

    void seti32FromPointer(long a){
        if(this->isValue){
            cout<<"error!!! 当前存的是值，无法使用 地址"<<endl;
        }
        *(int*)this->p=a;
    }

};

long fun7(RegisterJoe& rdi, RegisterJoe& rsi){//rdi一开始是0x6030f0，rsi一开始 是用户的输入
    RegisterJoe rdx;//存的是值
    RegisterJoe rax;//存的是值
    
    if(rdi.getv64()==0){//401208 40120b
        return -1;//401238
    }else{
        if(simulatedMemory.find(rdi.getv64())==simulatedMemory.end()){
            cout<<"内存越界"<<rdi.getv64()<<endl;
        }
        rdx.setv64(int(simulatedMemory[rdi.getv64()]));//40120d  mov (%rdi),%edx

        if(rdx.geti32()<=rsi.geti32()){//40120f 401211
            rax.setv64(0);//401220
            if(rdx.geti32()==rsi.geti32()){//401225 401227
                return rax.getv64();
            }else{
                if(simulatedMemory.find(rdi.getv64()+16)==simulatedMemory.end()){
                   cout<<"内存越界"<<rdi.getv64()+16<<endl;
                }
                rdi.setv64(simulatedMemory[rdi.getv64()+16]);//401229
                rax.setv64(fun7(rdi, rsi));//40122d
                
                rax.setv64(int(rax.getv64()+rax.getv64()+1));//401232
                return rax.getv64();//401236 jmp to 40123d
            }
        }else{  
            if(simulatedMemory.find(rdi.getv64()+8)==simulatedMemory.end()){
                cout<<"内存越界"<<rdi.getv64()+8<<endl;
            }          
            rdi.setv64(simulatedMemory[rdi.getv64()+8]);//401213
            rax.setv64(fun7(rdi, rsi));//401217
            rax.setv64(rax.geti32()+rax.geti32());//40121c
            return rax.getv64();//40121e jmp to 40123d
        }
    }
}

int secret_phase(long input){ //40125a之前可以知道，输入的是一个长整数

    RegisterJoe rsi(0, NULL, true);//40125a rsi 是指向值的。

    RegisterJoe rax(input, NULL, true);//401255 rax一开始 是指向值的。
    RegisterJoe rbx(input, NULL, true);//40125a rbx一开始 是指向值的。
    RegisterJoe rdi;

    rax.setv64(int(rax.getv64()-1));//40125d lea -0x1(%rax),%eax

    if(rax.geti32()<=0x3e8){//401260 401265  用户输入要<=1001      
        rsi.setv64(rbx.geti32());//40126c rsi是fun7 第2个参数输入。也就是用户的输入值
        rdi.setv64(0x6030f0);//40126e '$'字符的位置  rdi是fun7的第1个参数的值

        return fun7(rdi, rsi);//返回值需要是2才行
    }else{
        bomb();
    }  
}

int main()
{
    init();

    for(int i = 0;i<=1001;i++){
        if(secret_phase(i)==2){
            cout<<i<<endl;
        }
    }
    //答案为20或22
    
    return 0;
}
