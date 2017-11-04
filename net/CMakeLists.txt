aux_source_directory(. NET_ROOT_SRCS)
aux_source_directory(net-protocol-stacks/msg-worker-managers NPS_MWP_SRCS)
aux_source_directory(net-protocol-stacks/nonblocking NPS_NB_SRCS)
aux_source_directory(net-protocol-stacks/nonblocking/socket/network-api NPS_NB_SA_SRCS)
aux_source_directory(net-protocol-stacks/nonblocking/socket/network-api/posix NPS_NB_SA_POSIX_SRCS)
aux_source_directory(net-protocol-stacks/nonblocking/socket/network-api/posix/local NPS_NB_SA_POSIX_LOCAL_SRCS)
aux_source_directory(net-protocol-stacks/nonblocking/socket/network-api/posix/tcp NPS_NB_SA_POSIX_TCP_SRCS)
aux_source_directory(net-protocol-stacks/nonblocking/socket/network-api/posix/tcp/stack NPS_NB_SA_POSIX_TCP_SK_SRCS)
aux_source_directory(net-protocol-stacks/nonblocking/socket/event-drivers NPS_NB_SA_ED_SRCS)
aux_source_directory(net-protocol-stacks/nonblocking/socket/event-drivers/epoll NPS_NB_SA_ED_EP_SRCS)

add_library(net SHARED
        ${NET_ROOT_SRCS}
        ${NPS_MWP_SRCS}
        ${NPS_NB_SRCS}
        ${NPS_NB_SA_SRCS}
        ${NPS_NB_SA_POSIX_SRCS}
        ${NPS_NB_SA_POSIX_LOCAL_SRCS}
        ${NPS_NB_SA_POSIX_TCP_SRCS}
        ${NPS_NB_SA_POSIX_TCP_SK_SRCS}
        ${NPS_NB_SA_ED_SRCS}
        ${NPS_NB_SA_ED_EP_SRCS}
        )

