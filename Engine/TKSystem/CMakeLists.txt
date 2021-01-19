file(GLOB TKCore_Root_Cpp
        *.cpp)
file(GLOB TKCore_Root_Hpp
        *.hpp)
file(GLOB TKCore_Root_H
        *.h)

add_library(CECore STATIC ${TKCore_Root_Cpp} ${TKCore_Root_Hpp} ${TKCore_Root_H})
message("CECore: Enter TKCore CmakeLists.txt")

add_definitions(-DNF_NONCLIENT_BUILD)
set_target_properties( TKCore PROPERTIES OUTPUT_NAME_DEBUG "TKCore")
set_target_properties( TKCore PROPERTIES PREFIX "")
set_target_properties( TKCore PROPERTIES
        FOLDER "Engine/TKCore"
        ARCHIVE_OUTPUT_DIRECTORY ${CEOutPutDir}
        RUNTIME_OUTPUT_DIRECTORY ${CEOutPutDir}
        LIBRARY_OUTPUT_DIRECTORY ${CEOutPutDir} )
