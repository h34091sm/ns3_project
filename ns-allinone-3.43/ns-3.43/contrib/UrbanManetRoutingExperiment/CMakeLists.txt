check_include_file_cxx(stdint.h HAVE_STDINT_H)
if(HAVE_STDINT_H)
    add_definitions(-DHAVE_STDINT_H)
endif()


build_lib(
    LIBNAME UrbanManetRoutingExperiment
    SOURCE_FILES model/UrbanManetRoutingExperiment.cc
    HEADER_FILES model/UrbanManetRoutingExperiment.h
    LIBRARIES_TO_LINK 
    ${libcore}
    ${libnetwork}
    ${libapplications}
    ${libnetanim}
    ${libsocket}
    ${libaodv}
    ${libdsdv}
    ${libdsr}
    ${libolsr}
    ${libflowmonitor}
    ${libinternet}
    ${libmobility}
    ${libyanswifi}
    ${libcivillainnodes}
    ${libemergencycentre}
    ${libemergencyresponder}
    ${liburbanmanet}


)
