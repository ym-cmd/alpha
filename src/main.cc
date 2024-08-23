#include "../include/net/EchoServer.h"
#include <bits/stdc++.h>
using namespace std;

int main() {
    EchoServer server(4, 10, "192.168.137.146", 8888);
    server.start();
    return 0;
    return 0;
}