include_directories(.. cmdline)

add_executable(beargen  beargen.cpp bearoff.cpp ../move.cpp)

add_custom_target(
        bearoff.built
        DEPENDS beargen
        COMMAND touch bearoff.built
        )

add_custom_command(
        OUTPUT bearoff.dat
        COMMAND ./beargen > bearoff.dat
        DEPENDS bearoff.built
        VERBATIM
        )

add_library(bearoff  bdata.cpp bearoff.cpp ${CMAKE_CURRENT_BINARY_DIR}/bearoff.dat)
target_include_directories(bearoff PUBLIC  ${CMAKE_CURRENT_BINARY_DIR})
