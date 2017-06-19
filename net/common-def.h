/**
 * This work copyright Chao Sun(qq:296449610) and licensed under
 * a Creative Commons Attribution 3.0 Unported License(https://creativecommons.org/licenses/by/3.0/).
 */

#ifndef NET_CORE_COMMON_DEF_H
#define NET_CORE_COMMON_DEF_H

#include <string>
#include <sstream>

#include "../common/hash-algorithms.h"

namespace netty {
    namespace net {
        enum class NettyMsgCode {
            OK           = 0,
            ErrLocal,
            ErrTimeout,
            ErrPeer
        };

        enum class NonBlockingEventModel {
            DPDK   = 0,
            Posix
        };

        enum class SocketProtocal {
            None = 0,
            Tcp,
            Udp
        };

        typedef struct net_addr_s {
            std::string addr;
            uint16_t port;

            net_addr_s() = default;

            net_addr_s(std::string &&a, uint16_t p) : addr(std::move(a)), port(p) {}

            net_addr_s(const net_addr_s &nas) {
                addr = nas.addr;
                port = nas.port;
            }

            net_addr_s &operator=(const net_addr_s &nas) {
                addr = nas.addr;
                port = nas.port;
                return *this;
            }

            net_addr_s(net_addr_s &&nas) {
                addr = std::move(nas.addr);
                port = nas.port;
            }

            net_addr_s &operator=(net_addr_s &&nas) {
                addr = std::move(nas.addr);
                port = nas.port;
                return *this;
            }
        } net_addr_t;

        typedef struct net_peer_info_s {
            net_addr_t nat;
            SocketProtocal sp;

            net_peer_info_s() {
                sp = SocketProtocal::None;
            }

            net_peer_info_s(net_addr_t &n, SocketProtocal s) : nat(n), sp(s) {}

            net_peer_info_s(net_addr_t &&n, SocketProtocal s) : nat(std::move(n)), sp(s) {}

            net_peer_info_s(std::string &&addr, uint16_t port, SocketProtocal s) {
                nat = net_addr_t(std::move(addr), port);
                sp = s;
            }

            net_peer_info_s(const net_peer_info_s &npis) {
                nat = npis.nat;
                sp = npis.sp;
            }
        } net_peer_info_t, net_local_info_t;

        inline bool operator==(const net_peer_info_t &a, const net_peer_info_t &b) {
            return a.sp == b.sp && a.nat.port == b.nat.port && (0 == a.nat.addr.compare(b.nat.addr));
        }
    } // namespace net
} // namespace netty

// declare hash<net_peer_info_t>
namespace std {
    template<>
    struct hash<netty::net::net_peer_info_t> {
        size_t operator()(const netty::net::net_peer_info_t &npit) const {
            std::stringstream ss;
            ss << npit.nat.addr << ":" << npit.nat.port << "-" << (int) (npit.sp);
            auto key = ss.str();
            size_t hashcode;
            MurmurHash3_x86_32(key.c_str(), (int) (key.length()), 22, &hashcode);
            return hashcode;
        }
    };
}

#endif //NET_CORE_COMMON_DEF_H
