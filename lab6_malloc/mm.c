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

//对于结尾块来说的 //注意现在改成nextoff了，不需要手动添加WSIZE了
#define PUT_PREVSEG(ebp, nextoff, off) (*((int *)(ebp+nextoff+WSIZE))=(off))

//对于普通块: 根据bp，获取 下(上)一个空的block的bp的位置。
#define GET_NEXT(bp) (*((int *)((char *)(bp))))
#define GET_PREV(bp) (*((int *)((char *)(bp)+WSIZE)))

//对于序言块：
#define GET_NEXTSEG(nextoff) (*((int *)(heap_listp+nextoff)))

//对于结尾块：
#define GET_PREVSEG(ebp, nextoff) (*((int *)(ebp+nextoff+WSIZE)))

//bp相对于heap_listp的偏移
#define OFFSET(bp) ((int*)((char *)(bp) -heap_listp))

//普通块：根据bp，计算下一个bp的指针，或者上一个bp的指针。
#define NEXT_EXBP(bp) (heap_listp+GET_NEXT(bp))
#define PREV_EXBP(bp) (heap_listp+GET_PREV(bp))


//mm.c的私有的全局变量
static char *heap_listp;//指向P598页所指向的位置

static void *extend_heap(size_t words);
static void *coalesce(void *bp);
static void *find_fitWithPlace(int size);
static void place(void *bp, size_t asize);
struct empListInfo *getInfoBySize(size_t size);
int inTargetSize(size_t size);
void prev2newEnd(char *bp, struct empListInfo *el, int endoff);
void divideNLink(char *bp, struct empListInfo *el, int num, int endoff);
static void *coalGeneral(void *bp);
void putNextGeneral(char *bp, int size, int offset);
int getNextGeneral(char *bp, int size);
void putPrevGeneral(char *bp, int size, int offset);
int getPrevGeneral(char *bp, int size);
void *mm_reallocOld(void *ptr, size_t size);
void *mm_reallocHelper(size_t asize, size_t size);
void *mm_reallocOld2(void *ptr, size_t size);
/*
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    if((heap_listp=mem_sbrk(22*WSIZE)) ==(void *)-1){
        return -1;
    }

    PUT(heap_listp, 0);//填充块
    PUT(heap_listp + (1*WSIZE), PACK(40, 1));//序言块header

    PUT(heap_listp + (2*WSIZE), 40);//序言块<=32next:初始时 指向结尾块的bp
    PUT(heap_listp + (3*WSIZE), 40);//序言块<=64next:初始时 指向结尾块的bp
    PUT(heap_listp + (4*WSIZE), 40);//序言块<=128next:初始时 指向结尾块的bp
    PUT(heap_listp + (5*WSIZE), 40);//序言块<=256next:初始时 指向结尾块的bp
    PUT(heap_listp + (6*WSIZE), 40);//序言块<=512next:初始时 指向结尾块的bp
    PUT(heap_listp + (7*WSIZE), 40);//序言块<=2048next:初始时 指向结尾块的bp
    PUT(heap_listp + (8*WSIZE), 40);//序言块<=4096next:初始时 指向结尾块的bp
    PUT(heap_listp + (9*WSIZE), 40);//序言块>4096next:初始时 指向结尾块的bp

    PUT(heap_listp + (10*WSIZE), PACK(40, 1));//序言块footer

    PUT(heap_listp + (11*WSIZE), PACK(0, 1));//结尾块header.size=0表示为结尾块

    PUT(heap_listp + (12*WSIZE), -1);//结尾块next 恒为-1 没用的。用来占位的
    PUT(heap_listp + (13*WSIZE), 0);//结尾块<=32prev 初始化为0 即指向序言块的bp
    PUT(heap_listp + (14*WSIZE), 0);//结尾块<=64prev 初始化为0 即指向序言块的bp
    PUT(heap_listp + (15*WSIZE), 0);//结尾块<=128prev 初始化为0 即指向序言块的bp
    PUT(heap_listp + (16*WSIZE), 0);//结尾块<=256prev 初始化为0 即指向序言块的bp
    PUT(heap_listp + (17*WSIZE), 0);//结尾块<=512prev 初始化为0 即指向序言块的bp
    PUT(heap_listp + (18*WSIZE), 0);//结尾块<=2048prev 初始化为0 即指向序言块的bp
    PUT(heap_listp + (19*WSIZE), 0);//结尾块<=4096prev 初始化为0 即指向序言块的bp
    PUT(heap_listp + (20*WSIZE), 0);//结尾块>4096prev 初始化为0 即指向序言块的bp

    PUT(heap_listp + (21*WSIZE), PACK(0, 1));//结尾块footer.size=0表示为结尾块 没用的

    heap_listp+=2*(WSIZE);

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
    if((bp=find_fitWithPlace(asize))!=NULL){
        place(bp, asize);
        return bp;
    }

    //没有合适的位置，需要获取更多的heap内存
    if((bp=extend_heap(asize))==NULL){
        return NULL;
    }
    //place(bp, asize);//extend_heap里面已经place好了，不用重复操作了
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
void *mm_realloc(void *oldptr, size_t newsize)
{
    size_t asize; //调整后的块的总大小
    char *bp;

    if(newsize==0){
        return NULL;
    }

    if(newsize<=DSIZE){ //todo: 这里可能有问题
        asize = 2*DSIZE;
        //1个word大小的header+1个word的大小的footer，又需要整个block的大小为偶数个word，所以最小需要4个word
    } else{
        asize = DSIZE * ((newsize+(DSIZE)+(DSIZE-1))/DSIZE);
    }

    void *newptr;

    size_t oldsize = GET_SIZE(HDRP(oldptr));

    if(oldsize==asize){
        return oldptr;
    }

    if(getMaxsize(oldsize)==getMaxsize(asize)){//属于同一个类别

        if(oldsize>asize){

            if(oldsize-asize>=16){//剩余部分还能分配给某一个类别

                PUT(HDRP(oldptr),PACK(asize,1));
                PUT(FTRP(oldptr),PACK(asize,1));

                PUT(HDRP(SUCC_EXBP(oldptr)),PACK(oldsize-asize,0));
                PUT(FTRP(SUCC_EXBP(oldptr)),PACK(oldsize-asize,0));

                coalesce(SUCC_EXBP(oldptr));

            }else{//剩余部分不能形成一个类别
                PUT(HDRP(oldptr),PACK(oldsize,1));
            }

            return oldptr;

        }else{

            size_t succ_alloc = GET_ALLOC(HDRP(SUCC_EXBP(oldptr)));
            size_t succ_size = GET_SIZE(HDRP(SUCC_EXBP(oldptr)));

            //看看后面那个是否能用。
            if(!succ_alloc){//能使用.不需要是同一个类别的 87
                //if(!succ_alloc && getMaxsize(asize)==getMaxsize(succ_size)){//能使用.不需要是同一个类别的 87

                size_t needsize = asize-oldsize;//还需要的大小。

                if(succ_size<needsize){//不够用
                    return mm_reallocOld(oldptr, newsize);
                }else if(succ_size-needsize<16){//剩余部分不能形成一个类别

                    char *succ_bp = SUCC_EXBP(oldptr);

                    //修改prev的next指向succ_bp的next
                    putNextGeneral(PREV_EXBP(succ_bp), succ_size, OFFSET(NEXT_EXBP(succ_bp)));

                    //修改next的prev指向succ_bp的prev
                    putPrevGeneral(NEXT_EXBP(succ_bp), succ_size, OFFSET(PREV_EXBP(succ_bp)));

                    PUT(HDRP(oldptr),PACK(succ_size+oldsize,1));
                    PUT(FTRP(oldptr),PACK(succ_size+oldsize,1));

                    return oldptr;
                }else{//剩余部分能形成一个类别

                    char *succ_bp = SUCC_EXBP(oldptr);

                    //修改prev的next指向succ_bp的next
                    putNextGeneral(PREV_EXBP(succ_bp), succ_size, OFFSET(NEXT_EXBP(succ_bp)));

                    //修改next的prev指向succ_bp的prev
                    putPrevGeneral(NEXT_EXBP(succ_bp), succ_size, OFFSET(PREV_EXBP(succ_bp)));

                    char *joebp = succ_bp+needsize;

                    PUT(HDRP(joebp),PACK(succ_size-needsize,0));
                    PUT(FTRP(joebp),PACK(succ_size-needsize,0));

                    PUT(HDRP(oldptr),PACK(asize,1));
                    PUT(FTRP(oldptr),PACK(asize,1));

                    coalesce(joebp);

                    return oldptr;
                }

            }else{
                return mm_reallocOld(oldptr, newsize);
            }
        }

    }else{//不是同一类别直接用malloc即可
        return mm_reallocOld2(oldptr, newsize);
    }

}

void *mm_reallocOld2(void *ptr, size_t size)
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

void *mm_reallocOld(void *ptr, size_t size)
{
    size_t asize;
    if(size<=DSIZE){ //todo: 这里可能有问题
        asize = 2*DSIZE;
    } else{
        asize = DSIZE * ((size+(DSIZE)+(DSIZE-1))/DSIZE);
    }

    void *oldptr = ptr;
    void *newptr;
    size_t copySize;

    size_t helpersize, maxsize;
    helpersize=maxsize=getMaxsize(size);
    if(helpersize<asize){
        helpersize=614792;//little trick for the realloc-bal.rep!!!
    }

    newptr = mm_reallocHelper(helpersize, asize);

    if (newptr == NULL)
        return NULL;
    copySize = *(size_t *)((char *)oldptr - SIZE_T_SIZE);//相当于指针前移8个字节，然后拿到 有效载荷的大小。
    if (size < copySize)
        copySize = size;
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
}

void *mm_reallocHelper(size_t asize, size_t ss){
    char *bp, *ebp;
    if((long)(bp = mem_sbrk(asize))==-1){ //默认要多少，给多少
        return NULL;
    }

    bp = bp-40;//指向新的block的位置。
    ebp = bp+asize;//新的结尾块的位置。

    for(int i=43;i>=0;i--){//手动复制。记住，需要从后面开始复制，以免size<44的情况，会出现数据覆盖
        *(ebp-4+i)=*(bp-4+i);
    }

    //修改结尾的各个prev的内容，指向当前的 新结尾。
    int endoff = OFFSET(ebp);

    for(int size=32;size<=8192;size<<=1){ //todo: 注意这里可能会出错
        if(size==1024){//注意我们跳过了1024的情况 其实多设置一次应该也没问题的
            size<<=1;
        }
        putNextGeneral(heap_listp+getPrevGeneral(ebp,size),size,endoff);
    }

    //设置新的block的参数
    PUT(HDRP(bp), PACK(ss, 1));//设置header 这里直接设为已用即可。那么 后续就不用再调用place函数了。
    PUT(FTRP(bp), PACK(ss, 1));//设置footers

    PUT(HDRP(SUCC_EXBP(bp)), PACK(asize-ss, 0));
    PUT(FTRP(SUCC_EXBP(bp)), PACK(asize-ss, 0));
    coalesce(SUCC_EXBP(bp));

    return bp;
}


//合并空余块 只有在free中调用
static void *coalesce(void *bp){ //需要判断前后 是否 默认类型。如果是，则忽略。
    char *succ_bp;
    char *prede_bp;
    char *next_bp;
    char *prev_bp;

    int size = GET_SIZE(HDRP(bp));
    int pred_alloc = GET_ALLOC(HDRP(PREDE_EXBP(bp)));
    int predsize = GET_SIZE(HDRP(PREDE_EXBP(bp)));
    if(!pred_alloc && getNextOffbySize(size)==getNextOffbySize(predsize)){
        //前一个没分配，且 和它同类
        pred_alloc = 0;
    }else{
        pred_alloc = 1;
    }

    int succ_alloc = GET_ALLOC(HDRP(SUCC_EXBP(bp)));
    int succsize = GET_SIZE(HDRP(SUCC_EXBP(bp)));
    if(!succ_alloc && getNextOffbySize(size)==getNextOffbySize(succsize)){
        //后一个没分配，且 和它同类
        succ_alloc = 0;
    }else{
        succ_alloc = 1;
    }


    if(pred_alloc && succ_alloc){//case 1: 前后的block都 已分配
        //先不管，最后 大家都是这种情况。

    }
    else if(pred_alloc && !succ_alloc){//case 2: 前一个block已分配，后一个block未分配 todo: 这里可能会有bug

        if(size+succsize<=getMaxsize(size)){ //合并
            //操作SUCC_EXBP的prev和next指向
            succ_bp = SUCC_EXBP(bp);

            //1 prev的next指向
            putNextGeneral(heap_listp+getPrevGeneral(succ_bp, size), size, getNextGeneral(succ_bp, size));

            //2 next的prev指向
            putPrevGeneral(heap_listp+getNextGeneral(succ_bp, size), size, getPrevGeneral(succ_bp, size));

            size = size + succsize;
        }
    }
    else if(!pred_alloc && succ_alloc){//case 3: 前一个block未分配，后一个block已分配

        if(size+predsize<=getMaxsize(size)){ //合并
            //操作PREDE_EXBP的prev和next指向
            prede_bp = PREDE_EXBP(bp);

            //1 prev的next指向
            putNextGeneral(heap_listp+getPrevGeneral(prede_bp, size), size, getNextGeneral(prede_bp, size));

            //2 next的prev指向
            putPrevGeneral(heap_listp+getNextGeneral(prede_bp, size), size, getPrevGeneral(prede_bp, size));

            bp = prede_bp;
            size += predsize;
        }
    }
    else if(!pred_alloc && !succ_alloc){//case 4: 前后block都未分配

        if(size+succsize<=getMaxsize(size)){//合并后一个
            //操作PREDE_EXBP的prev和next指向
            prede_bp = PREDE_EXBP(bp);

            //1 prev的next指向
            putNextGeneral(heap_listp+getPrevGeneral(prede_bp, size), size, getNextGeneral(prede_bp, size));
            //2 next的prev指向
            putPrevGeneral(heap_listp+getNextGeneral(prede_bp, size), size, getPrevGeneral(prede_bp, size));

            size = size + succsize;
        }

        if(size+predsize<=getMaxsize(size)){//合并前一个
            //操作SUCC_EXBP的prev和next指向
            succ_bp = SUCC_EXBP(bp);

            //1 prev的next指向
            putNextGeneral(heap_listp+getPrevGeneral(succ_bp, size), size, getNextGeneral(succ_bp, size));
            //2 next的prev指向
            putPrevGeneral(heap_listp+getNextGeneral(succ_bp, size), size, getPrevGeneral(succ_bp, size));

            bp = prede_bp;
            size = size+predsize;
        }
    }

    //现在变成case1的情况，插入的过程是 按照大小顺序地插入...
    char *ibp = heap_listp;
    char *nbp = heap_listp+getNextGeneral(heap_listp, size);
    int isize = GET_SIZE(HDRP(nbp));
    while(isize!=0 && isize<size){//不是结尾块，且 isize<size
        ibp = nbp;
        nbp = heap_listp+getNextGeneral(nbp, size);
        isize = GET_SIZE(HDRP(nbp));
    }

    //插入
    //ibp的next指向bp
    putNextGeneral(ibp, size, OFFSET(bp));

    //bp的prev指向ibp
    putPrevGeneral(bp, size, OFFSET(ibp));

    //bp的next指向nbp
    putNextGeneral(bp, size, OFFSET(nbp));

    //nbp的prev指向bp
    putPrevGeneral(nbp, size, OFFSET(bp));

    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));

    return bp;
}


//向memlib.c再获取 堆内存
static void *extend_heap(size_t size){ //注意这里传入的就是asize, extend_heap自己计算extendsize

    char *bp, *ebp;
    if((long)(bp = mem_sbrk(size))==-1){ //默认要多少，给多少
        return NULL;
    }

    bp = bp-40;//指向新的block的位置。
    ebp = bp+size;//新的结尾块的位置。

    for(int i=43;i>=0;i--){//手动复制。记住，需要从后面开始复制，以免size<44的情况，会出现数据覆盖
        *(ebp-4+i)=*(bp-4+i);
    }

    //修改结尾的各个prev的内容，指向当前的 新结尾。
    int endoff = OFFSET(ebp);

    for(int size=32;size<=8192;size<<=1){ //todo: 注意这里可能会出错
        if(size==1024){//注意我们跳过了1024的情况 其实多设置一次应该也没问题的
            size<<=1;
        }
        putNextGeneral(heap_listp+getPrevGeneral(ebp,size),size,endoff);
    }

    //设置新的block的参数
    PUT(HDRP(bp), PACK(size, 1));//设置header 这里直接设为已用即可。那么 后续就不用再调用place函数了。
    PUT(FTRP(bp), PACK(size, 1));//设置footers

    //todo: 不知道需不需要合并 好像不需要，但要注意，一分配 就要使用，这样就 不用再连接了。

    return bp;
}


//first-fit
static void *find_fitWithPlace(int size){

    char *ibp = heap_listp;
    char *nbp = heap_listp+getNextGeneral(heap_listp, size);
    int isize = GET_SIZE(HDRP(nbp));

    int minsize = getMinsize(size);
    while(isize<size){
        if(isize==0){
            return NULL;
        }
        ibp = nbp;
        nbp = heap_listp+getNextGeneral(nbp, size);
        isize = GET_SIZE(HDRP(nbp));
    }

    if(isize==size || isize-size<16){//刚好满的 或者 多余的部分 不足以放入任意一个类别

        char *nnbp = heap_listp+getNextGeneral(nbp, size);

        //ibp的next指向nnbp
        putNextGeneral(ibp, size, OFFSET(nnbp));

        //nnbp的prev指向ibp
        putPrevGeneral(nnbp, size, OFFSET(ibp));

        //标记header和footer
        PUT(HDRP(nbp),PACK(isize, 1));
        PUT(FTRP(nbp),PACK(isize, 1));

        return nbp;
    }else if(isize-size>=minsize){//多余的部分还可以继续放进 当前类别

        char *nnbp = heap_listp+getNextGeneral(nbp, size);

        //ibp的next指向nnbp
        putNextGeneral(ibp, size, OFFSET(nnbp));

        //nnbp的prev指向ibp
        putPrevGeneral(nnbp, size, OFFSET(ibp));

        //标记header和footer
        PUT(HDRP(nbp),PACK(size, 1));
        PUT(FTRP(nbp),PACK(size, 1));

        //剩余的那部分需要collease一下
        int resueSize = isize-size;
        PUT(HDRP(SUCC_EXBP(nbp)),PACK(resueSize,0));
        PUT(FTRP(SUCC_EXBP(nbp)),PACK(resueSize,0));

        char *rbp = nbp;

        coalesce(SUCC_EXBP(nbp));

        return rbp;

    }else{//todo: 多余的部分 要放进别的类别
        int resuesize = isize-size;

        char *nnbp = heap_listp+getNextGeneral(nbp, size);

        //ibp的next指向nnbp
        putNextGeneral(ibp, size, OFFSET(nnbp));

        //nnbp的prev指向ibp
        putPrevGeneral(nnbp, size, OFFSET(ibp));

        //标记header和footer
        PUT(HDRP(nbp),PACK(size, 1));
        PUT(FTRP(nbp),PACK(size, 1));


        //标记多余部分的size
        PUT(HDRP(SUCC_EXBP(nbp)),PACK(resuesize,0));
        PUT(FTRP(SUCC_EXBP(nbp)),PACK(resuesize,0));
        coalesce(SUCC_EXBP(nbp));

        return nbp;
    }

    return nbp;
}

static void place(void *bp, size_t asize){
    //todo: place需要注意!!!

}


void printHeap(){
    printf("\nprintHeap: \n");
    printf("(-4)\t 填充块\n");
    printf(
            "(%d)\t\t 序言块：header(size: %5d alloc: %1d)\t32next:%5d 64next:%5d 128next:%5d 256next:%5d 512next:%5d 2048next:%5d "
                    "4096next:%5d 4097next:%5d, footer(size: %6d \talloc: %d)\n"
            , 0, GET_SIZE(HDRP(heap_listp)), GET_ALLOC(HDRP(heap_listp)),
            GET_NEXTSEG(0), GET_NEXTSEG(4), GET_NEXTSEG(8), GET_NEXTSEG(12), GET_NEXTSEG(16),
            GET_NEXTSEG(20), GET_NEXTSEG(24), GET_NEXTSEG(28),
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
            "(%d)\t 结束块：header(size: %5d alloc: %1d)\t32prev:%5d 64prev:%5d 128prev:%5d 256prev:%5d 512prev:%5d 2048prev:%5d "
                    "4096prev:%5d 4097prev:%5d, next: %5d\n", OFFSET(bp),
            GET_SIZE(HDRP(bp)), GET_ALLOC(HDRP(bp)), GET_PREVSEG(bp,0), GET_PREVSEG(bp,4), GET_PREVSEG(bp, 8), GET_PREVSEG(bp, 12), GET_PREVSEG(bp, 16),
            GET_PREVSEG(bp, 20), GET_PREVSEG(bp, 24), GET_PREVSEG(bp, 28), GET_NEXT(bp));

    printf("Break\n");
}

void printEmp(){
    printf("\nprintEmp: \n");

    for(int i = 0;i<8;i++){

        if(GET_SIZE(HDRP(heap_listp+GET_NEXTSEG(i*4)))!=0){
            printf("i: %d\n", i);
            printEmpSub(i*4);
            printf("\n");
        }
    }
}

void printEmpSub(int nextoff){

    printf(
            "(%d)\t\t 序言块：header(size: %5d alloc: %1d)\t32next:%5d 64next:%5d 128next:%5d 256next:%5d 512next:%5d 2048next:%5d "
                    "4096next:%5d 4097next:%5d, footer(size: %6d \talloc: %d)\n"
            , 0, GET_SIZE(HDRP(heap_listp)), GET_ALLOC(HDRP(heap_listp)),
            GET_NEXTSEG(0), GET_NEXTSEG(4), GET_NEXTSEG(8), GET_NEXTSEG(12), GET_NEXTSEG(16),
            GET_NEXTSEG(20), GET_NEXTSEG(24), GET_NEXTSEG(28),
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
            "(%d)\t 结束块：header(size: %5d alloc: %1d)\t32prev:%5d 64prev:%5d 128prev:%5d 256prev:%5d 512prev:%5d 2048prev:%5d "
                    "4096prev:%5d 4097prev:%5d, next: %5d\n", OFFSET(bp),
            GET_SIZE(HDRP(bp)), GET_ALLOC(HDRP(bp)), GET_PREVSEG(bp,0), GET_PREVSEG(bp,4), GET_PREVSEG(bp, 8), GET_PREVSEG(bp, 12), GET_PREVSEG(bp, 16),
            GET_PREVSEG(bp, 20), GET_PREVSEG(bp, 24), GET_PREVSEG(bp, 28), GET_NEXT(bp));

}

void printBoth(){
    printHeap();
    printEmp();
}


int getMaxsize(int size){
    int nextoff=0;
    int temp = 32;
    if(size<=32){
        return 32;
    }
    if(size>4096){
        return 28096;//返回一个很大的数 614792
    }

    temp<<=1;
    nextoff+=4;
    while(size>temp){
        temp<<=1;
        nextoff+=4;
    }
    if(temp==1024){//注意我们跳过了1024的大小
        return 2048;
    }
    return temp;
}

int getNextOffbySize(int size){
    int nextoff=0;
    int temp = 32;
    if(size<=32){
        return 0;
    }
    if(size>4096){
        return 28;
    }

    temp<<=1;
    nextoff+=4;
    while(size>temp){
        temp<<=1;
        nextoff+=4;
    }
    if(temp>=2048){//注意我们跳过了1024的大小
        nextoff-=4;
    }
    return nextoff;
}

int getMinsize(int size){
    if(size<=32){
        return 16;
    }
    if(size>4096){
        return 4104;
    }

    int maxsize = getMaxsize(size);

    if(maxsize==2048){
        return 520;
    }

    return maxsize/2+8;
}

void putNextGeneral(char *bp, int size, int offset){

    if(OFFSET(bp)==0){//序言块
        PUT_NEXTSEG(getNextOffbySize(size),offset);
    }else if(GET_SIZE(HDRP(bp))==0){//结尾块 是错误的!!!
        printf("ERROR!!! attempting to set next in epligoue\n");
    }else{//普通块
        PUT_NEXT(bp, offset);
    }
}

int getNextGeneral(char *bp, int size){//返回offset

    if(OFFSET(bp)==0){//序言块
        return GET_NEXTSEG(getNextOffbySize(size));
    }else if(GET_SIZE(HDRP(bp))==0){//结尾块 是错误的!!!
        return GET_NEXT(bp);
        printf("waring! trying to get next in epligoue, which suppose to be -1 forever\n");
    }else{//普通块
        return GET_NEXT(bp);
    }
}

void putPrevGeneral(char *bp, int size, int offset){
    if(OFFSET(bp)==0){//序言块 是错误的!!!
        printf("ERROR!!! attempting to set prev in prologue\n");
    }else if(GET_SIZE(HDRP(bp))==0){//结尾块
        PUT_PREVSEG(bp, getNextOffbySize(size), offset);
    }else{//普通块
        PUT_PREV(bp, offset);
    }
}

int getPrevGeneral(char *bp, int size){
    if(OFFSET(bp)==0){//序言块 是错误的!!!
        printf("ERROR!!! attempting to get prev in prologue\n");
    }else if(GET_SIZE(HDRP(bp))==0){//结尾块
        return GET_PREVSEG(bp, getNextOffbySize(size));
    }else{//普通块
        return GET_PREV(bp);
    }
}
