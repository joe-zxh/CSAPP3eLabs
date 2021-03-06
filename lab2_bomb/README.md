# lab2 bomblab
## 题解
Border relations with Canada have never been better.  
1 2 4 8 16 32  
1 311  
7 0 DrEvil  
)/.%&'  
4 3 2 1 6 5  
22  

phase4的"DrEvil"是用来启动 secretePhase的。


## phase1

知道edi存的是用户的输入，esi放的是密文
那么 在callq <strings_not_equal>前面打个断点：break * 0x400ee9
然后，查看esi存的地址值：print $esi  结果为: 4203520
再查看该地址值存的字符串：x/s (4203520)
得出：Border relations with Canada have never been better.

## phase2
拿到用户输入的6个int，并放入 (rsp) (rsp+4) (rsp+8) (rsp+12) (rsp+16) (rsp+20)
通过读代码，发现要求是 首项为1，q为2的等比数列。即：1 2 4 8 16 32

## phase3
在调用sscanf之前(0x400f5b)打个断点
查看一下 esi的值(sscanf的第二个参数):print $esi 结果为：4203983
它 说明了输入的格式。通过 x/s (4203983)查看 字符串，得到："%d %d"
即输入为2个整数。结果会放到rsp+8和rsp+12的位置 eax存的是获取到的输入个数，必须为2

0x400f75处的代码，是一个间接跳转，以mem(0x402470+%rax*8))为跳转目标。
如果第一个数是1，那么跳转到400fb9，得出第二个数为0x137(311)时，即可。


## phase4
和phase3类似，也需要用户输入2个整数。

0x401051要求 第二个输入的值为0

0x40102e要求 第一个输入的值<=14

并要求func4(用户输入1, 0, 14)的返回值为0.

因为func4含有迭代调用，我把func4写成c++的代码(phase4.cpp)，遍历第一个输入的值来求解。

## phase5

可以用C++代码近似地复原这个phase5.

1. 这个phase5比phase6友好一点，它的寄存器的使用都是专用的 寄存器，要么存 数值，要么存 指针值，
不会 既存数值，又存指针值。
所以 我用 char *rrr = new char[8];来表示寄存器。（8个字节，既可以用来存数值，又可以用来存指针）

而比较特殊的rsp给它分配了32个字节：char *rsp = new char[32];

2. phase5中只有0x401099的地方 索引了程序运行时，放在内存里面的值，通过gdb工具可以查看到位置
0x4024b0~0x4024fa中存放的是：
maduiersnfotvbylSo you think you can stop the bomb with ctrl-c, do you?

我直接在代码里面放了这段字符串 来模拟索引。

3. 在0x4010bd之前 是一段疯狂对 输入的字符串的一顿扫操作，我没管，直接用C++的模拟代码来跑了，
然后再在这个分水岭的地方，检查被疯狂操作之后的 字符串的值 和 需求值之间的 差别即可。
遍历找到 需要的输入的字符串。

具体可以看phase5.cpp的代码。


## phase6:

骚操作很多，大部分都能 转成C++代码：
1. 因为 有的寄存器 进行了混用，既用来 存值，又用来 存地址值。所以我写了一个RegisterJoe的类来操作。
里面有64位的long int参数v64，也有32位的int参数i32用于debug，有一个指针p，一个bool值isValue
制定改寄存器存的是值 还是 地址值。

把一系列操作封装起来方便操作。

2. 有些地方汇编代码中 还是直接索引 地址值了，我把需要用到的地址值 放到一个名为simulatedMemory的map中，
用来模拟运行时的内存值。（通过gdb在运行时获取的具体值）


代码的具体内容：
0. 输入了6个数字。
1. 检查每个数都<=6,且 各不相同。(phase6.cpp 207行附近)
2. 对每个输入的数字x，用7-x来替代原来的值。(phase6.cpp 219~234行附近)
3. 通过查看simulatedMemory时发现每个输入的数字都必须大于0，不然 索引到程序之外的内存了。
4. （最难读的部分）标号gt4011bd附近的循环操作（一段很长的操作），会使得 那段模拟的内存 有一个这样的重要性质：
simulatedMemory(value(rsp32)+8)=value(rsp40)
rsp32指的是rsp+32开始的某个long值，rsp40是该值的下一个long的值。
5. phase6.cpp 306行附近有点玄学，我没具体看功能了，暴力就完事了（6^7次遍历）。


## secrete_phase

有了phase6的代码基础，通过暴力转C++代码的形式解出 结果为22或20.

1. 先读汇编代码，发现secret_phase的入口在 phase_defused函数中(401630)
每次解完一个炸弹之后，都会调用phase_defused函数，但只有在phase_6完成之后，才能进入secret_phase
(4015d8 4015df)
2. (4015f5)查看 sscanf的输入，发现 竟然是phase_4时的输入，猜测 这里就是 secret_phase的输入。
(4015f0)发现sscanf的格式是 "%d %d %s"，最后的那个字符串 就是 secret_phase的输入。
(401604)可以查看到 要对比的密文是 DrEvil，所以 phase_4的输入除了 7 0，还需要输入DrEvil
然后 进入secret_phase

3. 观察(401242~401260)，可以发现 要输入的是一个 长整数，且值<=1001。

4. 后面的过程 就是转换成C++代码的过程(secrete_phase.cpp):
    但这里 有3个地方需要用到内存索引，和phase6类似，这里 采用模拟一个运行时内存的方法，
    需要那些内存，直接暴力硬编码到这个模拟内存里。别的过程和phase6类似。

    最后 暴力遍历1~1001, 找到最终的 答案为 20 或 22


最后结果：
<img src="result.png">  