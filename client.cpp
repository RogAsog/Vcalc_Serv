#include <bits/stdc++.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>
#include "sha256.hpp"

using namespace std;

constexpr size_t LOGIN_SZ = 256;
constexpr size_t SALT16_SZ = 16;
constexpr size_t HASH_SZ = 64;

static string to_hex_upper(const uint8_t *in, size_t len) {
    static const char* hex = "0123456789ABCDEF";
    string out; out.resize(len*2);
    for (size_t i=0;i<len;i++){ out[i*2]=hex[(in[i]>>4)&0xF]; out[i*2+1]=hex[in[i]&0xF]; }
    return out;
}

void gen_salt16(char out[17]) {
    uint64_t r1 = ((uint64_t)rand() << 32) ^ rand() ^ ((uint64_t)time(nullptr));
    unsigned char bytes[8];
    for (int i = 0; i < 8; i++) bytes[i] = (r1 >> (8*(7-i))) & 0xFF;
    for (int i=0;i<8;i++){
        static const char* hex="0123456789ABCDEF";
        out[i*2]=hex[(bytes[i]>>4)&0xF];
        out[i*2+1]=hex[bytes[i]&0xF];
    }
    out[16]=0;
}

ssize_t send_all(int fd, const void *buf, size_t len) {
    size_t sent = 0;
    while (sent < len) {
        ssize_t r = write(fd, (const char*)buf + sent, len - sent);
        if (r <= 0) return -1;
        sent += r;
    }
    return sent;
}
ssize_t recv_all(int fd, void *buf, size_t len) {
    size_t got = 0;
    while (got < len) {
        ssize_t r = read(fd, (char*)buf + got, len - got);
        if (r <= 0) return -1;
        got += r;
    }
    return got;
}

int main(int argc, char **argv) {
    if (argc < 5) {
        cout << "Usage: " << argv[0] << " SERVERIP PORT LOGIN PASSWORD\n";
        return 1;
    }
    const char *ip = argv[1];
    int port = atoi(argv[2]);
    const char *login = argv[3];
    const char *password = argv[4];
    srand(time(nullptr) ^ getpid());
    int s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) { perror("socket"); return 1; }
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &addr.sin_addr);
    if (connect(s, (struct sockaddr*)&addr, sizeof(addr)) < 0) { perror("connect"); return 1; }
    vector<char> login_field(LOGIN_SZ,0);
    strncpy(login_field.data(), login, LOGIN_SZ-1);
    char salt16[SALT16_SZ+1];
    gen_salt16(salt16);
    string concat = string(salt16) + string(password);
    uint8_t digest[32];
    sha256((const uint8_t*)concat.data(), concat.size(), digest);
    string hash = to_hex_upper(digest, 32);
    // send auth fields
    if (send_all(s, login_field.data(), LOGIN_SZ) < 0) { perror("send login"); close(s); return 1; }
    if (send_all(s, salt16, SALT16_SZ) < 0) { perror("send salt"); close(s); return 1; }
    if (send_all(s, hash.c_str(), HASH_SZ) < 0) { perror("send hash"); close(s); return 1; }
    char resp[4] = {0};
    if (recv_all(s, resp, 2) < 0) { perror("recv resp"); close(s); return 1; }
    if (strncmp(resp, "OK", 2) != 0) { cout << "Auth failed: " << string(resp,2) << "\n"; close(s); return 1; }
    cout << "Auth OK\n";
    // send two test vectors as before
    uint32_t N = 2;
    uint32_t N_net = htonl(N);
    if (send_all(s, &N_net, 4) < 0) { perror("send N"); close(s); return 1; }
    uint32_t S1 = 3; uint32_t S1_net = htonl(S1);
    send_all(s, &S1_net, 4);
    float v1[3] = {1.0f,2.0f,3.0f};
    for (int i=0;i<3;i++){ uint32_t bits; memcpy(&bits,&v1[i],4); bits = htonl(bits); send_all(s, &bits, 4); }
    uint32_t S2 = 2; uint32_t S2_net = htonl(S2);
    send_all(s, &S2_net, 4);
    float v2[2] = {0.5f, -0.5f};
    for (int i=0;i<2;i++){ uint32_t bits; memcpy(&bits,&v2[i],4); bits = htonl(bits); send_all(s, &bits, 4); }
    uint32_t outN_net;
    if (recv_all(s, &outN_net, 4) < 0) { perror("recv outN"); close(s); return 1; }
    uint32_t outN = ntohl(outN_net);
    cout << "Server returned " << outN << " results\n";
    for (uint32_t i=0;i<outN;i++){
        uint32_t bits_net;
        recv_all(s, &bits_net, 4);
        uint32_t bits = ntohl(bits_net);
        float res; memcpy(&res, &bits, 4);
        cout << "Result " << i+1 << " = " << res << "\n";
    }
    close(s);
    return 0;
}
