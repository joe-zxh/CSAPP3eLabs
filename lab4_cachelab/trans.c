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
void normalCase(int M, int N, int A[N][M], int B[M][N]);
void dealWith61x67(int M, int N, int A[N][M], int B[M][N]);
void dealWith64(int M, int N, int A[N][M], int B[M][N]);
void dealWith32(int M, int N, int A[N][M], int B[M][N]);
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
    if(M==32 && N==32){
        dealWith32(M,N,A,B);
    }else if(M==64 && M==64){
        dealWith64(M,N,A,B);
    }else if(M==61&&N==67){
        dealWith61x67(M,N,A,B);
    }else{
        normalCase(M,N,A,B);
    }
}

void normalCase(int M, int N, int A[N][M], int B[M][N]){
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }
}

void dealWith61x67(int M, int N, int A[N][M], int B[M][N]){
    int i,j,m,n;
    int t0;
    //int t1,t2,t3,t4,t5,t6,t7;

    int size = 16;
    for(i = 0;i+size<67;i+=size) {

        for(j=0;j+8<65;j+=8){

            if(j+8<61){
                t0=j+8;
            }else{
                t0=61;
            };

            for(m=i;m<i+size;m++){
                for(n=j;n<t0;n++){
                    B[n][m]=A[m][n];
                }
            }
        }
    }

    for(j=0;j+8<65;j+=8){

        if(j+8<61){
            t0=j+8;
        }else{
            t0=61;
        };

        for(m=64;m<67;m++){
            for(n=j;n<t0;n++){
                B[n][m]=A[m][n];
            }
        }
    }
}

void dealWith64(int M, int N, int A[N][M], int B[M][N]){
    int i,j,m,n;
    int t0,t1,t2,t3,t4,t5,t6,t7;
    for(i = 0;i<8;i++){
        for (j=0;j<8;j++){
            if(i==j){
                //printMatrixB(M,N,A,B);

                // 对角线的8*8的情况
                //trans8by8(M,N,A,B,i,j);

                B[i*8+3][j*8+1]=A[j*8+1][i*8+3];

                for(m=0;m<4;m++){
                    //(i*8+m*2,j*8+n*2)    (i*8+m*2,j*8+n*2+1)
                    //(i*8+m*2+1, j*8+n*2) (i*8+m*2+1,j*8+n*2+1)

                    //step0: 复制第1个不冲突的
                    n = (m+3)%4;//第k个框  等下可以用n来替代

                    B[j*8+n*2][i*8+m*2]=A[i*8+m*2][j*8+n*2];
                    B[j*8+n*2+1][i*8+m*2]=A[i*8+m*2][j*8+n*2+1];
                    B[j*8+n*2][i*8+m*2+1]=A[i*8+m*2+1][j*8+n*2];
                    B[j*8+n*2+1][i*8+m*2+1]=A[i*8+m*2+1][j*8+n*2+1];

                    //step1: 复制第2个不冲突的
                    n = (n+2)%4;
                    B[j*8+n*2][i*8+m*2]=A[i*8+m*2][j*8+n*2];
                    B[j*8+n*2+1][i*8+m*2]=A[i*8+m*2][j*8+n*2+1];
                    B[j*8+n*2][i*8+m*2+1]=A[i*8+m*2+1][j*8+n*2];
                    B[j*8+n*2+1][i*8+m*2+1]=A[i*8+m*2+1][j*8+n*2+1];


                    //step2: 保存第1个冲突的到临时变量
                    n = (n+1)%4;
                    t0=A[i*8+m*2][j*8+n*2];
                    t1=A[i*8+m*2][j*8+n*2+1];
                    t2=A[i*8+m*2+1][j*8+n*2];
                    t3=A[i*8+m*2+1][j*8+n*2+1];

                    //step3:保存第2个冲突的到临时变量
                    n = (n+2)%4;
                    t4=A[i*8+m*2][j*8+n*2];
                    t5=A[i*8+m*2][j*8+n*2+1];
                    t6=A[i*8+m*2+1][j*8+n*2];
                    t7=A[i*8+m*2+1][j*8+n*2+1];

                    //step4:复制第1个冲突的
                    n = (n+2)%4;
                    B[j*8+n*2][i*8+m*2]=t0;
                    B[j*8+n*2+1][i*8+m*2]=t1;
                    B[j*8+n*2][i*8+m*2+1]=t2;
                    B[j*8+n*2+1][i*8+m*2+1]=t3;

                    //step4:复制第2个冲突的
                    n = (n+2)%4;
                    B[j*8+n*2][i*8+m*2]=t4;
                    B[j*8+n*2+1][i*8+m*2]=t5;
                    B[j*8+n*2][i*8+m*2+1]=t6;
                    B[j*8+n*2+1][i*8+m*2+1]=t7;
                }


            }else{
                //非对角线的情况
                //A:
                //(i*8, j*8)    (i*8, j*8+7)
                //
                //(i*8+7, j*8)  (i*8+7, j*8+7)

                //B:
                //(j*8, i*8)    (j*8, i*8+7)
                //(j*8+7, i*8) (j*8+7, i*8+7)


                //step1: A的左上到B的左上 8misses
                for(m=i*8;m<i*8+4;m++){
                    for(n=j*8;n<j*8+4;n++){
                        B[n][m] = A[m][n];
                    }
                }

                t0=A[i*8][j*8+4];
                t1=A[i*8][j*8+5];
                t2=A[i*8][j*8+6];
                t3=A[i*8][j*8+7];

                t4=A[i*8+1][j*8+4];
                t5=A[i*8+1][j*8+5];
                t6=A[i*8+1][j*8+6];
                t7=A[i*8+1][j*8+7];

                //step2: A的左下到B的右上 4misses
                for(m=i*8+4;m<i*8+8;m++){
                    for(n=j*8;n<j*8+4;n++){
                        B[n][m] = A[m][n];
                    }
                }


                //step3: A的右下到B的右下 4misses
                for(m=i*8+4;m<i*8+8;m++){
                    for(n=j*8+4;n<j*8+8;n++){
                        B[n][m] = A[m][n];
                    }
                }

                //step4: A的右上到B的左下 4misses
                for(m=i*8+2;m<i*8+4;m++){
                    for(n=j*8+4;n<j*8+8;n++){
                        B[n][m] = A[m][n];
                    }
                }
                B[j*8+4][i*8]=t0;
                B[j*8+5][i*8]=t1;
                B[j*8+6][i*8]=t2;
                B[j*8+7][i*8]=t3;

                B[j*8+4][i*8+1]=t4;
                B[j*8+5][i*8+1]=t5;
                B[j*8+6][i*8+1]=t6;
                B[j*8+7][i*8+1]=t7;
            }
        }
    }

}

void dealWith32(int M, int N, int A[N][M], int B[M][N]){
    int i,j,m,n;
    int t0,t1,t2,t3,t4,t5,t6,t7;
    //先处理不是对角线的情况。
    for(i = 0;i<4;i++){
        for (j=0;j<4;j++){
            if(i==j){
                //step1: A的右上到 B的左下，A的左上到B的右下。8个miss(A:4miss  B:4miss)
                for(m=4;m<8;m++){
                    for(n=0;n<4;n++){
                        B[i*8+m][j*8+n] = A[j*8+n][i*8+m];//填充B的左下的4*4
                    }
                    for(n=4;n<8;n++){
                        B[i*8+m][j*8+n] = A[i*8+m-4][j*8+n-4];//填充B的右下的4*4
                    }
                }

                //step2：B的右下到B的左上 (B:4miss)
                for(m=0;m<4;m++){
                    for(n=0;n<4;n++){
                        B[i*8+m][j*8+n] = B[j*8+4+n][i*8+4+m];//填充B的左上的4*4
                    }
                }

                //step3：A的左下到B的右上  在local里面记录A的右下的 前2行的值  (A:4miss)
                for(m=0;m<4;m++){
                    for(n=4;n<8;n++){
                        B[i*8+m][j*8+n] = A[j*8+n][i*8+m];//填充B的右上的4*4
                    }
                }
                t0=A[i*8+4][j*8+4];
                t1=A[i*8+4][j*8+5];
                t2=A[i*8+4][j*8+6];
                t3=A[i*8+4][j*8+7];

                t4=A[i*8+5][j*8+4];
                t5=A[i*8+5][j*8+5];
                t6=A[i*8+5][j*8+6];
                t7=A[i*8+5][j*8+7];

                //printMatrixB(M,N,A,B);

                //step4: A的右下的 后2行的值 存到B的右下的前2行中(B:2miss)
                for(m=i*8+4;m<i*8+6;m++){
                    for(n=j*8+4;n<i*8+8;n++){
                        B[m][n] = A[m+2][n];
                    }
                }

                //step5: A的右下的 4x4的内部操作 并且使用上前面的变量值(B:2miss)
                B[i*8+6][j*8+6]=B[i*8+4][j*8+6];
                B[i*8+6][j*8+7]=B[i*8+5][j*8+6];
                B[i*8+7][j*8+6]=B[i*8+4][j*8+7];
                B[i*8+7][j*8+7]=B[i*8+5][j*8+7];
                //这4个操作必须在下面4个操作之前，否则会被覆盖。

                B[i*8+4][j*8+6]=B[i*8+4][j*8+4];
                B[i*8+4][j*8+7]=B[i*8+5][j*8+4];
                B[i*8+5][j*8+6]=B[i*8+4][i*8+5];
                B[i*8+5][j*8+7]=B[i*8+5][i*8+5];

                B[i*8+4][j*8+4]=t0;
                B[i*8+4][j*8+5]=t4;
                B[i*8+5][j*8+4]=t1;
                B[i*8+5][j*8+5]=t5;

                B[i*8+6][j*8+4]=t2;
                B[i*8+6][j*8+5]=t6;
                B[i*8+7][j*8+4]=t3;
                B[i*8+7][j*8+5]=t7;

            }else{
                //非对角线的情况
                for(m=0;m<8;m++){
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


/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */ 

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
//char trans_desc[] = "Simple row-wise scan transpose";
//void trans(int M, int N, int A[N][M], int B[M][N])
//{
////    int i, j, tmp;
////
////    for (i = 0; i < N; i++) {
////        for (j = 0; j < M; j++) {
////            tmp = A[i][j];
////            B[j][i] = tmp;
////        }
////    }
//
//}

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
    //registerTransFunction(trans, trans_desc);

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

