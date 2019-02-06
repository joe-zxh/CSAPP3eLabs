/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 * 
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
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
    "joeTeam",
    /* First member's full name */
    "Joe",
    /* First member's email address */
    "joe@scut.edu",
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

//根据bp，计算下一个bp的指针，或者上一个bp的指针。
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(((char *)(bp)-WSIZE)))//((char *)(bp)-WSIZE))应该可以改成HDRP(bp))joe
#define PREV_BLKP(bp) ((char *)(bp) - GET_SIZE(((char *)(bp)-DSIZE)))

//mm.c的私有的全局变量
static char *heap_listp;//指向P598页所指向的位置

static void *extend_heap(size_t words);
static void *coalesce(void *bp);
static void *find_fit(size_t asize);
static void place(void *bp, size_t asize);

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    //首先要4个word：1个word的填充+2个word的序言块+1个word的结尾块。
    if((heap_listp=mem_sbrk(4*WSIZE)) ==(void *)-1){
        return -1;
    }

    PUT(heap_listp ,0);//填充块
    PUT(heap_listp + (1*WSIZE), PACK(DSIZE, 1));//序言块header：记录2个word大小，有效位为1
    PUT(heap_listp + (2*WSIZE), PACK(DSIZE, 1));//序言块footer
    PUT(heap_listp + (3*WSIZE), PACK(0, 1));//结尾块只有header，有效位为1.块大小为0:用于标记末尾
    heap_listp+=(2*WSIZE);//指向P598页所指向的位置
    //printf("\nheap_listp: %p\n",heap_listp);

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

    if(size<=DSIZE){
        asize = 2*DSIZE;
        //1个word大小的header+1个word的大小的footer，又需要整个block的大小为偶数个word，所以最小需要4个word
    } else{
        asize = DSIZE * ((size+(DSIZE)+(DSIZE-1))/DSIZE);//这地方可能会有问题
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
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));//拿到前一个block的分配位。把FTRP改成HDRP应该也行。joe
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));//拿到后一个block的分配位。
    size_t size = GET_SIZE(HDRP(bp));

    if(prev_alloc && next_alloc){//case 1: 前后的block都 已分配
        return bp;
    }

    else if(prev_alloc && !next_alloc){//case 2: 前一个block已分配，后一个block未分配
        size+=GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
    }

    else if(!prev_alloc && next_alloc){//case 3: 前一个block未分配，后一个block已分配
        size+=GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(FTRP(bp), PACK(size, 0));//设置尾部
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));//设置头部
        bp = PREV_BLKP(bp);
    }

    else if(!prev_alloc && !next_alloc){//case 4: 前后block都未分配
        size+=GET_SIZE(HDRP(PREV_BLKP(bp)));//加前一个block的size
        size+=GET_SIZE(FTRP(NEXT_BLKP(bp)));//加后一个block的size 这里FTRP换成HDRP应该也是可以的joe
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));//设置头部
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));//设置尾部
    }

    return bp;
}

//向memlib.c再获取 words个 4字节的 堆内存
static void *extend_heap(size_t words){
    char *bp;
    size_t size;

    size = (words%2)?(words+1)*WSIZE:words*WSIZE;//偶数个word以致 对齐

    if((long)(bp = mem_sbrk(size))==-1){
        return NULL;
    }

    //设置一下这个空的块的参数。
    PUT(HDRP(bp), PACK(size, 0));//设置header，有效位为0
    PUT(FTRP(bp), PACK(size, 0));
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1)); //设置新的结尾块

    //如果前一个块是free的，那么需要合并一下
    return coalesce(bp);
}

//first-fit
static void *find_fit(size_t asize){
    char *bp = heap_listp;

    bp = NEXT_BLKP(bp);
    size_t size = GET_SIZE(HDRP(bp));
    size_t alloc = GET_ALLOC(HDRP(bp));
    //todo:这里应该有问题

    while(alloc==1 || (alloc==0&&size<asize)){
        if(size==0){ //遍历到末尾块了
            return NULL;
        }
        bp = NEXT_BLKP(bp); //遍历下一个block
        size = GET_SIZE(HDRP(bp));
        alloc = GET_ALLOC(HDRP(bp));
    }
    return bp;
}

static void place(void *bp, size_t asize){
    //printf("%p\n",bp);
    //printf("%d\n",GET_SIZE(HDRP(bp)));

    size_t nextBlockSize = GET_SIZE(HDRP(bp))-asize;

    PUT(HDRP(bp), PACK(asize, 1));//设置头部
    PUT(FTRP(bp), PACK(asize, 1));//设置尾部

    if(nextBlockSize>0){
        PUT(HDRP(NEXT_BLKP(bp)), PACK(nextBlockSize, 0));//设置后一个block的头部
        PUT(FTRP(NEXT_BLKP(bp)), PACK(nextBlockSize, 0));//设置后一个block的尾部
        //printf("%p\n",NEXT_BLKP(bp));
        //printf("%d\n",nextBlockSize);
    }
}









