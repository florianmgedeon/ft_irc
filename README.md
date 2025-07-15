# ft_irc – Custom IRC Server in C++98

## 📌 Project Overview

This project is a custom implementation of an IRC (Internet Relay Chat) server written in C++98, developed as part of the curriculum at [42 Vienna](https://42vienna.com). The server is designed to handle multiple clients efficiently, using non-blocking I/O and scalable multiplexing techniques.

It was developed collaboratively by:
- **Florian Gedeon**
- **Gregor Walchshofer**
- **Christoph Thaler**

## ⚙️ Key Features

- Full socket and signal handling
- Authentication and basic command parsing:
  - `PASS`
  - `NICK`
  - `USER`
- Multi-client handling with a single epoll instance
- Efficient, non-blocking communication using edge-triggered epoll
- Standards-compliant behavior for IRC clients

## 🧠 Why epoll?

Instead of using `poll()` as mentioned in the subject, we opted for `epoll()` due to its superior performance in managing large numbers of file descriptors. `epoll` allows us to build an efficient, event-driven server with edge-triggered notifications, reducing CPU overhead and improving responsiveness under high concurrency.

## 🧩 Responsibilities

**Florian Gedeon** was primarily responsible for:
- The complete socket lifecycle management (from creation to closure)
- Signal handling setup for graceful shutdowns
- Parsing and handling of IRC authentication commands (`NICK`, `USER`, `PASS`)
- Implementing the epoll-based event loop and file descriptor management logic

## 🔧 How to Use

```bash
# Clone the repository
git clone https://github.com/yourusername/ft_irc.git
cd ft_irc

# Build the server
make

# Run the server
./ircserv <port> <password>
