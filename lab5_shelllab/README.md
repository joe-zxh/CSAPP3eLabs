# lab5 Shell Lab
这个lab不是很难。  
难点在于处理 fg命令时， 将一个background的正在运行的工作 转换成 foreground的工作的操作：可能在转换完成之前，这个background的工作就跑完了，所以要：
1. 先block掉child信号。
2. 判断这个后台的工作是否还在运行，如果已经结束，那么直接unblock之前的信号就可以了。  
    否则，就通过一个循环等待这个进程完成：  
    while(!pid){//等这个foreground job跑完  
        sigsuspend(&prev);//一个原子操作，具体看书上P545  
    }

查看signal相关的各种函数的api：http://pubs.opengroup.org/onlinepubs/7908799/xsh/signal.h.html