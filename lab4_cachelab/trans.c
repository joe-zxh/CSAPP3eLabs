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
    //printMatrixA(M,N,A,B);
    //int t0,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11;//最多12个 局部变量
    //if(M==32 && N==32)

    //先处理不是对角线的情况。
    for(int i = 0;i<4;i++){
        for (int j=0;j<4;j++){
            if(i==j){
                // 对角线的情况

                //切成2*2的。
                for(int x=0;x<4;x++){
                    for(int y=0;y<4;y++){
                        for (int m = 0;m<2;m++){
                            int row = i*8+x*2+m;
                            int col = j*8+y*2;
                            for(int n=0;n<2;n++){
                                B[col+n][row] = A[row][col+n];
                            }
                        }
                    }
                }

            }else{
                //非对角线的情况
                for(int m=0;m<8;m++){
                    int row=i*8+m;
                    int col = j*8;
                    for(int n=0;n<8;n++) {
                        B[col+n][row]=A[row][col+n];
                    }
                }
            }
        }
    }
    //printMatrixB(M,N,A,B);
    printf("breakpoint\n");
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
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }    

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

