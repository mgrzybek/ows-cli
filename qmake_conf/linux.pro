linux {
	LIBS += -lthrift \
		-L/usr/local/lib \
		-L/usr/lib \
		-L/usr/local/lib \
		/usr/lib/libboost_regex.a \
		/usr/lib/libboost_filesystem.a \
		/usr/lib/libboost_system.a \
		-lssl \
		-lpthread \
		-lcrypto \
		-lm \
		-ldl \
		-lz

	INCLUDEPATH +=  /usr/include \
		/usr/include/thrift \
		src
}

