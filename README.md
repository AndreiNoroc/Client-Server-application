# UDP-TCP Client-Server Communication System

This project implements a communication system using both UDP and TCP sockets. The server handles multiple connections, receives messages from UDP clients, and forwards them to TCP clients subscribed to specific topics. It also supports client reconnection, offline message queuing, and basic command handling like `subscribe` and `unsubscribe`.

---

## Project Structure

### `helpers.h`
- Header file containing all data structures used in the application.
- Includes macro definitions and constants.

---

### `server.cpp`
Implements the main server logic:

- Creates UDP and TCP sockets.
- Binds sockets to appropriate addresses.
- Listens for incoming TCP connections.
- Disables Nagle’s algorithm to reduce latency.
- Uses `select()` to multiplex:
  - TCP socket,
  - UDP socket,
  - `stdin` (for server shutdown).
- Waits for client connections and processes socket events.
- Handles:
  - TCP client connections and identification.
  - UDP message reception and forwarding to relevant TCP clients.
  - Offline message storage for disconnected clients (store-and-forward).
  - Client reconnection with message delivery.
- Processes commands:
  - `exit` – shuts down the server.
  - `subscribe` – adds a client to a topic with an optional store-forward flag.
  - `unsubscribe` – removes a client from a topic.

---

### `subscribe.cpp`
Implements the TCP client logic:

- Creates and connects a socket to the server.
- Sends a unique ID to the server for identification.
- Disables Nagle’s algorithm to optimize communication.
- Uses `select()` to handle:
  - User input from `stdin` for commands.
  - Incoming messages from the server.
- Command handling:
  - `subscribe <topic> <SF>` – subscribes to a topic, optionally storing offline messages.
  - `unsubscribe <topic>` – unsubscribes from a topic.
  - `exit` – disconnects the client gracefully.
- Displays messages received from the server (originally sent by UDP clients).

---

## Features

- **I/O Multiplexing using `select()`** – efficiently handles multiple I/O sources.
- **Client reconnection** – clients can reconnect using the same ID and receive stored messages.
- **Offline message support** – messages are stored for inactive clients if requested.
- **UDP-to-TCP message forwarding** – the server relays UDP messages to relevant TCP clients.
- **Topic-based messaging system** – clients receive only the messages for the topics they subscribe to.

---

## Notes

- This implementation was based on Labs 6, 7, and 8 of the networking course.
- Written in C++ and tested on Linux systems using modern compilers.

---

## Build & Run

### Compile
```bash
make
