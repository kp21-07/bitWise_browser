#ifndef __NETWORK_HPP__
#define __NETWORK_HPP__

#include "../includes/telemetry.hpp"
#include "common.hpp"
#include <arpa/inet.h>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <netdb.h>
#include <functional>
#include <string>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>

using namespace std;

string asyncGetRequest(const string &host, const string &port, const string &path);

void fetch(const string url, function<void(string)> func);
void debugFetch(const string url, function<void(string)> func);

#endif
