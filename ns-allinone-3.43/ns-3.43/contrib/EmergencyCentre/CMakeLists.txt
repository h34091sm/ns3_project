check_include_file_cxx(stdint.h HAVE_STDINT_H)
if(HAVE_STDINT_H)
    add_definitions(-DHAVE_STDINT_H)
endif()

set(examples_as_tests_sources)
if(${ENABLE_EXAMPLES})
    set(examples_as_tests_sources
        #test/EmergencyCentre-examples-test-suite.cc
        )
endif()

build_lib(
    LIBNAME EmergencyCentre
    SOURCE_FILES model/EmergencyCentre.cc
                 helper/EmergencyCentre-helper.cc
    HEADER_FILES model/EmergencyCentre.h
                 helper/EmergencyCentre-helper.h
    LIBRARIES_TO_LINK 
    ${libcore}
    ${libnetwork}
    ${libinternet}
    ${libpointtopoint}
    ${libapplications}
    ${libdsdv}
    ${libmobility}
    ${libnetanim}
    TEST_SOURCES test/EmergencyCentre-test-suite.cc
                 ${examples_as_tests_sources}
)
