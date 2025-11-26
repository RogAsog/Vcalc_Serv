#include <bits/stdc++.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>
#include "sha256.hpp"

using namespace std;

constexpr size_t LOGIN_SZ = 256;
constexpr size_t SALT16_SZ = 16;
constexpr size_t HASH_SZ = 64;

void log_error(const string &logfile, const string &err, bool critical) {
    FILE *f = fopen(logfile.c_str(), "a");
    if (!f) return;
    time_t t = time(nullptr);
    struct tm tm = *localtime(&t);
    fprintf(f, "%04d-%02d-%02d %02d:%02d:%02d | %s | %s\n",
        tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday,
        tm.tm_hour, tm.tm_min, tm.tm_sec,
        critical ? "CRITICAL" : "NONCRIT", err.c_str());
    fclose(f);
}

int load_users(const string &file, vector<pair<string,string>> &users) {
    ifstream in(file);
    if (!in.is_open()) return -1;
    string line;
    while (getline(in, line)) {
        if (line.empty()) continue;
        auto pos = line.find(':');
        if (pos == string::npos) continue;
        string login = line.substr(0,pos);
        string pass = line.substr(pos+1);
        users.emplace_back(login, pass);
        if (users.size() >= 100) break;
    }
    return 0;
}

static string to_hex_upper(const uint8_t *in, size_t len) {
    static const char* hex = "0123456789ABCDEF";
    string out;
    out.resize(len*2);
    for (size_t i=0;i<len;i++){
        out[i*2] = hex[(in[i]>>4)&0xF];
        out[i*2+1] = hex[in[i]&0xF];
    }
    return out;
}

bool verify_auth(const string &login, const string &salt16, const string &hashhex,
                const vector<pair<string,string>> &users) {
    for (auto &u : users) {
        if (u.first == login) {
            string concat = salt16 + u.second;
            uint8_t digest[32];
            sha256((const uint8_t*)concat.data(), concat.size(), digest);
            string calc = to_hex_upper(digest, 32);
            return calc == hashhex;
        }
    }
    return false;
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
ssize_t send_all(int fd, const void *buf, size_t len) {
    size_t sent = 0;
    while (sent < len) {
        ssize_t r = write(fd, (const char*)buf + sent, len - sent);
        if (r <= 0) return -1;
        sent += r;
    }
    return sent;
}

float sum_squares(const vector<float> &v) {
    double s = 0.0;
    for (float x : v) s += (double)x * (double)x;
    return (float)s;
}

int handle_client(int cfd, const vector<pair<string,string>> &users, const string &logfile) {
    // read login, salt, hash fixed sizes
    vector<char> login_buf(LOGIN_SZ);
    vector<char> salt_buf(SALT16_SZ);
    vector<char> hash_buf(HASH_SZ);
    if (recv_all(cfd, login_buf.data(), LOGIN_SZ) < 0) { log_error(logfile, "Failed read login", false); return -1; }
    if (recv_all(cfd, salt_buf.data(), SALT16_SZ) < 0) { log_error(logfile, "Failed read salt", false); return -1; }
    if (recv_all(cfd, hash_buf.data(), HASH_SZ) < 0) { log_error(logfile, "Failed read hash", false); return -1; }
    string login(login_buf.data(), LOGIN_SZ);
    // trim trailing NULs
    login = string(login.c_str());
    string salt(salt_buf.data(), SALT16_SZ);
    string hash(hash_buf.data(), HASH_SZ);
    if (!verify_auth(login, salt, hash, users)) {
        send_all(cfd, "ERR", 3);
        return -1;
    }
    send_all(cfd, "OK", 2);
    uint32_t N_net;
    if (recv_all(cfd, &N_net, 4) < 0) { log_error(logfile, "Failed read N", false); return -1; }
    uint32_t N = ntohl(N_net);
    if (N > 1000000) { log_error(logfile, "N too large", true); return -1; }
    vector<float> results;
    results.reserve(N);
    for (uint32_t i=0;i<N;i++) {
        uint32_t S_net;
        if (recv_all(cfd, &S_net, 4) < 0) { log_error(logfile, "Failed read S", false); return -1; }
        uint32_t S = ntohl(S_net);
        if (S > 10000000) { log_error(logfile, "S too large", true); return -1; }
        vector<float> vec(S);
        for (uint32_t j=0;j<S;j++) {
            uint32_t fnet;
            if (recv_all(cfd, &fnet, 4) < 0) { log_error(logfile, "Failed read float", false); return -1; }
            uint32_t fh = ntohl(fnet);
            float f;
            memcpy(&f, &fh, 4);
            vec[j] = f;
        }
        results.push_back(sum_squares(vec));
    }
    // send back N then N floats
    uint32_t outN = htonl((uint32_t)results.size());
    if (send_all(cfd, &outN, 4) < 0) { log_error(logfile, "Failed send outN", false); return -1; }
    for (float r : results) {
        uint32_t bits;
        memcpy(&bits, &r, 4);
        uint32_t outbits = htonl(bits);
        if (send_all(cfd, &outbits, 4) < 0) { log_error(logfile, "Failed send result", false); return -1; }
    }
    return 0;
}

int main(int argc, char **argv) {
    string db_file = "/etc/vcalc.conf";
    string log_file = "/var/log/vcalc.log";
    int port = 33333;
    if (argc == 1 || (argc == 2 && string(argv[1])=="-h")) {
        cout << "Usage: " << argv[0] << " -d DBFILE -l LOGFILE -p PORT\n";
        return 0;
    }
    for (int i=1;i<argc;i++) {
        if (string(argv[i])=="-d" && i+1<argc) db_file = argv[++i];
        else if (string(argv[i])=="-l" && i+1<argc) log_file = argv[++i];
        else if (string(argv[i])=="-p" && i+1<argc) port = atoi(argv[++i]);
    }
    vector<pair<string,string>> users;
    if (load_users(db_file, users) < 0) {
        cerr << "Failed to load user DB " << db_file << "\n";
        log_error(log_file, "Failed to load user DB", true);
        return 1;
    }
    int sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sfd < 0) { perror("socket"); log_error(log_file, "socket failed", true); return 1; }
    int opt = 1;
    setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;
    if (bind(sfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) { perror("bind"); log_error(log_file, "bind failed", true); return 1; }
    if (listen(sfd, 5) < 0) { perror("listen"); log_error(log_file, "listen failed", true); return 1; }
    cout << "vcalc C++ server listening on port " << port << endl;
    while (1) {
        int cfd = accept(sfd, nullptr, nullptr);
        if (cfd < 0) { log_error(log_file, "accept failed", false); continue; }
        handle_client(cfd, users, log_file);
        close(cfd);
    }
    close(sfd);
    return 0;
}
