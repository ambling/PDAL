#
# SQLite plugin CMake configuration
#

include (${PDAL_CMAKE_DIR}/sqlite.cmake)
include (${PDAL_CMAKE_DIR}/spatialite.cmake)

# SQLite Reader
#

PDAL_ADD_PLUGIN(reader_libname reader sqlite
    FILES
        io/SQLiteReader.cpp
    LINK_WITH ${SQLITE3_LIBRARY})
target_include_directories(${reader_libname} PRIVATE ${LIBXML2_INCLUDE_DIR})

#
# SQLite Writer
#

PDAL_ADD_PLUGIN(writer_libname writer sqlite
    FILES
        io/SQLiteWriter.cpp
    LINK_WITH
        ${SQLITE3_LIBRARY})
target_include_directories(${writer_libname} PRIVATE ${LIBXML2_INCLUDE_DIR})

#
# SQLite tests
#
if(BUILD_SQLITE_TESTS)
	PDAL_ADD_TEST(sqlitetest
        FILES test/SQLiteTest.cpp
        LINK_WITH ${reader_libname} ${writer_libname})
    target_include_directories(sqlitetest PRIVATE
        ${PDAL_IO_DIR}
        ${LIBXML2_INCLUDE_DIR})
endif()
