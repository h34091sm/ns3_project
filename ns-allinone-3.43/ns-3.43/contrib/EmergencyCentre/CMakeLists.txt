check_include_file_cxx(stdint.h HAVE_STDINT_H)
if(HAVE_STDINT_H)
    add_definitions(-DHAVE_STDINT_H)
endif()


build_lib(
    LIBNAME EmergencyCentre
    SOURCE_FILES model/EmergencyCentre.cc
    HEADER_FILES model/EmergencyCentre.h
    LIBRARIES_TO_LINK 
    ${libcore}
    ${libnetwork}
    ${libinternet}
    ${libpointtopoint}
    ${libapplications}
    ${libdsdv}
    ${libmobility}
    ${libnetanim}

)
