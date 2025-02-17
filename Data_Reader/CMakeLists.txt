set(DATA_READER_SOURCES
    blf_parser/blfparser.cpp
    data/data.cpp
    data/data_dialog.cpp
    dbc_parser/dbciterator.cpp
    dbc_parser/message.cpp
    dbc_parser/signal.cpp
    csv_parser/csvparser.cpp
)

set(DATA_READER_HEADERS
    blf_parser/binlog.h
    blf_parser/binlog_objects.h
    blf_parser/blfparser.h
    data/data.h
    data/data_dialog.h
    data/data_types.h
    dbc_parser/dbciterator.hpp
    dbc_parser/message.hpp
    dbc_parser/signal.hpp
    csv_parser/csvparser.h
)

set(DATA_READER_FORMS
    "${CMAKE_CURRENT_SOURCE_DIR}/data/data_dialog.ui"
)

set(DATA_READER_RESOURCES
    datamanagerresources.qrc
)

add_library(Data_Reader STATIC
    ${DATA_READER_SOURCES}
    ${DATA_READER_HEADERS}
    ${DATA_READER_FORMS}
    ${DATA_READER_RESOURCES}
)


target_link_libraries(Data_Reader PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Concurrent
    ${CMAKE_CURRENT_SOURCE_DIR}/blf_parser/binlog.dll
)

target_compile_definitions(Data_Reader PRIVATE
    QT_DISABLE_DEPRECATED_BEFORE=0x065000
)

target_include_directories(Data_Reader PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}  # For generated UI headers
)

set_property(TARGET Data_Reader PROPERTY AUTOUIC_SEARCH_PATHS
    ${CMAKE_CURRENT_SOURCE_DIR}/data
) 