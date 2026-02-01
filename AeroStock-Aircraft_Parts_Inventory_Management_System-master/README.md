# AeroStock

AeroStock is a C++ client-server **Aircraft Parts Inventory Management System** developed for the CSCN74000 group project.

The system uses **TCP/IP communication** and a shared **packet-based protocol** between a client application and a server application. It supports inventory queries, stock updates, connection verification, large file transfer, packet logging, and automated testing across the solution.

## Current Status

The project currently includes:

- a completed shared `Common` layer
- a working `Server` application
- a working **CLI client**
- automated unit tests for `Common`, `Server`, and `Client`
- automated integration smoke tests
- GitHub Actions workflows for automated test execution
- requirements, sprint, and test tracking through the project workbook

## Features

AeroStock currently supports:

- TCP/IP client-server communication
- predefined packet-based request/response messages
- verification handshake before operational commands
- search by part number
- search by part name
- part detail lookup
- stock quantity updates
- large file transfer from server to client
- client and server packet logging
- server-side state-machine enforcement
- checksum-based packet integrity support
- automated unit and integration testing

## Solution Structure

The Visual Studio solution currently contains:

- `Client`  
  CLI client application used to connect to the server and perform inventory operations.

- `Client.Core`  
  Shared client-side workflow and file-management logic used by the client application and client tests.

- `Client.Tests`  
  MSTest project for client-side unit tests.

- `Server`  
  Server application responsible for accepting connections, validating sessions, processing requests, enforcing state transitions, and returning responses or file data.

- `Server.Tests`  
  MSTest project for server-side unit tests.

- `Common`  
  Shared protocol and model layer used by both client and server. This includes packets, enums, constants, serializers, checksum logic, payload helpers, and shared data models.

- `Common.Tests`  
  MSTest project for shared protocol and model testing.

- `Integration.Tests`  
  Automated integration smoke tests for live client-server behavior.

## Automated Testing

The project currently includes:

### Unit Tests
- `Common.Tests`
- `Server.Tests`
- `Client.Tests`

### Integration Tests
- `Integration.Tests`

The automated test coverage currently includes:

- protocol constants, enums, checksums, packet headers, packets, serializers, payload helpers, and shared records
- server inventory behavior
- request handling
- server state-machine behavior
- client workflow packet creation and response handling
- client file save behavior
- end-to-end connect / verify / disconnect
- end-to-end search flows
- end-to-end stock update flow
- end-to-end large file transfer
- negative-path integration behavior for invalid commands and invalid stock updates

## Continuous Integration

GitHub Actions workflows are configured for:

- `Common.Tests`
- `Server.Tests`
- `Client.Tests`
- `Integration.Tests`

These workflows build the solution and run the relevant MSTest projects on Windows runners.

## Project Documentation

The project workbook is maintained outside the repository:

- **Requirements Traceability Matrix and Test Log (Conestoga access only)**  
  https://stuconestogacon-my.sharepoint.com/:x:/g/personal/mcvetkovic2965_conestogac_on_ca/IQC8AyC-DLWSRIiJ2pkzqPVxAQmVR9dGPC7BCPekXSh3ZD8?e=MMxvVF

This workbook tracks:

- requirements
- mapped user stories
- sprint planning and status
- implementation progress
- unit, integration, system, and usability testing records
- latest test results and execution history

## Development Environment

The project is intended to be developed using:

- Visual Studio 2022 / 2026
- Desktop development with C++
- MSVC compiler toolset
- Windows SDK
- Qt 6.11.0 MSVC 2022 64-bit
- Qt Visual Studio Tools extension

### Toolset Note

The repository projects use the **v143** platform toolset for compatibility with GitHub Actions and hosted Windows CI runners.

## Setup Instructions

### 1. Install Visual Studio components

In Visual Studio Installer, install:

- Desktop development with C++
- MSVC compiler toolset
- Windows SDK
- CMake tools for Windows

### 2. Install Qt

Install:

- Qt 6.11.0
- MSVC 2022 64-bit

Do **not** use the MinGW kit for this project.

### 3. Install Qt Visual Studio Tools

Inside Visual Studio:

- go to `Extensions > Manage Extensions`
- search for `Qt Visual Studio Tools`
- install the extension
- restart Visual Studio

### 4. Register the Qt version in Visual Studio

After installing Qt and the extension:

- go to `Qt > Qt Versions`
- add the Qt version if needed
- confirm the selected version points to the MSVC Qt installation

Example path:

`C:\Qt\6.11.0\msvc2022_64`

### 5. Clone the repository

```bash
git clone <repo-url>
```

### 6. Open the solution

Open the solution in Visual Studio and restore/build all projects.

## Running the Applications

### Server
Run the `Server` project first so it can begin listening for client connections.

### Client
Run the `Client` project and enter:

- server IP address
- server port

Then use the CLI menu to:

- connect and verify
- search inventory
- request part details
- update stock
- request the inventory catalog file
- disconnect cleanly

## Generated Runtime Files

The applications may generate runtime files such as:

- `server_packets.log`
- `client_packets.log`
- `inventory_catalog.dat`
- `received_inventory_catalog.dat`

These are runtime-generated artifacts and should not normally be committed to the repository.

## Notes

This project was developed as a course project with a strong focus on:

- requirements traceability
- testing and documentation
- state-machine enforcement
- packetized communication
- verification and auditability
