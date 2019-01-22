#include<iostream>
#include <string.h>
#include <map>

using namespace std;

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

void print32to80(char *pch, map<long, long>mm){
    long *p = (long *)pch;
    cout<<hex;
    cout<<"32: "<<*p<<"\t40: "<<*(p+1)<<"\t48: "<<*(p+2);
    cout<<"\t56: "<<*(p+3)<<"\t64: "<<*(p+4)<<"\t72: "<<*(p+5)<<endl;

    cout<<"32: "<<mm[*p+8]<<"\t40: "<<mm[*(p+1)+8]<<"\t48: "<<mm[*(p+2)+8];
    cout<<"\t56: "<<mm[*(p+3)+8]<<"\t64: "<<mm[*(p+4)+8]<<"\t72: "<<mm[*(p+5)+8]<<endl<<endl;
}

//注意大小端的问题，左边的元素是小的
bool phase6(int a, int b, int c, int d, int e, int f){

    //0x6032d0
    long address[5]={0x6032e0,0x6032f0,0x603300,0x603310,0x603320};
    //输入的x必须， x<=6 && x>=1。x<1的话，索引不到地址了(0x0)

    map<long,long> simulatedMemory;//模拟那些不能访问的内存  key是位置，value是内容 
    //下面是通过debug工具获取的内容
    simulatedMemory[0x6032d0]=0x10000014c;//x=6时
    simulatedMemory[0x6032e0]=0x2000000a8;//x=5时
    simulatedMemory[0x6032f0]=0x30000039c;//x=4时
    simulatedMemory[0x603300]=0x4000002b3;//x=3时
    simulatedMemory[0x603310]=0x5000001dd;//x=2时
    simulatedMemory[0x603320]=0x6000001bb;//x=1时

    simulatedMemory[0x6032d8]=0x6032e0;
    simulatedMemory[0x6032e8]=0x6032f0;
    simulatedMemory[0x6032f8]=0x603300;
    simulatedMemory[0x603308]=0x603310;
    simulatedMemory[0x603318]=0x603320;
    simulatedMemory[0x603328]=0x0;  

    long tmp;
    
    RegisterJoe rbx,rcx,rdx,r12,rax,rsi;
    RegisterJoe r13(-1,NULL,false), r14(-1,NULL,false), rbp(-1,NULL,false);

    //rbx, rcx, rdx, r12是数
    //r13, r14, rbp是指针
    // rax 和出现了 rsi 既是数，又是指针的情况。

    

    //一共输入6个数字
    char *rsp = new char[80];//4010fc 一开始就分配了 80个字节
    // 寄存器 %rsp 的值是一个指针，即：
    // %rsp = rsp
    // (%rsp) = *(long int*)rsp

    
    r13.setPointer(rsp);//401100        
    rsi.setPointer(rsp);//401103

    //401106 的 read_six_numbers的效果
    *(int*)rsp = a;
    *(int*)(rsp+4) = b;
    *(int*)(rsp+8) = c;
    *(int*)(rsp+12) = d;
    *(int*)(rsp+16) = e;
    *(int*)(rsp+20) = f;
    
    r14.setPointer(rsp);//40110b
    
    r12.setv64(0);//40110e

    gt401114:    
    rbp.setPointer(r13.getPointer());//401114    
    rax.setv64(r13.geti32FromPointer());//401117
    
    rax.setv64(rax.geti32()-1);//40111b //第一个数的值-1

    //joe: 这上面的代码暂时没有问题

    //40111e 401121
    if(rax.geti32()<=0x5){//第一个数的值-1<=5才行。所有数都要<=6,且每个数各不相等
        
        r12.setv64(r12.geti32()+1);//401128 //r12=1
        
        if(r12.geti32()==6){//40112c 401130
            rsi.setPointer(rsp+0x18); //401153 //末尾

            //401158  mov %r14,%rax  !!!小心，rax这样赋值之后，就变成指针了
            rax.setPointer(r14.getPointer());
            
            rcx.setv64(7);//40115b

            gt401160:            
            rdx.setv64(rcx.geti32());//401160

            //401162
            rdx.setv64(rdx.geti32()-rax.geti32FromPointer());//7-第一个数的值

            //401164
            rax.seti32FromPointer(rdx.geti32());//第一个数的值=7-第一个数

            //401166
            rax.setPointer(((char*)rax.getPointer())+4);//注意rax还是指针  现在指向第二个数

            //40116a 40116d
            if (rax.getPointer()!=rsi.getPointer()){ //还没遍历完                
                goto gt401160;//401160  //安全范围，rax仍是指针没改变
            } else{//现在a=7-a b=7-b c=7-c d=7-d e=7-e f=7-f                
                rsi.setv64(0);//40116f //注意rsi变成了数

                //401174 jump到...401197 注意rsi是数
                rcx.setv64(*(int *)(rsp+ rsi.getv64()));//7-a
                
                if(rcx.geti32()<=1){//40119a 40119d //遍历7-a 7-b 7-c 7-d 7-e 7-f <=1时的情况
                 
                    gt401183://401183                    
                    rdx.setv64(0x6032d0);//看上去像是个地址

                    gt401188://401188 注意rsi还是数
                    *(long *)(rsp+ rsi.getv64()*2+32) = rdx.getv64();
                    //从rsp+32 rsp+40 rsp+48 rsp+56 rsp+64 rsp+72 rsp+80,当: 
                    
                    //  x=6存的是 0x6032d0; x=5存的是 0x6032e0; x=4存的是 0x6032f0;
                    //   x=3存的是 0x603300; x=2存的是 0x603310; x=1存的是 0x603320;                                        
        
                    rsi.setv64(rsi.getv64()+4);//40118d  rsi+=4

                    //401191 401195
                    if(rsi.getv64()==24){//填满了rsp+32~rsp+80的内容了

                        //print32to80(rsp+32,simulatedMemory);
                        
                        rbx.setv64(*(long *)(rsp+32));//4011ab: mov 0x20(%rsp),%rbx 应该没问题
                        //rbx是值,存的是rsp+32的值  rbx相当于指针头
                        
                        rax.setPointer(rsp+40);//4011b0: lea 0x28(%rsp),%rax 
                        //rax还是指针,一开始指向rsp+40的位置  rax相当于*iter指针

                        rsi.setPointer(rsp+80);//4011b5: lea 0x50(%rsp),%rsi
                        //rsi似乎变回了指针，存的是末尾的位置，即rsp+80的位置
                                                
                        rcx.setv64(rbx.getv64());//4011ba //rcx是值，一开始是rsp+32位置上的值

                        gt4011bd:  //4011bd
                        rdx.setv64(rax.getv64FromPointer());//rdx一开始 存的是 rsp+40位置上的值

                        //重要！！！rcx是前一个值，rdx是后一个值，最后出来的结果就是：
                        // mem(value(rsp+偏移)+8)=value(rsp+偏移+8)

                        //4011c0 有问题...!!!
                        simulatedMemory[0x8+rcx.getv64()] = rdx.getv64();
                        
                        rax.setPointer(rax.getPointer()+1);//4011c4
                        
                        //4011c8 4011cb
                        if (rax.getPointer()==rsi.getPointer()){ //遍历完了
                            //print32to80(rsp+32,simulatedMemory);

                            //4011d2 有问题...!!!:
                            simulatedMemory[0x8+rdx.getv64()] = 0;//初始化

                            rbp.setv64(0x5);//4011da //rbp怎么也变成了值... 变成下一个循环的counter，遍历5次

                            gt4011df:
                            //4011df 有问题...!!!:
                            if(simulatedMemory.find(rbx.getv64()+8)==simulatedMemory.end()){
                                return false;
                            }
                            rax.setv64(simulatedMemory[rbx.getv64()+8]);
                            //rax又变回了值

                            //4011e3
                            if(simulatedMemory.find(rax.getv64())==simulatedMemory.end()){
                                return false;
                            }

                            rax.setv64((int)simulatedMemory[rax.getv64()]);//4011e3 注意这里别用int不然不够大

                            //4011e5 4011e7
                            if((int)simulatedMemory[rbx.getv64()]>=rax.geti32()){ 
                                //这段取int的操作有点玄学，我就没看完了，直接暴力遍历即可。

                                if(simulatedMemory.find(rbx.getv64()+8)==simulatedMemory.end()){
                                    return false;
                                }                                
                                rbx.setv64(simulatedMemory[rbx.getv64()+8]);//4011ee
                                                                
                                rbp.setv64(rbp.geti32()-1);//4011f2
                                
                                if(rbp.getv64()!=0){//4011f5                                    
                                    goto gt4011df;//4011df
                                }else{
                                    return true;//成功
                                }

                            }else{
                                bomb();
                                return false;
                            }

                        }else{ //还没遍历完
                            //4011cd
                            rcx.setv64(rdx.getv64());

                            //4011d0 //jmp 4011bd
                            goto gt4011bd;
                        }

                    }else{//还没填满rsp+32~rsp+80

                        rcx.setv64(*(int*)(rsp+ rsi.getv64()));//401197
                        //遍历7-a 7-b 7-c 7-d 7-e 7-f

                        //40119a 40119d
                        if(rcx.geti32()<=1){
                            //jmp 401183
                            goto gt401183;

                        }else{
                            //40119f
                            goto gt40119f;
                        }
                    }               

                }else{//设置rsp+32~rsp+80位置存放的内容
                    gt40119f:                    
                    rax.setv64(1);//40119f //rax又变回数了,用于counter从2开始遍历到6

                    //rdx.setv64(0x6032d0);//4011a4

                    gt401176:
                    rdx.setv64(address[rax.getv64()-1]);
                    //rdx.setv64(simulatedMemory[8+rdx.getv64()]);//4011a9 401176
                    
                    rax.setv64(rax.geti32()+1);//40117a  
                    //rax是从2开始遍历的，如果是1的情况就是上面没进入else的情况 即0x6032d0的情况

                    //40117d 40117f
                    if (rax.geti32()!=rcx.geti32()){//rcx存的是是7-x
                        goto gt401176;
                    }else{//当前rax=7-x了
                        goto gt401188;//rax=7-x  //401181 jmp 401188                      
                    }
                }
            }


        }else{
            //这部分是要判断 每个数和别的数不一样才行。
            //joe: 这下面的部分应该没有问题

            rbx.setv64(r12.getv64()); //401132     

            gt401135:            
            rax.setv64(rbx.getv64()); //401135 movslq %ebx,%rax

            rax.setv64(*(int *)(rsp+4*rax.getv64())); //401138
            //遍历第i个输入的整数(i从1开始的)
            
            if(rbp.geti32FromPointer() != rax.geti32()){//40113b 40113e
                //要和第一个数不相等???  这部分好像说 其他数都要 和 第一个数不一样才行                
                rbx.setv64(rbx.geti32()+1);//401145
                
                if(rbx.getv64()<=5){    //401148 40114b                
                    goto gt401135;//401135 //安全
                }else{  
                    r13.setPointer(4+((char *)r13.getPointer())); //40114d //然后从第二个数开始 
                    goto gt401114; //当每个数都不一样时 跳转
                }

            }else{
                bomb();
                return false;
            }
        }

    }else{//第一个数的值<=6才不会爆炸
        bomb();
        return false;
    }

    return true;
    delete []rsp;   
}

int main()
{
    //读代码可以发现，需要输入6个各不相等的数字，且范围为 [1,6],暴力遍历一下即可：

    for(int a=0;a<=6;a++){
        for(int b=0;b<=6;b++){
            for(int c=0;c<=6;c++){
                for(int d=0;d<=6;d++){
                    for(int e=0;e<=6;e++){
                        for(int f=0;f<=6;f++){
                            if(phase6(a,b,c,d,e,f)){
                                cout<<a<<"\t"<<b<<"\t"<<c<<"\t"<<d<<"\t"<<e<<"\t"<<f<<"\n";
                                //break;
                            }
                        }
                    }
                }
            }
        }
    }
    
    return 0;
}
