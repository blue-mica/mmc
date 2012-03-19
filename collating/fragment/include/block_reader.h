#ifndef __BLOCK_READER_H__
#define __BLOCK_READER_H__ 1

#include "fragment_classifier.h"
#include "block_collection.h"

int classify(block_collection_t* pBlocks, 
        int pBlockSize, 
        int pNumBlocks, 
        const char* pImage, 
        ClassifyT* pTypes, 
        int pNumTypes); 

#endif /* __BLOCK_READER_H__ */