# lab4_cachelab

## Part A: 写一个cache模拟器

cache的结构如图：  

<img src="mdpics/cache.png" height="500" width="580">  


cache就是把 主存中2^64(实际可用的只有2^48)个地址 map到一个E×2^s*2^b个地址的cache中。

当需要索引地址A时，先判断其是否在cache中：
1. 提取A的组索引位 和 标记位
2. 通过组索引位 找到A所对应的组(set)
3. 通过标记位，在该组中 看 能否找到对应标记位的 block，如果能，那么A在cache中；否则，A需要从主存中 加载到cache中。  
这个找的过程是 通过遍历该组中的所有block来 实现的(实际的硬件应该已经设计了相应的机制，所以实际应该O(1)就能判断A是否在cache中)。

PartA中的cache模拟器中，E、s、b都不确定。主要思路是 运行时，动态地分配2^s*2^b个block



相对于PartB来说，PartA比较简单，只需要注意以下几点即可：

## partB




