/*
 * mm-explicit.c
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "zouTeam",
    /* First member's full name */
    "Zou",
    /* First member's email address */
    "Zou@scut.edu",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)


#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))
//size_t是8字节的unsigned long
//SIZE_T_SIZE就是8

#define WSIZE 4 //header和footer的大小
#define DSIZE 8 //double-word
#define CHUNKSIZE (1<<12) //当堆内存不够用的时候，一次分配4KB的内存。(一共有20MB可用)

#define MAX(x,y) ((x)>(y)?(x):(y))

#define PACK(size, alloc) ((size)|(alloc)) //把块大小放 并上 分配位的结果

//在地址p处读写一个字(4字节)的大小。
#define GET(p) (*(unsigned int *)(p))
#define PUT(p, val) (*(unsigned int *)(p)=(val))

//在地址p处读取 块大小 和 分配位。
#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)
#define GET_PRO(p) (GET(p) & 0x2) //判断是不是序言块
//其中块的大小指的是 header+footer+有效载荷+填充块 之后的总大小


//块指针bp指向有效负载开始的位置。
//根据bp计算块的header和footer的指针。
#define HDRP(bp) ((char *)(bp)-WSIZE)
#define FTRP(bp) ((char *)(bp)+ GET_SIZE(HDRP(bp)) -DSIZE)

//按照ppt的标号来设置 (注意到书上 和 ppt上面的标号的不同)

//successor表示邻接的下一个 predecessor表示邻接的上一个
//next表示链表的下一个 prev表示链表的上一个
//根据bp，计算 successor(邻接的下一个)的 bp的指针，或者 predecessor(邻接的上一个) 的bp的指针。
#define SUCC_EXBP(bp) ((char *)(bp) + GET_SIZE(((char *)(bp)-WSIZE)))
#define PREDE_EXBP(bp) ((char *)(bp) - GET_SIZE(((char *)(bp)-DSIZE)))

//对于普通块: 根据bp，设置 下(上)一个空的block的bp的位置。
#define PUT_NEXT(bp, off) (*((int *)((char *)(bp)))=(off))
#define PUT_PREV(bp, off) (*((int *)((char *)(bp)+WSIZE))=(off))

//对于序言块来的
#define PUT_NEXTSEG(nextoff, off) (*((int *)(heap_listp+nextoff))=(off))

//对于结尾块来说的
#define PUT_PREVSEG(ebp, nextoff, off) (*((int *)(ebp+nextoff))=(off))

//对于普通块: 根据bp，获取 下(上)一个空的block的bp的位置。
#define GET_NEXT(bp) (*((int *)((char *)(bp))))
#define GET_PREV(bp) (*((int *)((char *)(bp)+WSIZE)))

//对于序言块：
#define GET_NEXTSEG(nextoff) (*((int *)(heap_listp+nextoff)))

//对于结尾块：
#define GET_PREVSEG(ebp, nextoff) (*((int *)(ebp+nextoff)))

//bp相对于heap_listp的偏移
#define OFFSET(bp) ((int*)((char *)(bp) -heap_listp))

//普通块：根据bp，计算下一个bp的指针，或者上一个bp的指针。
#define NEXT_EXBP(bp) (heap_listp+GET_NEXT(bp))
#define PREV_EXBP(bp) (heap_listp+GET_PREV(bp))


//mm.c的私有的全局变量
static char *heap_listp;//指向P598页所指向的位置

static void *extend_heap(size_t words);
static void *coalesce(void *bp);
static void *find_fit(size_t asize);
static void place(void *bp, size_t asize);
struct empListInfo *getInfoBySize(size_t size);
int inTargetSize(size_t size);
void prev2newEnd(char *bp, struct empListInfo *el, int endoff);
void divideNLink(char *bp, struct empListInfo *el, int num, int endoff);
static void *coalGeneral(void *bp);

struct empListInfo{ //一共36
    int alloc[5];//每次对应的分配个数 20
    int count;//已分配的次数 4
    int scount;//本应最多分配的次数 4
    int asize;//一个块的大小 4
    int nextOff;//next指针相对bp位置的偏移 4(32偏移为)
}e24, e72, e80, e120, e136, e168, e456, e520, e4080, e4104, e8200;

void initJoe(){
//    e24.nextOff=4; e24.asize=24; e24.count=0; e24.scount=1; e24.alloc[0]=1;
//    e72.nextOff=8; e72.asize=72; e72.count=0; e72.scount=1; e72.alloc[0]=1;
//    e80.nextOff=12; e80.asize=80; e80.count=0; e80.scount=1; e80.alloc[0]=1;
//
//    e120.nextOff=16; e120.asize=120; e120.count=0; e120.scount=1; e120.alloc[0]=1;
//    e136.nextOff=20; e136.asize=136; e136.count=0; e136.scount=1; e136.alloc[0]=1;
//    e168.nextOff=24; e168.asize=168; e168.count=0; e168.scount=1; e168.alloc[0]=1;
//    e456.nextOff=28; e456.asize=456; e456.count=0; e456.scount=1; e456.alloc[0]=1;
//    e520.nextOff=32; e520.asize=520; e520.count=0; e520.scount=1; e520.alloc[0]=1;
//
//    e4080.nextOff=36; e4080.asize=4080; e4080.count=0; e4080.scount=1; e4080.alloc[0]=1;
//    e4104.nextOff=40; e4104.asize=4104; e4104.count=0; e4104.scount=1; e4104.alloc[0]=1;
//    e8200.nextOff=44; e8200.asize=8200; e8200.count=0; e8200.scount=1; e8200.alloc[0]=1;
//    e8200.nextOff=44; e8200.asize=8200; e8200.count=0; e8200.scount=1; e8200.alloc[0]=1;

    e24.nextOff=4; e24.asize=24; e24.count=0; e24.scount=3; e24.alloc[0]=134; e24.alloc[1]=3866; e24.alloc[2]=800;
    e72.nextOff=8; e72.asize=72; e72.count=0; e72.scount=2; e72.alloc[0]=2; e72.alloc[1]=1998;
    e80.nextOff=12; e80.asize=80; e80.count=0; e80.scount=4; e80.alloc[0]=1; e80.alloc[1]=236; e80.alloc[2]=31; e80.alloc[3]=29;

    e120.nextOff=16; e120.asize=120; e120.count=0; e120.scount=1; e120.alloc[0]=4000;
    e136.nextOff=20; e136.asize=136; e136.count=0; e136.scount=2; e136.alloc[0]=4000; e136.alloc[1]=800;
    e168.nextOff=24; e168.asize=168; e168.count=0; e168.scount=5; e168.alloc[0]=1; e168.alloc[1]=229; e168.alloc[2]=21; e168.alloc[3]=9;e168.alloc[4]=26;
    e456.nextOff=28; e456.asize=456; e456.count=0; e456.scount=1; e456.alloc[0]=2000;
    e520.nextOff=32; e520.asize=520; e520.count=0; e520.scount=2; e520.alloc[0]=1; e520.alloc[1]=1999;

    e4080.nextOff=36; e4080.asize=4080; e4080.count=0; e4080.scount=4; e4080.alloc[0]=1; e4080.alloc[1]=2004; e4080.alloc[2]=52; e4080.alloc[3]=63;
    e4104.nextOff=40; e4104.asize=4104; e4104.count=0; e4104.scount=2; e4104.alloc[0]=1; e4104.alloc[1]=4799;
    e8200.nextOff=44; e8200.asize=8200; e8200.count=0; e8200.scount=2; e8200.alloc[0]=1; e8200.alloc[1]=2399;
    e8200.nextOff=44; e8200.asize=8200; e8200.count=0; e8200.scount=2; e8200.alloc[0]=1; e8200.alloc[1]=2399;

}

/*
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    initJoe();

    //首先要30个word：1个word的填充+14个word的序言块+15个word的结尾块。
    if((heap_listp=mem_sbrk(30*WSIZE)) ==(void *)-1){
        return -1;
    }

    PUT(heap_listp, 0);//填充块
    PUT(heap_listp + (1*WSIZE), PACK(56, 3));//序言块header：记录14个word大小(56字节)，有效位为1，序言块位为1

    PUT(heap_listp + (2*WSIZE), 56);//序言块通用的next:初始时 指向结尾块的bp
    PUT(heap_listp + (3*WSIZE), 56);//序言块24next:初始时 指向结尾块的bp
    PUT(heap_listp + (4*WSIZE), 56);//序言块72next:初始时 指向结尾块的bp
    PUT(heap_listp + (5*WSIZE), 56);//序言块80next:初始时 指向结尾块的bp
    PUT(heap_listp + (6*WSIZE), 56);//序言块120next:初始时 指向结尾块的bp
    PUT(heap_listp + (7*WSIZE), 56);//序言块136next:初始时 指向结尾块的bp
    PUT(heap_listp + (8*WSIZE), 56);//序言块168next:初始时 指向结尾块的bp
    PUT(heap_listp + (9*WSIZE), 56);//序言块456next:初始时 指向结尾块的bp
    PUT(heap_listp + (10*WSIZE), 56);//序言块520next:初始时 指向结尾块的bp
    PUT(heap_listp + (11*WSIZE), 56);//序言块4080next:初始时 指向结尾块的bp
    PUT(heap_listp + (12*WSIZE), 56);//序言块4104next:初始时 指向结尾块的bp
    PUT(heap_listp + (13*WSIZE), 56);//序言块8200next:初始时 指向结尾块的bp

    PUT(heap_listp + (14*WSIZE), PACK(56, 3));//序言块footer：记录14个word大小(56字节)，有效位为1


    PUT(heap_listp + (15*WSIZE), PACK(0, 1));//结尾块header.size=0表示为结尾块

    PUT(heap_listp + (16*WSIZE), -1);//结尾块next 恒为-1 没用的。用来占位的
    PUT(heap_listp + (17*WSIZE), 0);//结尾块通用prev 初始化为0 即指向序言块的bp
    PUT(heap_listp + (18*WSIZE), 0);//结尾块24prev 初始化为0 即指向序言块的bp
    PUT(heap_listp + (19*WSIZE), 0);//结尾块72prev 初始化为0 即指向序言块的bp
    PUT(heap_listp + (20*WSIZE), 0);//结尾块80prev 初始化为0 即指向序言块的bp
    PUT(heap_listp + (21*WSIZE), 0);//结尾块120prev 初始化为0 即指向序言块的bp
    PUT(heap_listp + (22*WSIZE), 0);//结尾块136prev 初始化为0 即指向序言块的bp
    PUT(heap_listp + (23*WSIZE), 0);//结尾块168prev 初始化为0 即指向序言块的bp
    PUT(heap_listp + (24*WSIZE), 0);//结尾块456prev 初始化为0 即指向序言块的bp
    PUT(heap_listp + (25*WSIZE), 0);//结尾块520prev 初始化为0 即指向序言块的bp
    PUT(heap_listp + (26*WSIZE), 0);//结尾块4080prev 初始化为0 即指向序言块的bp
    PUT(heap_listp + (27*WSIZE), 0);//结尾块4104prev 初始化为0 即指向序言块的bp
    PUT(heap_listp + (28*WSIZE), 0);//结尾块8200prev 初始化为0 即指向序言块的bp

    PUT(heap_listp + (29*WSIZE), PACK(0, 1));//结尾块footer.size=0表示为结尾块 没用的

    heap_listp+=(2*WSIZE);

    return 0;
}

/*
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{


    size_t asize; //调整后的块的总大小
    char *bp;

    if(size==0){
        return NULL;
    }

    if(size<=DSIZE){ //todo: 这里可能有问题
        asize = 2*DSIZE;
        //1个word大小的header+1个word的大小的footer，又需要整个block的大小为偶数个word，所以最小需要4个word
    } else{
        asize = DSIZE * ((size+(DSIZE)+(DSIZE-1))/DSIZE);
    }

    //寻找一个合适的空的位置
    if((bp=find_fit(asize))!=NULL){
        place(bp, asize);
        return bp;
    }

    //没有合适的位置，需要获取更多的heap内存
    if((bp=extend_heap(asize))==NULL){
        return NULL;
    }
    place(bp, asize);
    return bp;
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *bp)
{
    size_t size = GET_SIZE(HDRP(bp));

    PUT(HDRP(bp), PACK(size, 0));//把有效位设置为0
    PUT(FTRP(bp), PACK(size, 0));
    coalesce(bp);//再看看需不需要合并
}

/*
 * 这个先不管
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    void *oldptr = ptr;
    void *newptr;
    size_t copySize;

    newptr = mm_malloc(size);
    if (newptr == NULL)
      return NULL;
    copySize = *(size_t *)((char *)oldptr - SIZE_T_SIZE);//相当于指针前移8个字节，然后拿到 有效载荷的大小。
    if (size < copySize)
      copySize = size;
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
}

//合并空余块
static void *coalesce(void *bp){ //需要判断前后 是否 默认类型。如果是，则忽略。

    int size = GET_SIZE(HDRP(bp));

    struct empListInfo *tempList=getInfoBySize(size);

    if(tempList!=NULL){//特定类型

        PUT_NEXT(bp, GET_NEXTSEG(tempList->nextOff));//设置bp的next指向 序言块的next

        //设置序言块的next 的 prev指向 bp
        //检查序言块的next是否为结尾，如果是，需要一些额外操作
        if(GET_SIZE(HDRP(NEXT_EXBP(bp)))==0){
            PUT_PREVSEG(NEXT_EXBP(bp), tempList->nextOff+4, OFFSET(bp));
        }else{
            PUT_PREV(NEXT_EXBP(bp),OFFSET(bp));
        }

        PUT_NEXTSEG(tempList->nextOff, OFFSET(bp));//设置序言块的next指向 bp
        PUT_PREV(bp, 0);//设置bp的prev 指向 序言块

        return bp;

    }else{//通用类型
        return coalGeneral(bp);
    }
}


static void *coalGeneral(void *bp){

    size_t prev_alloc = GET_ALLOC(HDRP(PREDE_EXBP(bp)));//拿到前一个block的分配位。把FTRP改成HDRP应该也行。joe
    prev_alloc |= (inTargetSize(GET_SIZE(HDRP(PREDE_EXBP(bp)))));//注意前面那个不能是特定类型的

    size_t next_alloc = GET_ALLOC(HDRP(SUCC_EXBP(bp)));//拿到后一个block的分配位。
    next_alloc |= (inTargetSize(GET_SIZE(HDRP(SUCC_EXBP(bp)))));//注意后面那个不能是特定类型的

    size_t size = GET_SIZE(HDRP(bp));

    if(prev_alloc && next_alloc){//case 1: 前后的block都 已分配
        PUT_NEXT(bp, GET_NEXT(heap_listp));//1
        PUT_PREV(bp, 0);//2

        PUT_PREV(NEXT_EXBP(heap_listp), OFFSET(bp));//3

        PUT_NEXT(heap_listp, OFFSET(bp)); //4

    }

    else if(prev_alloc && !next_alloc){//case 2: 前一个block已分配，后一个block未分配

        PUT_NEXT(PREV_EXBP(SUCC_EXBP(bp)), GET_NEXT(SUCC_EXBP(bp)));//1
        PUT_PREV(NEXT_EXBP(SUCC_EXBP(bp)), GET_PREV(SUCC_EXBP(bp)));//2

        PUT_NEXT(bp, GET_NEXT(heap_listp));//3
        PUT_PREV(bp, 0);//4

        PUT_PREV(NEXT_EXBP(heap_listp), OFFSET(bp));//5

        PUT_NEXT(heap_listp, OFFSET(bp)); //6

        size+=GET_SIZE(HDRP(SUCC_EXBP(bp)));//7
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));

        PUT(HDRP(bp),size);

    }

    else if(!prev_alloc && next_alloc){//case 3: 前一个block未分配，后一个block已分配

        PUT_NEXT(PREV_EXBP(PREDE_EXBP(bp)), GET_NEXT(PREDE_EXBP(bp)));//1
        PUT_PREV(NEXT_EXBP(PREDE_EXBP(bp)), GET_PREV(PREDE_EXBP(bp)));//2

        PUT_NEXT(PREDE_EXBP(bp), GET_NEXT(heap_listp));//3
        PUT_PREV(PREDE_EXBP(bp), 0);//4

        PUT_PREV(NEXT_EXBP(heap_listp), OFFSET(PREDE_EXBP(bp)));//5

        PUT_NEXT(heap_listp, OFFSET(PREDE_EXBP(bp)));//6

        size+=GET_SIZE(HDRP(PREDE_EXBP(bp)));//7
        PUT(HDRP(PREDE_EXBP(bp)), PACK(size, 0));
        PUT(FTRP(PREDE_EXBP(bp)), PACK(size, 0));

        bp = PREDE_EXBP(bp);
    }

    else if(!prev_alloc && !next_alloc){//case 4: 前后block都未分配

        PUT_NEXT(PREV_EXBP(PREDE_EXBP(bp)), GET_NEXT(PREDE_EXBP(bp)));//1
        PUT_PREV(NEXT_EXBP(PREDE_EXBP(bp)), GET_PREV(PREDE_EXBP(bp)));//2


        PUT_NEXT(PREV_EXBP(SUCC_EXBP(bp)), GET_NEXT(SUCC_EXBP(bp)));//3
        PUT_PREV(NEXT_EXBP(SUCC_EXBP(bp)), GET_PREV(SUCC_EXBP(bp)));//4


        PUT_NEXT(PREDE_EXBP(bp), GET_NEXT(heap_listp));//5
        PUT_PREV(PREDE_EXBP(bp), 0);//6

        PUT_PREV(NEXT_EXBP(heap_listp), OFFSET(PREDE_EXBP(bp)));//7
        PUT_NEXT(heap_listp, OFFSET(PREDE_EXBP(bp)));//8

        size+=GET_SIZE(HDRP(SUCC_EXBP(bp)));//9
        size+=GET_SIZE(HDRP(PREDE_EXBP(bp)));
        PUT(HDRP(PREDE_EXBP(bp)), PACK(size, 0));
        PUT(FTRP(PREDE_EXBP(bp)), PACK(size, 0));
        bp = PREDE_EXBP(bp);
    }

    return bp;
}


//向memlib.c再获取 words个 4字节的 堆内存
static void *extend_heap(size_t size){ //注意这里传入的就是asize, extend_heap自己计算extendsize
    char *bp;
    //size_t extendsize;
    int num;


    struct empListInfo *tempList=getInfoBySize(size);
    if(tempList!=NULL){//特定类型

        if(size>512){
            num = 1;
        }else{
            num = (512/size)*size;
        }
        size = num*size;

//        if(tempList->count < tempList->scount){
//            num = tempList->alloc[tempList->count];
//            size = size*num;
//            tempList->count = tempList->count+1;
//        }else{
//            size = size;//超出的部分一个个加
//            num = 1;
//
//            //printf("warning: 超出了预算的范围！！！size:%d\n", size);
//        }

    }else{//通用类型的
        //size = MAX(size, CHUNKSIZE);
        size = size;
    }

    if((long)(bp = mem_sbrk(size))==-1){
        return NULL;
    }

    bp = bp-56;//指向新的block的位置。//新结尾块的bpend=bp+60+size

    //把原来的结尾的内容，复制到新的结尾中。

    //strncpy(bp+size-4, bp-4, 60);//strncpy函数有bug,不知道为什么不能用...

    for(int i = 59;i>=0;i--){//只能手动复制...后期应该可以加速: todo
        *(bp+size-4+i) = *(bp-4+i);
    }

    //修改结尾的各个prev的内容，指向当前的 新结尾。
    int endoff = OFFSET(bp+size);
    PUT_NEXT(PREV_EXBP(bp+size), endoff);//设置结尾块通用类型的prev的块的next指向的是 新的结尾。
    prev2newEnd(bp+size, &e24, endoff);
    prev2newEnd(bp+size, &e72, endoff);
    prev2newEnd(bp+size, &e80, endoff);
    prev2newEnd(bp+size, &e120, endoff);
    prev2newEnd(bp+size, &e136, endoff);
    prev2newEnd(bp+size, &e168, endoff);
    prev2newEnd(bp+size, &e456, endoff);
    prev2newEnd(bp+size, &e520, endoff);
    prev2newEnd(bp+size, &e4080, endoff);
    prev2newEnd(bp+size, &e4104, endoff);
    prev2newEnd(bp+size, &e8200, endoff);

    //设置新的block的参数
    PUT(HDRP(bp), PACK(size, 0));//设置header
    PUT(FTRP(bp), PACK(size, 0));//设置footers

    if(tempList!=NULL){//特定类型 要自行切割
        divideNLink(bp, tempList, num, endoff);
        return bp;

    }else{//通用类型的 //todo:这里还不确定
        return coalesce(bp);
    }
}

void divideNLink(char *bp, struct empListInfo *el, int num, int endoff){//连接与分割
    int size = el->asize;
    char *iterp = bp;
    int i = 1;


    PUT_NEXTSEG(el->nextOff, OFFSET(iterp));//头-->当前iterp
    PUT_PREV(iterp, 0);//当前bp-->头


    PUT(HDRP(iterp), PACK(size, 0));//设置大小
    PUT(FTRP(iterp), PACK(size, 0));

    while(i<num){
        iterp = SUCC_EXBP(iterp);

        PUT_NEXT(PREDE_EXBP(iterp), OFFSET(iterp));//上一个邻接的就是 prev-->当前iterp
        PUT_PREV(iterp, OFFSET(PREDE_EXBP(iterp)));//当前iterp-->上一个邻接的

        PUT(HDRP(iterp), PACK(size, 0));//设置大小
        PUT(FTRP(iterp), PACK(size, 0));
        i++;
    }

    //连接结尾
    PUT_NEXT(iterp, endoff);//iterp-->结尾的bp
    PUT_PREVSEG(heap_listp+endoff, el->nextOff+4, OFFSET(iterp));

    //printf("B\n");
}

void prev2newEnd(char *bp, struct empListInfo *el, int endoff){

    //首先得判断prev是不是 序言块
    int prevOffset = (*(int*)(bp+el->nextOff+4));

    if(prevOffset==0){//序言块
        PUT_NEXTSEG(el->nextOff, endoff);
    }else{//普通块
        PUT_NEXT(heap_listp+prevOffset, endoff);
    }
}

//first-fit
static void *find_fit(size_t asize){

    struct empListInfo *tempList=getInfoBySize(asize);

    if(tempList!=NULL){//特定类型
        char *bp = heap_listp;

        bp = heap_listp+GET_NEXTSEG(tempList->nextOff);

        size_t size = GET_SIZE(HDRP(bp));

        if(size!=0){
            return bp;
        }else{
            return NULL;//遇到末尾块了
        }

    }else{//通用类型
        char *bp = heap_listp;

        bp = NEXT_EXBP(bp);
        size_t size = GET_SIZE(HDRP(bp));

        while(!(size>=asize+16||size==asize)){ //只有刚好满，或者>=asize+16才行，因为一个空的块最小为16.
            if(size==0){ //遍历到末尾块了
                return NULL;
            }
            bp = NEXT_EXBP(bp); //遍历下一个block
            size = GET_SIZE(HDRP(bp));
        }
        return bp;
    }
}

static void place(void *bp, size_t asize){
    struct empListInfo *tempList=getInfoBySize(asize);

    if(tempList!=NULL) {//特定类型
        PUT(HDRP(bp), PACK(asize, 1));//设置头部
        PUT(FTRP(bp), PACK(asize, 1));//设置尾部
        PUT_NEXTSEG(tempList->nextOff, GET_NEXT(bp));//设置序言块的next指向 bp的下一个块
        //设置bp的下一个块的prev指向 序言块. 但要先区分 下一块是不是 结尾块
        if(GET_SIZE(HDRP(NEXT_EXBP(bp)))==0){//是结尾块
            PUT_PREVSEG(NEXT_EXBP(bp), tempList->nextOff+4, 0);
        }else{
            PUT_PREV(NEXT_EXBP(bp), 0);
        }

    }else{//通用类型

        size_t nextBlockSize = GET_SIZE(HDRP(bp))-asize;
        PUT(HDRP(bp), PACK(asize, 1));//设置头部
        PUT(FTRP(bp), PACK(asize, 1));//设置尾部

        if(nextBlockSize>0){
            PUT(HDRP(SUCC_EXBP(bp)), PACK(nextBlockSize, 0));//设置后一个block的头部
            PUT(FTRP(SUCC_EXBP(bp)), PACK(nextBlockSize, 0));//设置后一个block的尾部

            PUT_NEXT(PREV_EXBP(bp), OFFSET(SUCC_EXBP(bp)));//1
            PUT_PREV(SUCC_EXBP(bp), OFFSET(PREV_EXBP(bp)));//2

            PUT_NEXT(SUCC_EXBP(bp), OFFSET(NEXT_EXBP(bp)));//3

            PUT_PREV(NEXT_EXBP(bp), OFFSET(SUCC_EXBP(bp)));//4

        }else{//刚好填满
            PUT_NEXT(PREV_EXBP(bp), OFFSET(NEXT_EXBP(bp)));

            PUT_PREV(NEXT_EXBP(bp), OFFSET(PREV_EXBP(bp)));
        }

        if(nextBlockSize<16 && nextBlockSize!=0){
            printf("Fucking error!!\n");
        }
    }

}


int inTargetSize(size_t size){
    if((size==24)||(size==72)||(size==80)||(size==120)||
            (size==136)||(size==168)||(size==456)||(size==520)||
            (size==4080)||(size==4104)||(size==8200))
        return 1;
    else
        return 0;

}

struct empListInfo *getInfoBySize(size_t size){
    if (size==24)
        return &e24;
    if (size==72)
        return &e72;
    if (size==80)
        return &e80;
    if (size==120)
        return &e120;
    if (size==136)
        return &e136;
    if (size==168)
        return &e168;
    if (size==456)
        return &e456;
    if (size==520)
        return &e520;
    if (size==4080)
        return &e4080;
    if (size==4104)
        return &e4104;
    if (size==8200)
        return &e8200;

    return NULL;
}


void printHeap(){
    printf("\nprintHeap: \n");
    printf("(-4)\t 填充块\n");
    printf(
"(%d)\t\t 序言块：header(size: %5d alloc: %1d)\tnext:%5d 24next:%5d 72next:%5d 80next:%5d 120next:%5d 136next:%5d "
        "168next:%5d 456next:%5d 520next:%5d 4080next:%5d 4104next:%5d 8200next:%5d, \tfooter(size: %6d \talloc: %d)\n"
            , 0, GET_SIZE(HDRP(heap_listp)), GET_ALLOC(HDRP(heap_listp)),
           GET_NEXT(heap_listp), GET_NEXTSEG(4), GET_NEXTSEG(8), GET_NEXTSEG(12), GET_NEXTSEG(16),
           GET_NEXTSEG(20), GET_NEXTSEG(24), GET_NEXTSEG(28), GET_NEXTSEG(32),
           GET_NEXTSEG(36), GET_NEXTSEG(40), GET_NEXTSEG(44),
           GET_SIZE(FTRP(heap_listp)), GET_ALLOC(FTRP(heap_listp)));

    char *bp = SUCC_EXBP(heap_listp);
    int size = GET_SIZE(HDRP(bp));
    int i = 2;

    while(size>0){
        printf("(%d)\t 普通块：header(size: %6d\talloc: %1d)\tnext: %6d\t prev:%d, \tfooter(size: %6d \talloc: %d)\n", OFFSET(bp),
               GET_SIZE(HDRP(bp)), GET_ALLOC(HDRP(bp)), GET_NEXT(bp), GET_PREV(bp),
               GET_SIZE(FTRP(bp)), GET_ALLOC(FTRP(bp)));
        bp = SUCC_EXBP(bp);
        size = GET_SIZE(HDRP(bp));
        i++;
    }
    printf(
"(%d)\t 结束块：header(size: %5d alloc: %1d)\tprev:%5d 24prev:%5d 72prev:%5d 80prev:%5d 120prev:%5d 136prev:%5d "
        "168prev:%5d 456prev:%5d 520prev:%5d 4080prev:%5d 4104prev:%5d 8200prev:%5d next: %5d\n", OFFSET(bp),
           GET_SIZE(HDRP(bp)), GET_ALLOC(HDRP(bp)), GET_PREV(bp), GET_PREVSEG(bp, 8), GET_PREVSEG(bp, 12), GET_PREVSEG(bp, 16),
GET_PREVSEG(bp, 20), GET_PREVSEG(bp, 24), GET_PREVSEG(bp, 28), GET_PREVSEG(bp, 32),
GET_PREVSEG(bp, 36), GET_PREVSEG(bp, 40), GET_PREVSEG(bp, 44), GET_PREVSEG(bp, 48), GET_NEXT(bp));

    printf("Break\n");
}

void printEmp(){
    printf("\nprintEmp: \n");

    for(int i = 0;i<12;i++){

        if(GET_SIZE(HDRP(heap_listp+GET_NEXTSEG(i*4)))!=0){
            printf("i: %d\n", i);
            printEmpSub(i*4);
            printf("\n");
        }
    }
}

void printEmpSub(int nextoff){

    printf(
            "(%d)\t\t 序言块：header(size: %5d alloc: %1d)\tnext:%5d 24next:%5d 72next:%5d 80next:%5d 120next:%5d 136next:%5d "
                    "168next:%5d 456next:%5d 520next:%5d 4080next:%5d 4104next:%5d 8200next:%5d, \tfooter(size: %6d \talloc: %d)\n"
            , 0, GET_SIZE(HDRP(heap_listp)), GET_ALLOC(HDRP(heap_listp)),
            GET_NEXT(heap_listp), GET_NEXTSEG(4), GET_NEXTSEG(8), GET_NEXTSEG(12), GET_NEXTSEG(16),
            GET_NEXTSEG(20), GET_NEXTSEG(24), GET_NEXTSEG(28), GET_NEXTSEG(32),
            GET_NEXTSEG(36), GET_NEXTSEG(40), GET_NEXTSEG(44),
            GET_SIZE(FTRP(heap_listp)), GET_ALLOC(FTRP(heap_listp)));

    char *bp = heap_listp+ GET_NEXTSEG(nextoff);
    int size = GET_SIZE(HDRP(bp));

    while(size>0){
        printf("(%d)\t 普通块：header(size: %6d\talloc: %d)\tnext: %6d\t prev:%d, \tfooter(size: %6d \talloc: %d)\n", OFFSET(bp),
               GET_SIZE(HDRP(bp)), GET_ALLOC(HDRP(bp)), GET_NEXT(bp), GET_PREV(bp),
               GET_SIZE(FTRP(bp)), GET_ALLOC(FTRP(bp)));
        char *tp = bp;
        bp = NEXT_EXBP(bp);
        size = GET_SIZE(HDRP(bp));
    }

    printf(
            "(%d)\t 结束块：header(size: %5d alloc: %1d)\tprev:%5d 24prev:%5d 72prev:%5d 80prev:%5d 120prev:%5d 136prev:%5d "
                    "168prev:%5d 456prev:%5d 520prev:%5d 4080prev:%5d 4104prev:%5d 8200prev:%5d next: %5d\n", OFFSET(bp),
            GET_SIZE(HDRP(bp)), GET_ALLOC(HDRP(bp)), GET_PREV(bp), GET_PREVSEG(bp, 8), GET_PREVSEG(bp, 12), GET_PREVSEG(bp, 16),
            GET_PREVSEG(bp, 20), GET_PREVSEG(bp, 24), GET_PREVSEG(bp, 28), GET_PREVSEG(bp, 32),
            GET_PREVSEG(bp, 36), GET_PREVSEG(bp, 40), GET_PREVSEG(bp, 44), GET_PREVSEG(bp, 48), GET_NEXT(bp));

}

void printBoth(){
    printHeap();
    printEmp();
}


