if(cppcheck_included)
    return()
endif()
set(cppcheck_included true)

if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    set(report_template "--template=vs")
else()
    set(report_template "--template=gcc")
endif()

find_program(
    CPP_CHECK_EXE
    NAMES "cppcheck"
    DOC "Path to cppcheck executable"
    )
if(NOT CPP_CHECK_EXE)
    message(WARNING "cppcheck not found.")
else()
    find_program(CMAKE_CXX_CPPCHECK NAMES cppcheck)
    message(STATUS "cppcheck found: ${CPP_CHECK_EXE}")
    list(
            APPEND CMAKE_CXX_CPPCHECK
                "--enable=warning,style,performance,portability,information,missingInclude"
                ${report_template}
                "--force"
        )
endif()
