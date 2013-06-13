Open Workload Scheduler - Command Line Interface
================================================

Usage
-----

ows-cli <domain_name> <hostname>

Architecture
------------

This program is based on both [libcli] [3] and [OWS] [1]

Buidling
--------

# List of includes

* algorithm
* boost/algorithm/string.hpp
* boost/foreach.hpp
* boost/lexical_cast.hpp
* boost/multi_index/member.hpp
* boost/multi_index/ordered_index.hpp
* boost/multi_index_container.hpp
* boost/regex.hpp
* boost/thread.hpp
* boost/thread/mutex.hpp
* boost/thread/thread.hpp
* csignal
* cstdlib
* cstring
* ctype.h
* errno.h
* fstream
* iostream
* libcli.h
* limits
* malloc.h
* map
* memory.h
* mysql.h
* netdb.h
* netinet/in.h
* protocol/TBinaryProtocol.h
* readline/history.h
* readline/readline.h
* regex.h
* resolv.h
* server/TSimpleServer.h
* sqlite3.h
* stdarg.h
* stdint.h
* stdio.h
* stdlib.h
* string.h
* string
* sys/socket.h
* sys/time.h
* sys/types.h
* thrift/TApplicationException.h
* thrift/TDispatchProcessor.h
* thrift/Thrift.h
* thrift/protocol/TBinaryProtocol.h
* thrift/protocol/TProtocol.h
* thrift/server/TSimpleServer.h
* thrift/transport/TBufferTransports.h
* thrift/transport/TServerSocket.h
* thrift/transport/TTransport.h
* time.h
* transport/TBufferTransports.h
* transport/TServerSocket.h
* transport/TSocket.h
* unistd.h
* vector

# Commands

1. Install boost
2. Install readline
3. Install thrift
4. Install QMake

5. Download [open-workload-scheduler] [1]
> git clone git://github.com/MisterG/open-workload-scheduler.git

6. Generate the Thrift's source code
> cd open-workload-scheduler/src

> thrift --gen cpp model.thrift 

7. Patch the generated sources
> cd ..

> patch -p1 < patch/platform.path

8. Download [ows-cli] [2]
> cd ..

> git clone  git://github.com/MisterG/ows-cli.git

9. Create the Makefile according to the binary you want to build
> cd ows-cli

> qmake

10. Run make
> make

[1]: https://github.com/MisterG/open-workload-scheduler "open-workload-scheduler"
[2]: https://github.com/MisterG/ows-cli "ows-cli"
[3]: https://github.com/dparrish/libcli?source=cc "libcli"
