#include<iostream>
using namespace std;

int func4(int *x, int *y, int *z){
    //x在edi中，是用户输入的，在前面的要求中，它需要<=14
    //y在esi中
    //z在edx中

    int t0 = *z;//t0是eax  400fce

    t0 -= *y;//400fd4

    unsigned int t1 = t0;//t1是ecx  400fd6

    t1>>=31;//左边填0 逻辑移位

    t0 += t1;

    t0>>=1;//左边填符号位 算数移位

    t1 = t0 +*y;

    if(t1<=*x){
        t0=0;
        if(t1>=*x){
            return t0;
        }else{
            *y = t1+1;
            func4(x,y,z);
            t0=t0+t0+1;
            return t0;
        }
    }else{
        *z=t1-1;//400fe6
        func4(x,y,z);//400fe9
        t0+=t0;//400fee
        return t0;//
    }

    //如果返回时t0=0，那么就是正确的输出。
}

int main()
{
  int x = 0;
  int y = 0;
  int z = 14;

  for (int i=0;i<=14;i++){
    x = i;
    y = 0;
    z = 14;
    cout<<x<<"  "<<func4(&x,&y,&z)<<endl;
  }

  return 0;
}