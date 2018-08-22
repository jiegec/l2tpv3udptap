// Copyright (C) 2018 Jiajie Chen
// 
// This file is part of l2tpv3udptap.
// 
// l2tpv3udptap is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// l2tpv3udptap is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with l2tpv3udptap.  If not, see <http://www.gnu.org/licenses/>.
// 

#include <unistd.h>
#include <fcntl.h>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string>
#include <thread>
#include <strings.h>

struct l2tpv3udp {
    uint8_t res1;
    uint8_t ver;
    uint16_t res2;
    uint32_t session_id;
    uint32_t cookie[1]; // 8bit or 16bit
};

struct l2tpv3l2spec {
    uint8_t flags; // zero sequence
    uint8_t seq_hi;
    uint16_t seq_lo;
};

uint32_t session_id = 0, peer_session_id = 0;
uint32_t cookie[2] = {0}, cookie_len = 4;
uint32_t peer_cookie[2] = {0}, peer_cookie_len = 4;
std::string local = "0.0.0.0", remote = "0.0.0.0";
uint16_t port = 0, peer_port = 0;
std::string tap_if;
int udp_fd, tap_fd;

void l2tpv3_to_tap() {
    uint8_t buffer[4096] = {0};
    size_t hdrlen = sizeof(struct l2tpv3udp) + sizeof(struct l2tpv3l2spec) + (cookie_len - 4);
    l2tpv3udp *header = (l2tpv3udp*)buffer;

    struct sockaddr_in saddr;
    socklen_t size_saddr = sizeof(struct sockaddr_in);

    while(1) {
        ssize_t size = recvfrom(udp_fd, buffer, sizeof(buffer), 0, (sockaddr *)&saddr, &size_saddr);
        if (size == -1)
            continue;
        if (size < hdrlen) {
            printf("Length too short\n");
            continue;
        }
        if (header->ver != 3) {
            printf("Got L2TPv%d, ignoring\n", header->ver);
            continue;
        }
        if (htonl(header->session_id) != session_id) {
            printf("Bad session ID %08X, ignoring\n", htonl(header->session_id));
            continue;
        }
        if (bcmp(header->cookie, cookie, cookie_len) != 0) {
            printf("Bad cookie %08X%08X, ignoring\n", htonl(header->cookie[0]), htonl(header->cookie[1]));
            continue;
        }
        // TODO: check saddr == remote
        if (write(tap_fd, buffer + hdrlen, size - hdrlen) < 0) {
            perror("write");
        }
    }
}

void tap_to_l2tpv3() {
    uint8_t buffer[4096] = {0};
    size_t hdrlen = sizeof(struct l2tpv3udp) + sizeof(struct l2tpv3l2spec) + (peer_cookie_len - 4);
    l2tpv3udp *header = (l2tpv3udp*)buffer;
    header->ver = 3;
    header->session_id = htonl(peer_session_id);
    memcpy(header->cookie, peer_cookie, peer_cookie_len);

    sockaddr_in daddr;
    memset(&daddr, 0, sizeof(daddr));
    daddr.sin_family = AF_INET;
    daddr.sin_addr.s_addr = inet_addr(remote.c_str());
    daddr.sin_port = htons(peer_port);

    while(1) {
        ssize_t size = read(tap_fd, buffer+hdrlen, sizeof(buffer)-hdrlen);
        if (size == -1)
            continue;
        int result = sendto(udp_fd, buffer, size+hdrlen, 0, (sockaddr *)&daddr, sizeof(daddr));
        if (result < 0) {
            perror("sendto");
        }
    }
}

bool read_cookie(uint32_t cookie[2], uint32_t &cookie_len, const char *str) {
    size_t len = strlen(str);
    if (len == 8) {
        cookie_len = 4;
        sscanf(str, "%8X", &cookie[0]);
        cookie[0] = htonl(cookie[0]);
        return true;
    } else if (len == 16) {
        cookie_len = 8;
        sscanf(str, "%8X%8X", &cookie[0], &cookie[1]);
        cookie[0] = htonl(cookie[0]);
        cookie[1] = htonl(cookie[1]);
        return true;
    }
    return false;
}

int main(int argc, char *argv[]) {
    if (argc != 10) {
        printf("Usage: l2tpv3udptap [tap_if] [local_ip] [remote_ip] [port] [peer_port] [session_id] [peer_session_id] [cookie] [peer_cookie]\n");
        printf("\tequivalent to: ip l2tp add tunnel remote [remote_ip] local [local_ip] tunnel_id 0 peer_tunnel_id 0 encap udp\n");
        printf("\t             : ip l2tp add session name [tap_if] tunnel_id 0 session_id [session_id] peer_session_id [peer_session_id] cookie [cookie] peer_cookie [peer_cookie]\n");
        return 1;
    }

    tap_if = argv[1];
    local = argv[2];
    remote = argv[3];
    port = atoi(argv[4]);
    peer_port = atoi(argv[5]);
    session_id = atoi(argv[6]);
    peer_session_id = atoi(argv[7]);
    if (!read_cookie(cookie, cookie_len, argv[8])) {
        printf("Bad cookie\n");
        return 1;
    }
    if (!read_cookie(peer_cookie, peer_cookie_len, argv[9])) {
        printf("Bad peer cookie\n");
        return 1;
    }

    std::string dev_path = "/dev/" + tap_if;
    if ((tap_fd = open(dev_path.c_str(), O_RDWR)) < 0) {
        perror("open");
        return 1;
    }

    if ((udp_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        perror("socket");
        return 1;
    }

    sockaddr_in saddr;
    memset(&saddr, 0, sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = inet_addr(local.c_str());
    saddr.sin_port = htons(port);

    if (bind(udp_fd, (sockaddr *)&saddr, sizeof(saddr)) < 0) {
        perror("bind");
        return 1;
    }

    std::thread t1(tap_to_l2tpv3);
    std::thread t2(l2tpv3_to_tap);
    t1.join();
    t2.join();
    return 0;
}
