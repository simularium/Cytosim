set(SOURCES_BASE
    operator_new.cc
    node_list.cc
    messages.cc
    filewrapper.cc
    filepath.cc
    iowrapper.cc
    exceptions.cc
    tictoc.cc
    inventory.cc
    stream_func.cc
    tokenizer.cc
    glossary.cc
    property.cc
    property_list.cc
    backtrace.cc
    print_color.cc
)

list(TRANSFORM SOURCES_BASE PREPEND "${PROJECT_SOURCE_DIR}/src/base/")

set(BASE_INCLUDES math base)
list(TRANSFORM BASE_INCLUDES PREPEND "${PROJECT_SOURCE_DIR}/src/")


add_library(${BASE_LIBRARY} STATIC ${SOURCES_BASE})

set_source_files_properties(
    PROPERTIES
    EXTERNAL_OBJECT true
    GENERATED true
)

target_include_directories(${BASE_LIBRARY} PUBLIC "${BASE_INCLUDES}" )
