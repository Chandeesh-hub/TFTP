# TFTP Client-Server (C, UDP Sockets)

A C implementation of the **Trivial File Transfer Protocol (TFTP)**, built on UDP. The project has two standalone programs — a menu-driven **client** and a **server** — that transfer files between each other using a simplified TFTP-style handshake (RRQ/WRQ, DATA, ACK).

**Author:** Chandeesh K M

## Features

- Menu-driven client: `Connect`, `Put`, `Get`, `Mode`, `Exit`
- File upload (`Put` → WRQ) and download (`Get` → RRQ)
- Three transfer modes:
  - **Default** — 512 bytes per packet
  - **Octet** — 1 byte per packet
  - **Netascii** — 512 bytes per packet, with `\n` ↔ `\r\n` conversion on send/receive
- IP address validation on the client before connecting
- Retry logic (up to `MAX_RETRIES`) on `sendto`/`recvfrom` timeouts
- Socket-level send/receive timeouts (`TIMEOUT_SEC`)

## Project Structure

```
TFTP/
├── CLIENT/
│   ├── tftp.h              # Shared protocol definitions (opcodes, packet struct, constants)
│   ├── tftp_client.h        # Client-side function declarations & struct
│   ├── tftp_client.c        # Client implementation (menu, connect, put, get, mode)
│   └── tftp_project.pdf     # Project write-up / documentation
└── SERVER/
    ├── tftp.h                # Shared protocol definitions (copy of client's tftp.h)
    └── tftp_server.c         # Server implementation (listens, handles WRQ/RRQ/DATA)
```

## Protocol Overview

Defined in `tftp.h`:

| Constant       | Value | Meaning                                   |
|----------------|-------|--------------------------------------------|
| `PORT`         | 6969  | UDP port the server listens on             |
| `BUFFER_SIZE`  | 516   | Packet size (4-byte header + 512 data)     |
| `TIMEOUT_SEC`  | 5     | Socket send/receive timeout (seconds)      |
| `MAX_RETRIES`  | 5     | Retry attempts before aborting a transfer  |

Opcodes: `RRQ` (1), `WRQ` (2), `DATAW` (3), `DATAR` (4), `ACK` (5), `ERROR` (6).

**Upload (Put):** client sends `WRQ` → server opens file, replies `ACK` → client streams `DATA` packets → server writes and `ACK`s each block until a short/empty packet signals end of file.

**Download (Get):** client sends `RRQ` → server checks the file exists and replies success/failure → client requests blocks via `DATAR` → server streams data until a short/empty packet signals end of file.

## Building

Requires `gcc` and a POSIX/Linux environment (uses `sys/socket.h`, `arpa/inet.h`, etc.).

```bash
# Build the client
cd TFTP/CLIENT
gcc -o tftp_client tftp_client.c

# Build the server
cd ../SERVER
gcc -o tftp_server tftp_server.c
```

> **Known build issue:** `tftp_server.c` currently fails to compile — `handle_client()` references an undeclared variable `ack_sent` (line ~242) instead of the `ack` variable it declares. Rename that check to `ack` (or declare `ack_sent`) to get a clean build.

## Running

1. **Start the server** on the host machine:
   ```bash
   ./tftp_server
   ```
   > The server currently binds to a hardcoded IP address (`192.168.163.86` in `tftp_server.c`). Edit this to match the machine's own interface IP (or use `INADDR_ANY`) before running elsewhere.

2. **Start the client** and use the menu:
   ```bash
   ./tftp_client
   ```
   ```
   MENU
   1. Connect
   2. Put
   3. Get
   4. Mode
   5. Exit
   ```
   - **Connect** — enter the server's IP address to open a UDP session.
   - **Mode** — choose Default / Octet / Netascii before transferring (defaults to Default/512 bytes).
   - **Put** — enter a filename that exists in the client's working directory to upload it.
   - **Get** — enter a filename that exists on the server to download it.
   - **Exit** — closes the socket and quits.

## Notes / Limitations

- Server handles one client request at a time in a single-threaded loop (no concurrency).
- IP validation on the client accepts standard dotted-decimal IPv4 addresses only.
- Filenames are limited by fixed-size buffers (`filename[35]` on the client, `filename[256]` in the packet struct).
- See `CLIENT/tftp_project.pdf` for the original project write-up.
