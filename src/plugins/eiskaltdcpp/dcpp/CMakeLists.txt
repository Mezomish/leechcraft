project (dcpp)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
cmake_minimum_required (VERSION 2.6)

aux_source_directory(${PROJECT_SOURCE_DIR} dcpp_srcs)
file (GLOB dcpp_hdrs ${PROJECT_SOURCE_DIR}/*.h)
set (DCPP_SRCS @dcpp_srcs@ PARENT_SCOPE)
set (DCPP_HDRS @dcpp_hdrs@ PARENT_SCOPE)

if (NOT LUA_SCRIPT)
  list (REMOVE_ITEM dcpp_srcs ${CMAKE_CURRENT_SOURCE_DIR}/ScriptManager.cpp)
endif ()

include_directories (${PROJECT_SOURCE_DIR}/.. ${Boost_INCLUDE_DIR} ${BZIP2_INCLUDE_DIR} ${ZLIB_INCLUDE_DIR} ${OPENSSL_INCLUDE_DIR} ${GETTEXT_INCLUDE_DIR} ${LUA_INCLUDE_DIR} ${IDNA_INCLUDE_DIR})

if (WIN32)
  set (LINK STATIC)
  set (WIN32_LIBS "ws2_32")
else (WIN32)
  if (NOT CMAKE_CROSSCOMPILING)
    if (HAIKU)
        set (PTHREADS "root")
    else (HAIKU)
        set (PTHREADS "pthread")
    endif(HAIKU)
  endif (NOT CMAKE_CROSSCOMPILING)
endif (WIN32)

if (NOT LINK)
  set (LINK SHARED)
endif (NOT LINK)

if (APPLE)
  set (APPLE_LIBS "iconv")
endif (APPLE)

set (UPNP "extra")

add_library (dcpp STATIC ${dcpp_srcs})

if (USE_MINIUPNP)
    set_property(SOURCE ${PROJECT_SOURCE_DIR}/DCPlusPlus.cpp ${PROJECT_SOURCE_DIR}/UPnPManager.cpp  PROPERTY COMPILE_DEFINITIONS USE_MINIUPNP )
endif()

if (ICONV_SECOND_ARGUMENT_IS_CONST)
    set_property(SOURCE ${PROJECT_SOURCE_DIR}/Text.cpp PROPERTY COMPILE_DEFINITIONS ICONV_CONST=const)
endif (ICONV_SECOND_ARGUMENT_IS_CONST)

if (FORCE_XDG)
   set_property(SOURCE ${PROJECT_SOURCE_DIR}/Util.cpp PROPERTY COMPILE_DEFINITIONS FORCE_XDG )
endif (FORCE_XDG)

if (WIN32)
   set_property(TARGET dcpp PROPERTY COMPILE_FLAGS)
else(WIN32)
   set_property(TARGET dcpp PROPERTY COMPILE_FLAGS -fPIC)
endif (WIN32)

target_link_libraries (dcpp ${PTHREADS} ${BZIP2_LIBRARIES} ${ZLIB_LIBRARIES}
${OPENSSL_LIBRARIES} ${GETTEXT_LIBRARIES} ${ICONV_LIBRARIES} ${WIN32_LIBS} ${APPLE_LIBS} ${LUA_LIBRARIES} ${UPNP} ${PCRE} ${DHT_LIB} ${IDNA_LIBRARIES})

if (APPLE)
  set (LIBDIR1 .)
  set (LIBDIR ${PROJECT_NAME_GLOBAL}.app/Contents/MacOS)
endif(APPLE)

