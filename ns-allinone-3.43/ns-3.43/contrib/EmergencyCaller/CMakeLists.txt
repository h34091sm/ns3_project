check_include_file_cxx(stdint.h HAVE_STDINT_H)
if(HAVE_STDINT_H)
    add_definitions(-DHAVE_STDINT_H)
endif()

set(examples_as_tests_sources)
if(${ENABLE_EXAMPLES})
    set(examples_as_tests_sources
        #test/EmergencyCaller-examples-test-suite.cc
        )
endif()

build_lib(
    LIBNAME EmergencyCaller
    SOURCE_FILES model/EmergencyCaller.cc
                 helper/EmergencyCaller-helper.cc
    HEADER_FILES model/EmergencyCaller.h
                 helper/EmergencyCaller-helper.h
    LIBRARIES_TO_LINK 
    ${libcore}
    ${libnetwork}
    ${libinternet}
    ${libpointtopoint}
    ${libapplications}
    ${libdsdv}
    ${libmobility}
    ${libnetanim}

    TEST_SOURCES test/EmergencyCaller-test-suite.cc
                 ${examples_as_tests_sources}
)
