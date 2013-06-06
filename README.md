Open Workload Scheduler - Command Line Interface

h1. Usage

ows-cli <domain_name> <hostname>

h1. Architecture

This program is based on both "libcli":https://github.com/dparrish/libcli?source=cc and "OWS":https://github.com/MisterG/open-workload-scheduler

h1. Buidling

h2. List of includes

algorithm
boost/algorithm/string.hpp
boost/foreach.hpp
boost/lexical_cast.hpp
boost/multi_index/member.hpp
boost/multi_index/ordered_index.hpp
boost/multi_index_container.hpp
boost/regex.hpp
boost/thread.hpp
boost/thread/mutex.hpp
boost/thread/thread.hpp
csignal
cstdlib
cstring
ctype.h
errno.h
fstream
iostream
libcli.h
limits
malloc.h
map
memory.h
mysql.h
netdb.h
netinet/in.h
protocol/TBinaryProtocol.h
readline/history.h
readline/readline.h
regex.h
resolv.h
server/TSimpleServer.h
sqlite3.h
stdarg.h
stdint.h
stdio.h
stdlib.h
string.h
string
sys/socket.h
sys/time.h
sys/types.h
thrift/TApplicationException.h
thrift/TDispatchProcessor.h
thrift/Thrift.h
thrift/protocol/TBinaryProtocol.h
thrift/protocol/TProtocol.h
thrift/server/TSimpleServer.h
thrift/transport/TBufferTransports.h
thrift/transport/TServerSocket.h
thrift/transport/TTransport.h
time.h
transport/TBufferTransports.h
transport/TServerSocket.h
transport/TSocket.h
unistd.h
vector

h2. Commands

# Install boost
# Install readline
# Install thrift
# Install QMake

# Download "open-workload-scheduler":https://github.com/MisterG/open-workload-scheduler
@git clone git://github.com/MisterG/open-workload-scheduler.git@

# Generate the Thrift's source code
@cd open-workload-scheduler/src@
@thrift --gen cpp model.thrift@ 

# Patch the generated sources
@cd ..@
@patch -p1 < patch/platform.path@

# Download "ows-cli":open-workload-scheduler":https://github.com/MisterG/ows-cli"
@cd ..@
@git clone  git://github.com/MisterG/ows-cli.git@

# Create the Makefile according to the binary you want to build
@cd ows-cli@
@qmake@

# Run make
@make@

