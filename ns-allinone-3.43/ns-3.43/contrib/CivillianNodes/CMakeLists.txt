check_include_file_cxx(stdint.h HAVE_STDINT_H)
if(HAVE_STDINT_H)
    add_definitions(-DHAVE_STDINT_H)
endif()

set(examples_as_tests_sources)
if(${ENABLE_EXAMPLES})
    set(examples_as_tests_sources
        #test/CivillianNodes-examples-test-suite.cc
        )
endif()

build_lib(
    LIBNAME CivillianNodes
    SOURCE_FILES model/CivillianNodes.cc
    HEADER_FILES model/CivillianNodes.h

    LIBRARIES_TO_LINK 
    ${libcore}
    ${libnetwork}
    ${libinternet}
    ${libpointtopoint}
    ${libapplications}
    ${libdsdv}
    ${libmobility}
    ${libnetanim}

    TEST_SOURCES test/CivillianNodes-test-suite.cc
                 ${examples_as_tests_sources}
)
