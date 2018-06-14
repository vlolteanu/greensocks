TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt
LIBS += -lpthread

QMAKE_CXXFLAGS += -std=c++11

SOURCES += \
    core/listenreactor.cc \
    core/poller.cc \
    core/reactor.cc \
    core/streamreactor.cc \
    greensocks.cc \
    proxifier/proxifier.cc \
    proxifier/proxifierdownstreamer.cc \
    proxifier/proxifierupstreamer.cc \
    proxifier/proxifiertfopolicy.cc

HEADERS += \
    core/poller.hh \
    core/listenreactor.hh \
    core/reactor.hh \
    core/streamreactor.hh \
    proxifier/proxifier.hh \
    proxifier/proxifierdownstreamer.hh \
    proxifier/proxifierupstreamer.hh \
    proxifier/proxifiertfopolicy.hh

LIBS = -lsocks6msg -lsocks6util -lpthread -lboost_system
