!macx:unix {
	LIBS += -lthrift \
		-L/usr/local/lib \
		-L/usr/lib \
		-L/usr/local/lib \
		/usr/local/lib/libboost_regex.a \
		/usr/local/lib/libboost_filesystem.a \
		/usr/local/lib/libboost_system-mt.a \
		-lssl \
		-lpthread \
		-lcrypto \
		-lm \
		-ldl \
		-lz \

	INCLUDEPATH +=  /usr/local/include \
		/usr/local/include/thrift \
		src
}

