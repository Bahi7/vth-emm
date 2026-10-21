# VTH: a volume-hiding encrypted multi-map driven by a queue

Implementation and evaluation harness of VTH, an encrypted multi-map whose
leakage is controlled by *when* the server is accessed rather than by padding
*how much* is accessed. The scheme comes from joint work with Seny Kamara and
Tarik Moataz; the accompanying paper, in which the construction is called EXH,
is under submission and will be cited here once it is public. The code in this
repository is mine. I wrote it between July 2023 and January 2024 inside Tarik
Moataz's [`latencyQT`](https://github.com/orochi89/latencyQT) repository, and
this repository is that code with its git history, moved out on its own.

## The idea in one paragraph

A standard encrypted multi-map answers a query for label `l` by returning all
`|MM[l]|` encrypted values at once, so the server learns the volume of every
query. VTH puts a proxy between the client and the server. The proxy turns a
query into `|MM[l]|` *sub-tokens*, one per value, and pushes them onto a queue.
Independently of what the clients do, a queue process pops one sub-token at a
time and performs one access to a leakage-free encrypted dictionary (an
encrypted array behind a Path ORAM); when the queue is empty it accesses a fixed
dummy address instead. The server therefore sees a stream of single ORAM
accesses whose timing does not depend on the queries, and the only thing that
varies is the latency the client experiences, which is governed by the arrival
rate `λ` of sub-tokens and the service rate `μ` of the queue, exactly as in an
M/M/1 queue. The evaluation measures that latency and the queue length over
time, and compares them with two baselines: padding every response to the
maximum response length, and storing each tuple as one block.

## Repository layout

```
synvhq/      the implementation used for the evaluation (January 2024)
  Client.cpp     loads a multi-map, starts the proxy thread, and runs clients that
                 issue queries for random labels with exponential inter-arrival
                 times (lambda = 32 by default)
  Lat.h          the VTH proxy: the sub-token queue, the queue process SQPD_P_S,
                 dummy accesses to the address "alpha", the two baselines, and the
                 CSV logging of queue sizes and per-query latency
  LDX.h          the leakage-free dictionary: client side of a non-recursive Path
                 ORAM (position map, stash, AES-encrypted blocks) over a TCP
                 connection (asio) to the storage server
  PathOram.h     the ORAM tree, path reads and writes, eviction and re-encryption
  Server.cpp     the storage server: loads the serialised tree, serves path reads
                 and writes on port 3333
  Includes.h     shared includes
  PlotLatency.py, PlotQDist.py   plots from latency_data.csv and QueueSize.csv
  README.md      the notes written for the artifact: how to switch between VTH
                 and the two baselines, what the client prints and writes
  SynVHQ.sln, SynVHQ.vcxproj     Visual Studio 2022 project (C++20)
prototype/   the first version (October and November 2023)
  Lat.cpp, LDX.cpp, PathORAM.cpp, client.cpp, server.cpp, Makefile
                 the same three components over raw Windows sockets and cereal,
                 kept as it was, with a reconstructed utils.h and a Makefile that
                 matches the file names
  MMc.py         a simpy simulation of the proxy queue as an M/M/1 and M^X/M/1
                 system fed with the real response-length distribution of a
                 multi-map (June and July 2023)
  Main.tex       my June 2023 note on using queueing models to hide volume
```

## Building and running the evaluation code

The project was built with Visual Studio 2022 (C++20, x64) on Windows. It needs:

* OpenSSL 3 (AES-CTR, key derivation, random bytes) and nlohmann/json, for
  example through vcpkg with `vcpkg integrate install`;
* three header-only libraries: cereal 1.3.2, standalone asio 1.28 and Boost 1.84
  (only `boost/lockfree`). The project looks for them under
  `third_party/cereal/include`, `third_party/asio/include` and
  `third_party/boost` next to the solution; adjust `IncludePath` in
  `SynVHQ.vcxproj` if they live elsewhere.

Run `Server.cpp` first, then `Client.cpp`. The client reads the multi-map from
the folder named in `input_directory` (`MM\MM` by default) as JSON files that map
each document to its keywords, the dumps written by `TextExtractPar.saveMM` in
[volume-hiding-emm](https://github.com/Bahi7/volume-hiding-emm), and inverts
them; `pow(2, 12)` in `main` caps the number of pairs loaded. Setup serialises
the ORAM tree to `serialized_data.bin`, which the server reads from its working
directory, so both programs must run in the same folder. After a warm-up of 100
queries the proxy appends to `QueueSize.csv` (queue length and the time spent at
that length) and `latency_data.csv` (response length and end-to-end latency of
each query), prints the mean service time every 5000 sub-tokens, and stops after
10,000 queries. `synvhq/README.md` explains which lines to toggle to run the
two baselines instead of VTH.

The prototype builds with MinGW `g++` and cereal only (`make CEREAL_INC=...` in
`prototype/`); it is the first working version and is kept for the record rather
than for use.

## Provenance and license

Everything here was written by me, Amine Bahi, between July 2023 and January
2024; the commits are mine (the "Husky" author on two commits is the Windows
account the Visual Studio project was created under) and Tarik Moataz's two
commits edit the artifact README. The history was extracted from `latencyQT`
with `git filter-repo`. The 60 MB of Enron multi-map dumps used as input were
left out. A later two-client harness built on a later revision of these components (the
position map encrypted and stored on the server) is in
[TigroEXH](https://github.com/Bahi7/TigroEXH); that revision is not in this repository.

Released under the MIT License; see [LICENSE](LICENSE).
