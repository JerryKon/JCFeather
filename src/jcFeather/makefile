JCFEATHER_SUBS=TinyXml RandomLib FeatherGen MayaPlugin JCFeatherLib  DelightDso

export CXXFLAGS=-O2 -Wall -c -fmessage-length=0 -fPIC

export MAYA_CXXFLAGS = -DBits64_ -m64 -DUNIX -D_BOOL -DLINUX -DFUNCPROTO -D_GNU_SOURCE \
         -DLINUX_64 \
         -fno-strict-aliasing -DREQUIRE_IOSTREAM -Wno-deprecated \
         -Wno-multichar -Wno-comment -Wno-sign-compare -funsigned-char \
         -Wno-reorder -fno-gnu-keywords -pthread -fopenmp \

export MAYA_VERSION=2013
export BOOST_HOME=/home/yingjie/lib/boost
export MAYA_LOCATION=/usr/autodesk/maya$(MAYA_VERSION)-x64
export ZLIB=./zlib
export DELIGHT=/home/yingjie/me/programe/3delight-11.0.108/Linux-x86_64

JCFeather:$(JCFEATHER_SUBS)

TinyXml:
	cd tinyXml && $(MAKE)
RandomLib:
	cd randomLib && $(MAKE)
FeatherGen:
	cd featherGen && $(MAKE)
MayaPlugin:
	cd featherMayaPlugin && $(MAKE)
JCFeatherLib:
	cd jcFeatherLib && $(MAKE)
DelightDso:
	-cd delightdso && $(MAKE)

clean:
	cd tinyXml && $(MAKE) clean
	cd randomLib && $(MAKE) clean
	cd featherGen && $(MAKE) clean
	cd featherMayaPlugin && $(MAKE) clean
	cd jcFeatherLib && $(MAKE) clean
	cd delightdso && $(MAKE) clean

