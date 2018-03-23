/**
  * dsp_frame.h : 1-D signal frame extraction in C.
  *
  * Written in 2017 by Nattapong W. <https://github.com/jazzpiano1004>.
  *
  * To the extent possible under law, the author(s) have dedicated all
  * copyright and related and neighboring rights to this software to
  * the public domain worldwide. This software is distributed without
  * any warranty.
  *
  */

#ifndef  __DSP_FRAME_H
#define  __DSP_FRAME_H

#include "circularBuffer.h"
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/********************* Defines for frame data structure - start **********************/
#define     FRAME_SIZE_DEFAULT              256
#define     _FRAME_DATA_TYPE_DEFAULT        int32_t

typedef enum
{
		FIRST_FRAME_IS_NOT_COMPLETED = 0,
		FIRST_FRAME_IS_COMPLETED,
		FRAME_IS_NOT_READY,
		FRAME_IS_READY,
		FRAME_ERROR

}dspFrame_result;

typedef _FRAME_DATA_TYPE_DEFAULT   _FRAME_DATA_TYPE;
/*********************  Defines for frame data structure - end  **********************/

typedef struct
{
    void        *frame;
    int32_t     frameSize;      //frame size    (elements)
    int32_t     overlap;        //overlap size  (elements)
    int8_t      elementSize;    //size per element (bytes)
    uint8_t 	firstFrameCompleteFlag;      //1st frame flag
    void        *p_previousOverlap;          //pointer of previous overlap buffer (with allocated memory)

} dspFrame_TypeDef;

void    DSP_frameExtraction_Init(dspFrame_TypeDef *targetFrame,
                                 void *pFrame,
                                 int8_t SetElementSize,
                                 int32_t SetFrameSize,
                                 int32_t SetOverlap);

dspFrame_result  DSP_frameExtraction_IsNextFrameReady(circularBuffer_TypeDef *targetBuf, dspFrame_TypeDef *targetFrame);

#endif /* dsp_frame.h */
