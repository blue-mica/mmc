import platform
import sys
import os

from cx_Freeze import setup, Executable


def pathAdd(pList, pPath, pExtension):
    for lFile in os.listdir(pPath):
        lFilePath = os.path.join(pPath, lFile)
        if os.path.isfile(lFilePath) \
                and lFile.endswith(pExtension):
            pList.append(lFilePath)

include_files = []
lTargetIncludes = ["PySide.QtCore", "PySide.QtGui", "PySide.QtXml"]
lPlatform = platform.system().lower()
if lPlatform == "linux":
    include_files.append(("collating/fragment/libfragment_classifier.so", \
            "libfragment_classifier.so"))
    lTargetIncludes.append("magic")
elif lPlatform == "windows":
    lBits = 32
    if sys.maxsize > 2**32:
        lBits = 64
    lPath = r"collating\magic\lib\magic\dll" + str(lBits)
    include_files.append((lPath + r"\magic.dll", \
            lPath + r"\magic.dll"))
    if lBits == 64:
        include_files.append((lPath + r"\libgnurx-0.dll", \
                lPath + r"\libgnurx-0.dll"))
    else:
        include_files.append((lPath + r"\regex2.dll", \
                lPath + r"\regex2.dll"))
    pathAdd(include_files, "bin", ".exe")
    pathAdd(include_files, "bin", ".dll")

pathAdd(include_files, os.path.join('data', 'magic'), ".mgc")
pathAdd(include_files, 'data', ".img")

lTarget = Executable(
    script="gui/gui_fc.py",
    includes=lTargetIncludes
    )

setup(
    version="0.9.5",
    description="Multimedia File Carver",
    author="Rainer Poisel",
    author_email="rainer.poisel@fhstp.ac.at",
    name="mmc",
    options={"build_exe": {
                             "append_script_to_exe": True,
                             "create_shared_zip": False,
                             "include_files": include_files,
                            }
               },
    executables=[lTarget]
    )