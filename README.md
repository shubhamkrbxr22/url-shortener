Distributed URL Shortener

High-performance URL shortening service built using C++, Crow and Nginx.

Features

- Fast URL shortening using Base62 encoding
- REST API built with Crow (C++)
- Nginx reverse proxy support
- Threaded high-performance server

Tech Stack

- C++
- Crow
- Nginx
- Base62 Encoding

How to Run

```bash
g++ server.cpp -o server -lpthread
./server 8081
