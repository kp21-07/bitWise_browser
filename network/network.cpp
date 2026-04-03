#include "../includes/network.hpp"
#include <bits/types/struct_timeval.h>
#include <chrono>
#include <cstdio>
#include <fcntl.h>
#include <iostream>
#include <netdb.h>
#include <string>
#include <sys/socket.h>
#include <thread>
#include <fstream>
#include <sstream>
#include <unistd.h>

using namespace std;

string currentUrl;

string asyncGetRequest(const string &host, const string &port, const string &path) {
  struct addrinfo *req, addrinfo{};
  char buffer[4096];
  string response;

  addrinfo.ai_family = AF_INET;
  addrinfo.ai_socktype = SOCK_STREAM;
  addrinfo.ai_protocol = IPPROTO_TCP;

  if (getaddrinfo(host.c_str(), port.c_str(), &addrinfo, &req) != 0)
    return "";

  int sock = socket(addrinfo.ai_family, addrinfo.ai_socktype, addrinfo.ai_protocol);
  if (connect(sock, req->ai_addr, req->ai_addrlen) != 0) {
    close(sock);
    freeaddrinfo(req);
    return "";
  }

  string request = "GET " + path + " HTTP/1.1\r\nHost: " + host + "\r\nConnection: close\r\n\r\n";
  send(sock, request.c_str(), request.size(), 0);

  while (true) {
    int n = recv(sock, buffer, sizeof(buffer), 0);
    if (n <= 0)
      break;
    response.append(buffer, n);
  }

  close(sock);
  freeaddrinfo(req);
  return response;
}

string readFromFile(const string filename){
  ifstream file(filename);

  if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return "";
  }

  stringstream buffer;
  buffer << file.rdbuf();
  return buffer.str();
}

void debugFetch(const string url, function<void(string)> func){
thread([url, func] {
    currentUrl = url;

    string fetchUrl = url;
    if (fetchUrl.length() >= 7 && fetchUrl.substr(0, 7) == "http://")
      fetchUrl = fetchUrl.substr(7);
    if (fetchUrl.length() >= 8 && fetchUrl.substr(0, 8) == "https://")
      fetchUrl = fetchUrl.substr(8);

    string response = readFromFile(fetchUrl);
    if (!response.empty())
      func(response);
    else
      func("Error");
  }).detach();
}

void fetch(const string url, function<void(string)> func) {
  thread([url, func] {
    currentUrl = url;

    string fetchUrl = url;
    if (fetchUrl.length() >= 7 && fetchUrl.substr(0, 7) == "http://")
      fetchUrl = fetchUrl.substr(7);
    if (fetchUrl.length() >= 8 && fetchUrl.substr(0, 8) == "https://")
      fetchUrl = fetchUrl.substr(8);

    string response = asyncGetRequest(fetchUrl, "80", "/");
    if (!response.empty())
      func(response);
    else
      func("Error");
  }).detach();
}

#ifdef NETWORK_DEBUG
int main() {
  fetch("www.google.com", [](string response) { cout << "Done \n" << endl; });
  while (true) {
    this_thread::sleep_for(chrono::seconds(2));
    if (!currentUrl.empty())
      cout << "Current URL: " << currentUrl << endl;
  }

  return 0;
}
#endif
