# ✈️ AeroStock
### Aircraft Parts Inventory Management System

AeroStock is a **C++ client-server application** built for managing aircraft parts inventory over a TCP/IP network. A CLI client communicates with a server using a custom packet-based protocol to query parts, update stock, and transfer inventory files.

---

## 📋 Table of Contents

- [Overview](#overview)
- [Features](#features)
- [Project Structure](#project-structure)
- [Getting Started](#getting-started)
- [Running the App](#running-the-app)
- [Testing](#testing)
- [Documentation](#documentation)

---

## Overview

The system is split into a **Server** and a **Client** that communicate over TCP/IP using a predefined packet protocol. The server manages inventory state and enforces a state machine on all sessions. The client connects, authenticates via a handshake, and issues commands through a CLI menu.

---

## Features

| Feature | Description |
|---|---|
| TCP/IP Communication | Full client-server networking over sockets |
| Packet Protocol | Custom request/response message format |
| Verification Handshake | Session must be verified before any commands |
| Part Search | Search by part number or part name |
| Part Detail Lookup | Retrieve full details for a specific part |
| Stock Updates | Modify quantity for any part |
| File Transfer | Download full inventory catalog from server to client |
| Packet Logging | Both client and server log all packet traffic |
| State Machine | Server enforces valid session state transitions |
| Checksum Integrity | Packets include checksum validation |
| Automated Testing | Unit + integration tests across all layers |

---

## Project Structure

```
AeroStock/
├── Common/             # Shared protocol layer (packets, enums, checksum, serializer)
├── Common.Tests/       # Unit tests for the Common layer
├── Server/             # TCP server application (entry point)
├── Server.Core/        # Server logic (inventory, request handler, state machine)
├── Server.Tests/       # Unit tests for the Server layer
├── Client/             # CLI client application (entry point)
├── Client.Core/        # Client workflow and file management logic
├── Client.Tests/       # Unit tests for the Client layer
└── Integration.Tests/  # End-to-end integration smoke tests
```

---

## Getting Started

### Prerequisites

Install the following before opening the solution:

**Visual Studio 2022/2026**
- Workload: `Desktop development with C++`
- Components: MSVC compiler toolset, Windows SDK, CMake tools for Windows

**Qt 6.11.0**
- Kit: `MSVC 2022 64-bit`
- Do **not** use the MinGW kit

**Qt Visual Studio Tools Extension**
1. Open Visual Studio → `Extensions > Manage Extensions`
2. Search `Qt Visual Studio Tools` → Install → Restart Visual Studio
3. Go to `Qt > Qt Versions` and confirm the path points to your MSVC Qt install

Example Qt path:
```
C:\Qt\6.11.0\msvc2022_64
```

### Clone & Open

```bash
git clone https://github.com/mohiniu4/AeroStock.git
```

Open `AeroStock - Aircraft Parts Inventory Management System.slnx` in Visual Studio and build all projects.

---

## Running the App

### 1. Start the Server
Run the `Server` project first — it needs to be listening before the client connects.

### 2. Start the Client
Run the `Client` project and enter:
- Server IP address
- Server port

### 3. Use the CLI Menu
Once connected and verified, you can:
- Search inventory by part number or name
- Look up full part details
- Update stock quantities
- Download the inventory catalog file
- Disconnect cleanly

### Runtime Files
The following files may be generated at runtime and should not be committed:

```
server_packets.log
client_packets.log
inventory_catalog.dat
received_inventory_catalog.dat
```

---

## Testing

Tests are organized by layer and run automatically via GitHub Actions on every push.

| Test Project | Coverage |
|---|---|
| `Common.Tests` | Protocol constants, enums, checksums, packets, serializers, payload helpers |
| `Server.Tests` | Inventory behavior, request handling, state machine transitions |
| `Client.Tests` | Packet creation, response handling, file save behavior |
| `Integration.Tests` | End-to-end connect/verify/disconnect, search flows, stock updates, file transfer, negative paths |

To run tests locally, use the Visual Studio Test Explorer or run the test projects directly.

---

## Documentation

The workbook tracks:
- Functional requirements and user stories
- Sprint planning and progress
- Unit, integration, system, and usability test results

