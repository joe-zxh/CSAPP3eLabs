/* 
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */ 
#include <stdio.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);
void trans8by8(int M, int N, int A[N][M], int B[M][N], int i, int j);

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    int i,j,m,n;

    //if(M==32 && N==32)

    //先处理不是对角线的情况。
    for(i = 0;i<M/8;i++){
        for (j=0;j<M/8;j++){
            if(i==j){
                // 对角线的8*8的情况
                trans8by8(M,N,A,B,i,j);
                //printMatrixB(M,N,A,B);
                //printf("breakpoint\n");
            }else{
                //非对角线的情况
                for(m=0;m<4;m++){
                    for(n=0;n<8;n++) {
                        B[j*8+n][i*8+m]=A[i*8+m][j*8+n];
                    }
                }
                for(m=4;m<8;m++){
                    for(n=0;n<8;n++) {
                        B[j*8+n][i*8+m]=A[i*8+m][j*8+n];
                    }
                }
            }
        }
    }
    //printMatrixB(M,N,A,B);
    //printf("breakpoint\n");
}


void trans8by8(int M, int N, int A[N][M], int B[M][N], int i, int j){
    int x, y, t0,t1,t2,t3;;

    //切成4*4的。

    //step1:8个miss(A:4miss  B:4miss)
    for(x=4;x<8;x++){
        for(y=0;y<4;y++){
            B[i*8+x][j*8+y] = A[j*8+y][i*8+x];//填充B的左下的4*4
        }
        for(y=4;y<8;y++){
            B[i*8+x][j*8+y] = A[i*8+x-4][j*8+y-4];//填充B的右下的4*4
        }
    }

    //printMatrixB(M,N,A,B);

    //step2:4个miss(B:4miss)
    for(x=0;x<4;x++){
        for(y=0;y<4;y++){
            B[i*8+x][j*8+y] = B[j*8+4+y][i*8+4+x];//填充B的左上的4*4
        }
    }

    //printMatrixB(M,N,A,B);

    //step3:4个miss(A:4miss)
    for(x=0;x<4;x++){
        for(y=4;y<8;y++){
            B[i*8+x][j*8+y] = A[j*8+y][i*8+x];//填充B的右上的4*4
        }
    }

    //printMatrixB(M,N,A,B);

    //step4: 现在还剩最后B的右下角的4*4不正确了
    //现在只剩11个miss能用了。

    //step4_1: 记录A的右下角2*2的元素(0 miss)
    t0 = A[i*8+6][j*8+6];
    t1 = A[i*8+6][j*8+7];
    t2 = A[i*8+7][j*8+6];
    t3 = A[i*8+7][j*8+7];

    //step4_2:填充B的左下角2*2的元素(2 miss)
    B[i*8+6][j*8+4]=A[j*8+4][i*8+6];
    B[i*8+6][j*8+5]=A[j*8+5][i*8+6];
    B[i*8+7][j*8+4]=A[j*8+4][i*8+7];
    B[i*8+7][j*8+5]=A[j*8+5][i*8+7];

    //printMatrixB(M,N,A,B);

    //step4_3:填充B的右下角的2*2的元素(0 miss)
    B[i*8+6][j*8+6]=t0;
    B[i*8+6][j*8+7]=t2;
    B[i*8+7][j*8+6]=t1;
    B[i*8+7][j*8+7]=t3;

    //printMatrixB(M,N,A,B);

    //step4_3:记录A的左上角2*2的元素(0 miss)
    t0 = A[i*8+4][j*8+4];
    t1 = A[i*8+4][j*8+5];
    t2 = A[i*8+5][j*8+4];
    t3 = A[i*8+5][j*8+5];

    //step4_4:填充B的右上角的2*2元素(2+2=4miss)
    B[i*8+4][j*8+6]=A[i*8+6][j*8+4];
    B[i*8+4][j*8+7]=A[i*8+7][j*8+4];
    B[i*8+5][j*8+6]=A[i*8+6][j*8+5];
    B[i*8+5][j*8+7]=A[i*8+7][j*8+5];

    //printMatrixB(M,N,A,B);

    //step4_5: 填充B的左上角的2*2元素(0 miss)
    B[i*8+4][j*8+4]=t0;
    B[i*8+4][j*8+5]=t2;
    B[i*8+5][j*8+4]=t1;
    B[i*8+5][j*8+5]=t3;

    //printMatrixB(M,N,A,B);
    //printf("breakpoint\n");
}


/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */ 

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
//    int i, j, tmp;
//
//    for (i = 0; i < N; i++) {
//        for (j = 0; j < M; j++) {
//            tmp = A[i][j];
//            B[j][i] = tmp;
//        }
//    }

}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc); 

    /* Register any additional transpose functions */
    registerTransFunction(trans, trans_desc); 

}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}

