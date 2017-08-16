aux_source_directory(. NET_ROOT_SRCS)
aux_source_directory(net-protocal-stacks/msg-worker-managers NPS_MWP_SRCS)
aux_source_directory(net-protocal-stacks/nonblocking NPS_NB_SRCS)
aux_source_directory(net-protocal-stacks/nonblocking/socket/network-api NPS_NB_SA_SRCS)
aux_source_directory(net-protocal-stacks/nonblocking/socket/network-api/posix NPS_NB_SA_POSIX_SRCS)
aux_source_directory(net-protocal-stacks/nonblocking/socket/network-api/posix/local NPS_NB_SA_POSIX_LOCAL_SRCS)
aux_source_directory(net-protocal-stacks/nonblocking/socket/network-api/posix/tcp NPS_NB_SA_POSIX_TCP_SRCS)
aux_source_directory(net-protocal-stacks/nonblocking/socket/network-api/posix/tcp/stack NPS_NB_SA_POSIX_TCP_SK_SRCS)
aux_source_directory(net-protocal-stacks/nonblocking/socket/event-drivers NPS_NB_SA_ED_SRCS)
aux_source_directory(net-protocal-stacks/nonblocking/socket/event-drivers/epoll NPS_NB_SA_ED_EP_SRCS)

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
        notify-message.h
        common-def.h
        net-protocal-stacks/nonblocking/socket/network-api/abstract-event-manager.h
        net-protocal-stacks/nonblocking/ievent-handler.h
        net-protocal-stacks/inet-stack-worker-manager.h
        net-protocal-stacks/nonblocking/socket/event-drivers/ievent-driver.h
        net-protocal-stacks/nonblocking/socket/event-drivers/factory.h
        net-protocal-stacks/nonblocking/socket/network-api/file-descriptor.h
        net-protocal-stacks/nonblocking/socket/network-api/abstract-file-event-handler.h
        message.h
        isocket-service.h
        abstract-socket-service.h
        iencoder.h
        net-protocal-stacks/nonblocking/socket/network-api/posix/tcp/connect-messages/connect-request-message.h
        net-protocal-stacks/nonblocking/socket/network-api/posix/tcp/connect-messages/connect-response-message.h
        )

