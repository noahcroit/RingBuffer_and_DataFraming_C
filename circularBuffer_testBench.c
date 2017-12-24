#include <stdio.h>
#include "circularBuffer.h"

#define     SOURCE_BUFFER_SIZE      4
#define     SINK_BUFFER_SIZE        5

#define     DEFAULT_FRAME_SIZE      4
#define     DEFAULT_FRAME_OVERLAP   2

CircularBufferTypeDef myBuffer;
void testBench_fullTest();
void frameBasedFunctionTest();

_BUFFER_DATA_TYPE sourceBuffer[4]    = {0, 0, 0, 0};
_BUFFER_DATA_TYPE sinkBuffer[5]      = {0, 0, 0, 0, 0};

int main()
{
    //testBench_fullTest();
    frameBasedFunctionTest();
    return 0;
}

void testBench_fullTest()
{
    static int i;

    //CircularBuffer_Init(&myBuffer, sizeof(_BUFFER_DATA_TYPE), DEFAULT_FRAME_SIZE, DEFAULT_FRAME_OVERLAP);
    CircularBuffer_Init(&myBuffer, sizeof(int32_t), DEFAULT_FRAME_SIZE, DEFAULT_FRAME_OVERLAP);
    //myBuffer.buffer[0] = 0; myBuffer.buffer[1] = 0; myBuffer.buffer[2] = 0; myBuffer.buffer[3] = 0; myBuffer.buffer[4] = 0; myBuffer.buffer[5] = 0; myBuffer.buffer[6] = 0; myBuffer.buffer[7] = 0;
    printf("myBuffer :\t");
    for(i=0; i<myBuffer.bufferSize; i++)
    {
        printf("%d\t", myBuffer.buf[i]);
    }

    /* 1) Enqueue 1 2 3 4
                  5 6 7 8
    */
    sourceBuffer[0] = 1; sourceBuffer[1] = 2; sourceBuffer[2] = 3; sourceBuffer[3] = 4;
    CircularBuffer_Enqueue(&myBuffer, sourceBuffer, SOURCE_BUFFER_SIZE);
    printf("\n\nmyBuffer :\t");
    for(i=0; i<myBuffer.bufferSize; i++)
    {
        printf("%d\t", myBuffer.buf[i]);
    }
    printf("\nrear = %d\tfront = %d\n", myBuffer.r, myBuffer.f);
    printf("size = %d\n", myBuffer.bufferSize);
    sourceBuffer[0] = 5; sourceBuffer[1] = 6; sourceBuffer[2] = 7; sourceBuffer[3] = 8;
    CircularBuffer_Enqueue(&myBuffer, sourceBuffer, SOURCE_BUFFER_SIZE);
    printf("\n\nmyBuffer :\t");
    for(i=0; i<myBuffer.bufferSize; i++)
    {
        printf("%d\t", myBuffer.buf[i]);
    }
    printf("\nrear = %d\tfront = %d\n", myBuffer.r, myBuffer.f);
    printf("size = %d\n", myBuffer.bufferSize);

    /* 2) Dequeue with size = 5 */
    CircularBuffer_Dequeue(&myBuffer, sinkBuffer, SINK_BUFFER_SIZE);
    printf("\n\nmyBuffer :\t");
    for(i=0; i<myBuffer.bufferSize; i++)
    {
        printf("%d\t", myBuffer.buf[i]);
    }
    printf("\nrear = %d\tfront = %d\n", myBuffer.r, myBuffer.f);
    printf("size = %d\n", myBuffer.bufferSize);

    /* 3) Enqueue 9 10 11 12 */
    sourceBuffer[0] = 9; sourceBuffer[1] = 10; sourceBuffer[2] = 11; sourceBuffer[3] = 12;
    CircularBuffer_Enqueue(&myBuffer, sourceBuffer, SOURCE_BUFFER_SIZE);
    printf("\n\nmyBuffer :\t");
    for(i=0; i<myBuffer.bufferSize; i++)
    {
        printf("%d\t", myBuffer.buf[i]);
    }
    printf("\nrear = %d\tfront = %d\n", myBuffer.r, myBuffer.f);
    printf("size = %d\n", myBuffer.bufferSize);

    /* 4) Dequeue with size = 5 again */
    CircularBuffer_Dequeue(&myBuffer, sinkBuffer, SINK_BUFFER_SIZE);
    printf("\n\nmyBuffer :\t");
    for(i=0; i<myBuffer.bufferSize; i++)
    {
        printf("%d\t", myBuffer.buf[i]);
    }

    printf("\n\nFinal buffer properties\n");
    printf("rear = %d\tfront = %d\n", myBuffer.r, myBuffer.f);
    printf("size = %d\n", myBuffer.bufferSize);
}
void frameBasedFunctionTest()
{
    _BUFFER_DATA_TYPE dataIn;
    //_BUFFER_DATA_TYPE *frame;
    _BUFFER_DATA_TYPE frame[DEFAULT_FRAME_SIZE];

    int i;
    int checkFrame;

    /* initialize buffer & frame */
    CircularBuffer_Init(&myBuffer, sizeof(_BUFFER_DATA_TYPE), DEFAULT_FRAME_SIZE, DEFAULT_FRAME_OVERLAP);
    //myBuffer.buffer[0] = 0; myBuffer.buffer[1] = 0; myBuffer.buffer[2] = 0; myBuffer.buffer[3] = 0; myBuffer.buffer[4] = 0; myBuffer.buffer[5] = 0; myBuffer.buffer[6] = 0; myBuffer.buffer[7] = 0;
    printf("myBuffer :\t");
    for(i=0; i<myBuffer.bufferSize; i++)
    {
        printf("%.2f\t", myBuffer.buf[i]);
    }

    /* frame-based routine start */
    while(1)
    {
        scanf("%f", (int *)&dataIn);
        CircularBuffer_Enqueue(&myBuffer, (void *)&dataIn, 1);
        checkFrame = CircularBuffer_IsNextFrameReady(&myBuffer, frame);
        printf("frame ready = %d\n", checkFrame);

        if(checkFrame == FRAME_IS_READY)
        {
            printf("frame :\t");
            for(i=0; i<DEFAULT_FRAME_SIZE; i++)
            {
                printf("%.2f\t", frame[i]);
            }
            printf("\n");
        }

        printf("myBuffer :\t");
        for(i=0; i<myBuffer.bufferSize; i++)
        {
            printf("%.2f\t", myBuffer.buf[i]);
        }
        printf("rear = %d\tfront = %d\n\n", myBuffer.r, myBuffer.f);
    }
}
