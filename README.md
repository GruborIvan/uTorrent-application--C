# uTorrent-application (C/C++)

A small peer-to-peer (P2P) file sharing project inspired by BitTorrent/uTorrent concepts, implemented in C/C++ and using WebSockets for communication.

## What it does
- Demonstrates a basic **P2P file-sharing workflow**
- Tracks **which peers have which files** (availability/discovery concept)
- Includes a **stress test** folder and basic performance/memory artifacts

## Tech stack
- **C++ / C**
- **WebSockets** (for peer communication / coordination)

## Repository structure
- `Peer-2-Peer-file-sharing---C-master/` – main project source code
- `Stress Test/` – load/stress testing utilities
- `IKP_Projekat_Dokumentacija.pdf` – project documentation
- `HeapShot.png`, `PerfMonitor.png` – profiling/performance snapshots

## How to run (high level)
1. Build the project (C/C++ toolchain required).
2. Start the coordinating component (if present) and/or the peer instance.
3. Run multiple peers to test sharing and availability behavior.
4. Use the `Stress Test/` tools to validate performance under load.

## Notes
This is a learning/prototype project intended to demonstrate P2P concepts and WebSocket-based communication, not a production-grade torrent client.
