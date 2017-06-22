aux_source_directory(. SOURCE_FILES)
aux_source_directory(tcp TCP_SRCS)
aux_source_directory(tcp/client TCP_CLIENT_SRCS)
aux_source_directory(tcp/server TCP_SERVER_SRCS)

add_executable(net_test ${TCP_SRCS} ${TCP_CLIENT_SRCS} ${TCP_SERVER_SRCS} ${SOURCE_FILES})

# 顺序不可以反了，net依赖于common
add_dependencies(net_test net common)
target_link_libraries(net_test net common pthread)
