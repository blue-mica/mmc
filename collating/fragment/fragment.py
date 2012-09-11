import platform
import os
from ctypes import *


class FileType:
    FT_JPG = 0
    FT_PNG = 1
    FT_DOC = 2
    FT_XLS = 3
    FT_PDF = 4
    FT_H264 = 5
    FT_MP3 = 6
    FT_ZIP = 7
    FT_RAR = 8
    FT_TXT = 9
    FT_HTML = 10
    FT_XML = 11
    FT_UNKNOWN = 12
    FT_HIGH_ENTROPY = 13
    FT_LOW_ENTROPY = 14
    FT_VIDEO = 15
    FT_IMAGE = 16


class ClassifyT(Structure):
    _fields_ = [
            ("mType", c_uint),
            ("mStrength", c_int),
            ("mIsHeader", c_int),
            # 256 corresponds to MAX_STR_LENGTH in fragment_classifier.h
            ("mInfo", c_char * 256),
            ]

# 24 corresponds to the number of filetypes to be searched for
# this number is estimated to be enough (no more than 24 file types
# searched for at the same time)
ClassifyTArray = ClassifyT * 24


class CBlockOptions(Structure):
    pass


class CBlockOptions(Structure):
    _fields_ = [
            ("mOption1", c_char_p),
            ("mOption2", c_char_p),
            ("mOption3", c_char_p),
            ("mOption4", c_char_p),
            ("mOption5", c_char_p),
            ("mSubOptions", POINTER(CBlockOptions)),
            ]


CBlockOptionsPointer = POINTER(CBlockOptions)


class CClassifyHandler(Structure):
    _fields_ = []


CClassifyHandlerPointer = POINTER(CClassifyHandler)


class CFragmentStruct(Structure):
    _fields_ = [
            ("mOffset", c_ulonglong),
            ("mSize", c_ulonglong),
            ("mIsHeader", c_int),
            ("mIsFooter", c_int),
            ("mPicBegin", c_char_p),
            ("mPicEnd", c_char_p),
            ("mIsSmall", c_int),
            ("mIdxDecode", c_int),
            ("mIdxFile", c_int)
            ]

    def __str__(self):
        lString = str(self.mOffset) + " / " + str(self.mSize)
        if self.mIsHeader:
            lString += " | Header"
        if self.mIsFooter:
            lString += " | Footer"
        return lString


CFragmentStructPointer = POINTER(CFragmentStruct)


class CFragmentCollection(Structure):
    _fields_ = [("mNumFrags", c_ulonglong),
            ("mMaxFrags", c_ulonglong),
            ("mFrags", CFragmentStructPointer)]


CFragmentCollectionPointer = POINTER(CFragmentCollection)


class CFragments(object):

    def __init__(self, pCollection, pDestructor):
        super(CFragments, self).__init__()
        self.__mCollection = pCollection
        self.__mDestructor = pDestructor

    def __getitem__(self, pKey):
        return self.__mCollection.contents.mFrags[pKey]

    def __len__(self):
        return self.__mCollection.contents.mNumFrags

    def __iter__(self):
        for lCnt in xrange(self.__mCollection.contents.mNumFrags):
            yield self.__mCollection.contents.mFrags[lCnt]

    def __del__(self):
        if self.__mCollection is not None:
            self.__mDestructor(self.__mCollection)
        self.__mCollection = None


class CFragmentClassifier(object):

    def __init__(self):
        super(CFragmentClassifier, self).__init__()

        # load library (do not change)
        if platform.system().lower() == "windows":
            self._mLH = cdll.LoadLibrary("libblock_classifier.dll")
        elif platform.system().lower() == "linux":
            self._mLH = cdll.LoadLibrary("libblock_classifier.so")

        self._mClassify = self._mLH.classify_tsk
        self._mClassify.restype = CFragmentCollectionPointer
        self._mClassify.argtypes = \
            [c_ulonglong, c_uint, c_int, c_char_p, c_ulonglong,
            ClassifyTArray, c_int, c_ulonglong, c_ulonglong,
            c_char_p, c_int]

        self._mClassifyFree = self._mLH.classify_nofs_free
        self._mClassifyFree.restype = None
        self._mClassifyFree.argtypes = \
            [CFragmentCollectionPointer]

    def classify(self, pImageSize, pBlockSize, pNumBlocks, pImage,
            pOffset, pTypes, pBlockGap, pMinFragSize, pNumThreads):

        lCnt = 0
        lTypes = ClassifyTArray()
        for lType in pTypes:
            lTypes[lCnt].mType = lType['mType']
            lTypes[lCnt].mStrength = lType['mStrength']
            lCnt += 1

        return CFragments(self._mClassify(
                    pImageSize, pBlockSize, pNumBlocks,
                    pImage, pOffset, lTypes, lCnt, pBlockGap,
                    pMinFragSize,
                    os.path.join(
                        "collating",
                        "fragment",
                        "data",
                        "magic",
                        "media.mgc"),
                    pNumThreads
                ), self._mClassifyFree)
