aux_source_directory(. SOURCE_FILES)

add_executable(net_test ${SOURCE_FILES}
        itest.h)

# 顺序不可以反了，net依赖于common
add_dependencies(net_test net common)
target_link_libraries(net_test net common pthread)
