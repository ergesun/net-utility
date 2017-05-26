aux_source_directory(. NET_ROOT_SRCS)
aux_source_directory(net-protocal-stacks/msg-worker-policies NPS_MWP_SRCS)
aux_source_directory(net-protocal-stacks/msg-worker-policies NPS_MWP_SRCS)
aux_source_directory(net-protocal-stacks/nonblocking NPS_NB_SRCS)
aux_source_directory(net-protocal-stacks/nonblocking/socket-api NPS_NB_SA_SRCS)
aux_source_directory(net-protocal-stacks/nonblocking/socket-api/posix NPS_NB_SA_POSIX_SRCS)
aux_source_directory(net-protocal-stacks/nonblocking/socket-api/posix/tcp NPS_NB_SA_POSIX_TCP_SRCS)
aux_source_directory(net-protocal-stacks/nonblocking/socket-api/posix/tcp/stack NPS_NB_SA_POSIX_TCP_SK_SRCS)
aux_source_directory(net-protocal-stacks/nonblocking/event-drivers/epoll NPS_NB_ED_EP_SRCS)

add_library(net SHARED
        ${NET_ROOT_SRCS}
        ${NPS_MWP_SRCS}
        ${NPS_NB_SRCS}
        ${NPS_NB_SA_SRCS}
        ${NPS_NB_SA_POSIX_SRCS}
        ${NPS_NB_SA_POSIX_TCP_SRCS}
        ${NPS_NB_SA_POSIX_TCP_SK_SRCS}
        ${NPS_NB_ED_EP_SRCS}
        common-def.h
        idispatcher.h
        net-protocal-stacks/nonblocking/ievent-handler.h
        net-protocal-stacks/inet-msg-worker-manager.h
        net-protocal-stacks/nonblocking/event-drivers/ievent-driver.h
        message.h
        isocket-service.h
        abstract-socket-service.h
        iencoder.h
        )

