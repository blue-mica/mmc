# Set up the building environment as described in: 
# http://wiki.cython.org/64BitCythonExtensionsOnWindows
# C:\Program Files\Microsoft SDKs\Windows\v7.0>set DISTUTILS_USE_SDK=1
# C:\Program Files\Microsoft SDKs\Windows\v7.0>setenv /x64 /release

LIBFRAGMENT_CLASSIFIER=libfragment_classifier
FRAGMENT_CONTEXT=fragment_context

all: $(FRAGMENT_CONTEXT)
    move build\lib.win32-2.7\fragment_context.pyd . 
    copy lib\zlib\dll32\zlib1.dll .

$(FRAGMENT_CONTEXT): $(LIBFRAGMENT_CLASSIFIER).dll
    python setup.py build_ext 

$(LIBFRAGMENT_CLASSIFIER).dll: 
    cl /c src\fragment_classifier.c /Iinclude /I. 
    cl /c src\ncd.c /Iinclude /I. /Ilib\zlib
    link fragment_classifier.obj ncd.obj lib\zlib\dll32\zlib.lib /DLL /out:$(LIBFRAGMENT_CLASSIFIER).dll 
    
clean:
    del $(LIBFRAGMENT_CLASSIFIER).* *.obj *.pyd
    rmdir /S /Q build