file(GLOB_RECURSE
    ALL_CXX_SOURCE_FILES
    core/*.[ch]pp
    core/*.[ch]
    node/*.[ch]pp
    node/*.[ch]
    test/*.[ch]pp
    test/*.[ch]
    )

# Adding clang-format target if executable is found
if(NOT CLANG_FORMAT_BIN)
  # Find particular version
  find_program(CLANG_FORMAT_BIN NAMES clang-format-${RECOMMENDED_CLANG_FORMAT_VERSION})
  if(NOT CLANG_FORMAT_BIN)
    # Find default version and check it
    find_program(CLANG_FORMAT_BIN NAMES clang-format)
    if(CLANG_FORMAT_BIN)
      execute_process(
          COMMAND ${CLANG_FORMAT_BIN} --version
          COMMAND sed -r "s/.*version \([[:digit:]]+\).*/\\1/"
          OUTPUT_VARIABLE DEFAULT_CLANG_FORMAT_VERSION
      )
      math(EXPR DEFAULT_CLANG_FORMAT_VERSION "0 + 0${DEFAULT_CLANG_FORMAT_VERSION}")
      # Try to find newest version
      foreach(CLANG_FORMAT_VERSION RANGE ${RECOMMENDED_CLANG_FORMAT_VERSION} ${DEFAULT_CLANG_FORMAT_VERSION} -1)
        find_program(CLANG_FORMAT_BIN_ NAMES clang-format-${CLANG_FORMAT_VERSION})
        if(CLANG_FORMAT_BIN_)
          message(STATUS "Found clang-format version ${CLANG_FORMAT_VERSION}")
          set(CLANG_FORMAT_BIN ${CLANG_FORMAT_BIN_})
          break()
        endif()
      endforeach()
    endif()
  endif()
endif()

if(CLANG_FORMAT_BIN)
  execute_process(
      COMMAND ${CLANG_FORMAT_BIN} --version
      COMMAND sed -r "s/.*version \([[:digit:]]+\).*/\\1/"
      OUTPUT_VARIABLE CLANG_FORMAT_VERSION
  )
  math(EXPR CLANG_FORMAT_VERSION "0 + 0${CLANG_FORMAT_VERSION}")
  if(NOT ${CLANG_FORMAT_VERSION} EQUAL ${RECOMMENDED_CLANG_FORMAT_VERSION})
    message(WARNING "Found clang-format version ${CLANG_FORMAT_VERSION}, "
        "but version ${RECOMMENDED_CLANG_FORMAT_VERSION} is recommended")
  endif()

  message(STATUS "Target clang-format is enabled")
  add_custom_target(
      clang-format
      COMMAND "${CLANG_FORMAT_BIN}"
      -i
      ${ALL_CXX_SOURCE_FILES}
  )
endif()
