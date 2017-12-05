#include <stdio.h>
#include "circularBuffer.h"

#define     SOURCE_BUFFER_SIZE      4
#define     SINK_BUFFER_SIZE        5

CircularBufferTypeDef myBuffer;
void testBench_fullTest();

_BUFFER_DATA_TYPE sourceBuffer[]    = {0, 0, 0, 0};
_BUFFER_DATA_TYPE sinkBuffer[]      = {0, 0, 0, 0, 0};

int main()
{
    testBench_fullTest();
    return 0;
}

void testBench_fullTest()
{
    static int i;

    CircularBuffer_Init(&myBuffer);
    myBuffer.rear = -1;
    myBuffer.front = -1;
    myBuffer.buffer[0] = 0; myBuffer.buffer[1] = 0; myBuffer.buffer[2] = 0; myBuffer.buffer[3] = 0; myBuffer.buffer[4] = 0; myBuffer.buffer[5] = 0; myBuffer.buffer[6] = 0; myBuffer.buffer[7] = 0;
    printf("myBuffer :\t");
    for(i=0; i<myBuffer.size; i++)
    {
        printf("%d\t", myBuffer.buffer[i]);
    }

    /* 1) Enqueue 1 2 3 4
                  5 6 7 8
    */
    sourceBuffer[0] = 1; sourceBuffer[1] = 2; sourceBuffer[2] = 3; sourceBuffer[3] = 4;
    CircularBuffer_Enqueue(&myBuffer, sourceBuffer, SOURCE_BUFFER_SIZE);
    printf("\n\nmyBuffer :\t");
    for(i=0; i<myBuffer.size; i++)
    {
        printf("%d\t", myBuffer.buffer[i]);
    }
    sourceBuffer[0] = 5; sourceBuffer[1] = 6; sourceBuffer[2] = 7; sourceBuffer[3] = 8;
    CircularBuffer_Enqueue(&myBuffer, sourceBuffer, SOURCE_BUFFER_SIZE);
    printf("\n\nmyBuffer :\t");
    for(i=0; i<myBuffer.size; i++)
    {
        printf("%d\t", myBuffer.buffer[i]);
    }

    /* 2) Dequeue with size = 5 */
    CircularBuffer_Dequeue(&myBuffer, sinkBuffer, SINK_BUFFER_SIZE);
    printf("\n\nmyBuffer :\t");
    for(i=0; i<myBuffer.size; i++)
    {
        printf("%d\t", myBuffer.buffer[i]);
    }

    /* 3) Enqueue 9 10 11 12 */
    sourceBuffer[0] = 9; sourceBuffer[1] = 10; sourceBuffer[2] = 11; sourceBuffer[3] = 12;
    CircularBuffer_Enqueue(&myBuffer, sourceBuffer, SOURCE_BUFFER_SIZE);
    printf("\n\nmyBuffer :\t");
    for(i=0; i<myBuffer.size; i++)
    {
        printf("%d\t", myBuffer.buffer[i]);
    }

    /* 4) Dequeue with size = 5 again */
    CircularBuffer_Dequeue(&myBuffer, sinkBuffer, SINK_BUFFER_SIZE);
    printf("\n\nmyBuffer :\t");
    for(i=0; i<myBuffer.size; i++)
    {
        printf("%d\t", myBuffer.buffer[i]);
    }

    printf("\n\nFinal buffer properties\n");
    printf("rear = %d\tfront = %d\n", myBuffer.rear, myBuffer.front);
    printf("size = %d\n", myBuffer.size);
}
