#include <stdlib.h>
#include <stdio.h>

#include "logging.h"
#include "fragment_collection.h"

static int sort_callback(const void* pFrag1, const void* pFrag2);
static int fragment_collection_add(fragment_collection_t* , 
        fragment_t* , unsigned long long, unsigned long long);

fragment_collection_t* fragment_collection_new(
        block_collection_t* pBlocks, 
        int pFactor,
        unsigned long long pOffset, 
        unsigned long long pBlockGap,
        unsigned long long pMinFragSize)
{
    unsigned long long lCntBlock = 0;
    storage_t lBlockTmp = 0;
    unsigned long long lBlockGap = 0;
    int lFlagExisting = 0;
    unsigned lBlockSize = block_collection_get_bs(pBlocks);
    fragment_t lFragTmp = { 0, 0, 0, 0, "", "", 0, -1 };

    fragment_collection_t* lFragments = 
        (fragment_collection_t* )malloc(sizeof(fragment_collection_t));

    lFragments->mNumFrags = 0;
    lFragments->mMaxFrags = block_collection_len(pBlocks) / pFactor;

    LOGGING_DEBUG("Max Frags: %lld\n", lFragments->mMaxFrags);
    LOGGING_DEBUG("Fragmentizer offset: %lld\n", pOffset);
    
    lFragments->mFrags = (fragment_t* )malloc(sizeof(fragment_t) * \
            lFragments->mMaxFrags);

    for(lCntBlock = 0; 
            lCntBlock < block_collection_len(pBlocks); 
            ++lCntBlock)
    {
        lBlockTmp = block_collection_get(pBlocks, lCntBlock);

        /* deterine if it is a relevant block */
        if (BLOCK(lBlockTmp))
        {
            /* determine if current block is a header block */
            if (HEADER(lBlockTmp))
            {
                /* temporary fragment is already existing */
                if (lFlagExisting)
                {
                    LOGGING_DEBUG("Start % 12lld, End: % 12lld, Size: % 12lld %s\n",
                            lFragTmp.mOffset,
                            lFragTmp.mOffset + lFragTmp.mSize,
                            lFragTmp.mSize,
                            lFragTmp.mIsHeader ? "| Header" : "");
                    if (fragment_collection_add(lFragments, &lFragTmp, 
                                pOffset, 
                                pMinFragSize * lBlockSize) != 0)
                    {
                        /* fragments buffer full => quit */
                        break;
                    }
                }
                /* reset lFragTmp to start a new fragment */
                /* lFragTmp = (fragment_t){ lBlockSize * lCntBlock, lBlockSize, -1, 1, 0, "", "", 0 }; */
                lFragTmp.mOffset = lBlockSize * lCntBlock;
                lFragTmp.mSize = lBlockSize;
                lFragTmp.mIsHeader = 1;
                lFragTmp.mIsFooter = 0;
                lFragTmp.mPicBegin = "";
                lFragTmp.mPicEnd = "";
                lFragTmp.mIsSmall = 0;
                lFragTmp.mIdxDecode = -1;
                lFragTmp.mIdxFile = -1;
                lBlockGap = 0;
            }
            else /* non-header block */
            {
                /* start new fragment or update current fragment depending on
                 * block gap being reached */
                if (lFlagExisting)
                {
                    if (lBlockGap <= pBlockGap * lBlockSize)
                    {
                        lFragTmp.mSize = lBlockSize * (lCntBlock + 1) - lFragTmp.mOffset;
                    }
                }
                else
                {
                    /* lFragTmp = (fragment_t){ lBlockSize * lCntBlock, lBlockSize, -1, 0, 0, "", "", 0 }; */
                    lFragTmp.mOffset = lBlockSize * lCntBlock;
                    lFragTmp.mSize = lBlockSize;
                    lFragTmp.mIsHeader = 0;
                    lFragTmp.mIsFooter = 0;
                    lFragTmp.mPicBegin = "";
                    lFragTmp.mPicEnd = "";
                    lFragTmp.mIsSmall = 0;
                    lFragTmp.mIdxDecode = -1;
                    lFragTmp.mIdxFile = -1;
                }
                lBlockGap = 0;
            }
            lFlagExisting = 1;
        }
        else
        {
            /* determine if block gap has been reached */
            /* if yes, add the fragment and set conditions to start a new one */
            if (lBlockGap > pBlockGap * lBlockSize)
            {
                LOGGING_DEBUG("Start % 12lld, End: % 12lld, Size: % 12lld %s\n",
                        lFragTmp.mOffset,
                        lFragTmp.mOffset + lFragTmp.mSize,
                        lFragTmp.mSize,
                        lFragTmp.mIsHeader ? "| Header" : "");
                if (fragment_collection_add(lFragments, &lFragTmp, 
                            pOffset, 
                            pMinFragSize * lBlockSize) != 0)
                {
                    /* fragments buffer full => quit */
                    break;
                }
                lBlockGap = 0;
                lFlagExisting = 0;
            }
            else if (lFlagExisting)
            {
                lBlockGap += lBlockSize;
            }
        }
    }

    if (lFlagExisting && lFragTmp.mSize > 0)
    {
        /* TODO check if buffer full */
        fragment_collection_add(lFragments, &lFragTmp, 
                pOffset, 
                pMinFragSize * lBlockSize);
    }


    /* sort fragments */
    qsort(lFragments->mFrags, lFragments->mNumFrags, sizeof(fragment_t),
            sort_callback);

    return lFragments;
}

static int sort_callback(const void* pFrag1, const void* pFrag2)
{
    if (((const fragment_t* )pFrag1)->mIsHeader &&
            !((const fragment_t* )pFrag2)->mIsHeader)
    {
        return -1;
    }
    else if (!((const fragment_t* )pFrag1)->mIsHeader &&
            ((const fragment_t* )pFrag2)->mIsHeader)
    {
        return 1;
    }

    return 0;
}

static int fragment_collection_add(fragment_collection_t* pFragments, 
        fragment_t* pFragment, 
        unsigned long long pOffset, 
        unsigned long long pMinFragSize)
{
    /* buffer full condition */
    if (pFragments->mNumFrags >= pFragments->mMaxFrags)
    {
        return -1;
    }
    /* fragment is too small */
    else if (pFragment->mSize <= pMinFragSize && !pFragment->mIsHeader)
    {
        return 0; /* this is not an error case */
    }

    /* add fragment to buffer */
    *(pFragments->mFrags + pFragments->mNumFrags) = *pFragment;
    (pFragments->mFrags + pFragments->mNumFrags)->mOffset += pOffset;
    (pFragments->mNumFrags)++;

    return 0;
}

void fragment_collection_free(
        fragment_collection_t* pCollection)
{
    free(pCollection->mFrags);
    free(pCollection);
}
