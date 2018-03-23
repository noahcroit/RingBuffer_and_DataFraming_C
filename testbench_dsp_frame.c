#include <stdio.h>
#include <stdlib.h>
#include "dsp_frame.h"
#include "circularBuffer.h"

#define     RING_LENGTH         8
#define     FRAME_SIZE          4
#define     OVERLAP_LENGTH      2
#define     BLOCKSIZE_PER_CALL  1

circularBuffer_TypeDef      myRingBuffer_1;
dspFrame_TypeDef            myFrame_1;
circularBuffer_TypeDef      myRingBuffer_2;
dspFrame_TypeDef            myFrame_2;
_RING_BUFFER_DATA_TYPE      p_myBuffer_1[RING_LENGTH];
_RING_BUFFER_DATA_TYPE      p_myFrame_1[FRAME_SIZE];
_RING_BUFFER_DATA_TYPE      p_myBuffer_2[RING_LENGTH];
_RING_BUFFER_DATA_TYPE      p_myFrame_2;
_RING_BUFFER_DATA_TYPE      userInput;

int main()
{
    int i=0;

    CircularBuffer_Init (&myRingBuffer_1,
                         p_myBuffer_1,
                         sizeof(_RING_BUFFER_DATA_TYPE),
                         RING_LENGTH);

    DSP_frameExtraction_Init(&myFrame_1,
                             p_myFrame_1,
                             sizeof(_RING_BUFFER_DATA_TYPE),
                             FRAME_SIZE,
                             OVERLAP_LENGTH);

    CircularBuffer_Init (&myRingBuffer_2,
                         p_myBuffer_2,
                         sizeof(_RING_BUFFER_DATA_TYPE),
                         RING_LENGTH);

    DSP_frameExtraction_Init(&myFrame_2,
                             &p_myFrame_2,
                             sizeof(_RING_BUFFER_DATA_TYPE),
                             BLOCKSIZE_PER_CALL,
                             0);


    while(1)
    {
        for(i=0; i<RING_LENGTH; i++)
        {
            printf("%d\t", p_myBuffer_1[i]);
        }
        printf("\nr=%d, f=%d\n", myRingBuffer_1.r, myRingBuffer_1.f);

        printf("enter enqueue value :");
        scanf("%d", &userInput);


        CircularBuffer_Enqueue (&myRingBuffer_2,
                                &userInput,
                                BLOCKSIZE_PER_CALL);

        if(DSP_frameExtraction_IsNextFrameReady(&myRingBuffer_2, &myFrame_2) == FRAME_IS_READY)
        {
            printf("sub ready! : ");
            printf("%d\t", p_myFrame_2);
            CircularBuffer_Enqueue (&myRingBuffer_1,
                                    &p_myFrame_2,
                                    BLOCKSIZE_PER_CALL);
        }
        else
        {

        }

        if(DSP_frameExtraction_IsNextFrameReady(&myRingBuffer_1, &myFrame_1) == FRAME_IS_READY)
        {
            printf("\nmain ready! :\t");
            for(i=0; i<FRAME_SIZE; i++)
            {
                printf("%d\t", p_myFrame_1[i]);
            }
        }
        else
        {

        }
        printf("\n\n");
    }
    return 0;
}
