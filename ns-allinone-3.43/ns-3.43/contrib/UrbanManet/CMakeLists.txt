check_include_file_cxx(stdint.h HAVE_STDINT_H)
if(HAVE_STDINT_H)
    add_definitions(-DHAVE_STDINT_H)
endif()



build_lib(
    LIBNAME UrbanManet
    SOURCE_FILES model/UrbanManet.cc
    HEADER_FILES model/UrbanManet.h
    LIBRARIES_TO_LINK 
    ${libcore}
    ${libnetwork}
    ${libinternet}
    ${libapplications}
    ${libnetanim}
    ${libsocket}
    ${libdsdv}
    ${libolsr}
    ${libaodv}
    ${libdsr}
    ${libmobility}
    ${libyanswifihelper}
    ${libflowmonitor}



  
)
