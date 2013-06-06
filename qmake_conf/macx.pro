macx {
#	PKGCONFIG += thrift libzmq

	LIBS += -lthrift \
		-L/opt/local/lib \
		-L/usr/lib \
		-L/usr/local/lib \
		/opt/local/lib/libboost_regex-mt.a \
		/opt/local/lib/libboost_system-mt.a \
		-lssl \
		-lpthread \
		-lcrypto \
		-lm \
		-ldl \
		-lz \
		-lreadline

	INCLUDEPATH += /usr/include \
		/opt/local/include \
		/opt/local/include/thrift \
		src
}

