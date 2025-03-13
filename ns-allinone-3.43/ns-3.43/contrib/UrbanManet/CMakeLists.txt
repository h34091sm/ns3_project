check_include_file_cxx(stdint.h HAVE_STDINT_H)
if(HAVE_STDINT_H)
    add_definitions(-DHAVE_STDINT_H)
endif()

set(examples_as_tests_sources)
if(${ENABLE_EXAMPLES})
    set(examples_as_tests_sources
        #test/UrbanManet-examples-test-suite.cc
        )
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



    TEST_SOURCES test/UrbanManet-test-suite.cc
                 ${examples_as_tests_sources}
)
