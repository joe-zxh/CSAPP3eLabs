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

//根据bp，设置 下(上)一个空的block的bp的位置。
#define PUT_NEXT(bp, off) (*((int *)((char *)(bp)))=(off))
#define PUT_PREV(bp, off) (*((int *)((char *)(bp)+WSIZE))=(off))

//根据bp，获取 下(上)一个空的block的bp的位置。
#define GET_NEXT(bp) (*((int *)((char *)(bp))))
#define GET_PREV(bp) (*((int *)((char *)(bp)+WSIZE)))

//bp相对于heap_listp的偏移
#define OFFSET(bp) ((int*)((char *)(bp) -heap_listp))

//根据bp，计算下一个bp的指针，或者上一个bp的指针。
#define NEXT_EXBP(bp) (heap_listp+GET_NEXT(bp))
#define PREV_EXBP(bp) (heap_listp+GET_PREV(bp))


//mm.c的私有的全局变量
static char *heap_listp;//指向P598页所指向的位置

static void *extend_heap(size_t words);
static void *coalesce(void *bp);
static void *find_fit(size_t asize);
static void place(void *bp, size_t asize);
void printHeap();
void printEmp();
/*
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    //首先要6个word：1个word的填充+4个word的序言块+3个word的结尾块。
    if((heap_listp=mem_sbrk(8*WSIZE)) ==(void *)-1){
        return -1;
    }

    PUT(heap_listp, 0);//填充块
    PUT(heap_listp + (1*WSIZE), PACK(16, 1));//序言块header：记录4个word大小(16字节)，有效位为1
    PUT(heap_listp + (2*WSIZE), 16);//序言块next:初始时 指向结尾块的bp
    PUT(heap_listp + (3*WSIZE), -1);//序言块prev:null。-1表示当前为序言块
    PUT(heap_listp + (4*WSIZE), PACK(16, 1));//序言块的footer

    PUT(heap_listp + (5*WSIZE), PACK(0, 1));//结尾块.size=0表示为结尾块
    PUT(heap_listp + (6*WSIZE), -1);//结尾块的next一直是-1
    PUT(heap_listp + (7*WSIZE), 0);//结尾块的prev初始化为0 即指向序言块的bp

    heap_listp+=(2*WSIZE);
    int size;
    //获取一部分堆用于分配。
    if(extend_heap(CHUNKSIZE/WSIZE) == NULL){
        return -1;
    }

    return 0;
}

/*
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    size_t asize; //调整后的块的总大小
    size_t extendsize; //需要扩展heap的大小。
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
    extendsize = MAX(asize, CHUNKSIZE);
    if((bp=extend_heap(extendsize/WSIZE))==NULL){
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
static void *coalesce(void *bp){
    int temp;

    temp = GET_SIZE(((char *)(bp)-DSIZE));

    size_t prev_alloc = GET_ALLOC(HDRP(PREDE_EXBP(bp)));//拿到前一个block的分配位。把FTRP改成HDRP应该也行。joe
    size_t next_alloc = GET_ALLOC(HDRP(SUCC_EXBP(bp)));//拿到后一个block的分配位。
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

//向memlib.c再获取 words个 4字节的 堆内存 todo: 这里有严重的bug
static void *extend_heap(size_t words){

    char *bp;
    size_t size;

    size = (words%2)?(words+1)*WSIZE:words*WSIZE;//偶数个word以致 对齐

    if((long)(bp = mem_sbrk(size))==-1){
        return NULL;
    }

    bp=bp-8;//指向新增的内存的bp

    //设置新的block的参数
    PUT(HDRP(bp), PACK(size, 0));//设置header
    PUT(FTRP(bp), PACK(size, 0));//设置footers


    PUT_NEXT(PREV_EXBP(bp), OFFSET(SUCC_EXBP(bp)));
    PUT_PREV(SUCC_EXBP(bp), OFFSET(PREV_EXBP(bp)));


    //设置新的结尾块的参数
    PUT(HDRP(SUCC_EXBP(bp)), PACK(0, 1));//设置结尾块的header
    PUT_NEXT(SUCC_EXBP(bp), -1);//设置结尾块的next为-1

    return coalesce(bp);//todo:这里还不确定
}

//first-fit
static void *find_fit(size_t asize){
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

static void place(void *bp, size_t asize){ //todo: 这里可能有问题

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


void DPrintInt(void *a){
    printf("%d\n",a);
}

void DPrintPtr(char *a){
    printf("%p\n",a);
}

void printHeap(){
    printf("\nprintHeap: \n");
    printf("(-4)\t 填充块\n");
    printf("(%d)\t\t 序言块：header(size: %6d\talloc: %d)\tnext: %6d\t prev:%d, \tfooter(size: %6d \talloc: %d)\n", 0,
           GET_SIZE(HDRP(heap_listp)), GET_ALLOC(HDRP(heap_listp)), GET_NEXT(heap_listp), GET_PREV(heap_listp),
           GET_SIZE(FTRP(heap_listp)), GET_ALLOC(FTRP(heap_listp)));
    char *bp = SUCC_EXBP(heap_listp);
    int size = GET_SIZE(HDRP(bp));
    int i = 2;

    while(size>0){
        printf("(%d)\t 普通块：header(size: %6d\talloc: %d)\tnext: %6d\t prev:%d, \tfooter(size: %6d \talloc: %d)\n", OFFSET(bp),
               GET_SIZE(HDRP(bp)), GET_ALLOC(HDRP(bp)), GET_NEXT(bp), GET_PREV(bp),
               GET_SIZE(FTRP(bp)), GET_ALLOC(FTRP(bp)));
        bp = SUCC_EXBP(bp);
        size = GET_SIZE(HDRP(bp));
        i++;
    }
    printf("(%d)\t 结束块：header(size: %6d\talloc: %d)\tnext: %6d\t prev:%d\n", OFFSET(bp),
           GET_SIZE(HDRP(bp)), GET_ALLOC(HDRP(bp)), GET_NEXT(bp), GET_PREV(bp));


    printf("Break\n");
}

void printEmp(){
    printf("\nprintEmp: \n");
    printf("(%d)\t\t 序言块：header(size: %6d\talloc: %d)\tnext: %6d\t prev:%d, \tfooter(size: %6d \talloc: %d)\n", 0,
           GET_SIZE(HDRP(heap_listp)), GET_ALLOC(HDRP(heap_listp)), GET_NEXT(heap_listp), GET_PREV(heap_listp),
           GET_SIZE(FTRP(heap_listp)), GET_ALLOC(FTRP(heap_listp)));
    char *bp = NEXT_EXBP(heap_listp);
    int size = GET_SIZE(HDRP(bp));
    int i = 2;

    while(size>0){
        printf("(%d)\t 普通块：header(size: %6d\talloc: %d)\tnext: %6d\t prev:%d, \tfooter(size: %6d \talloc: %d)\n", OFFSET(bp),
               GET_SIZE(HDRP(bp)), GET_ALLOC(HDRP(bp)), GET_NEXT(bp), GET_PREV(bp),
               GET_SIZE(FTRP(bp)), GET_ALLOC(FTRP(bp)));
        bp = NEXT_EXBP(bp);
        size = GET_SIZE(HDRP(bp));
        i++;
    }
    printf("(%d)\t 结束块：header(size: %6d\talloc: %d)\tnext: %6d\t prev:%d\n", OFFSET(bp),
           GET_SIZE(HDRP(bp)), GET_ALLOC(HDRP(bp)), GET_NEXT(bp), GET_PREV(bp));

    printf("Break\n");

}

void printBoth(){
    printHeap();
    printEmp();
}



