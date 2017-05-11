aux_source_directory(. SRCS)
add_library(common SHARED ${SRCS}
        common-def.h
        thread-pool.h
        blocking-get-queue.h
        )
