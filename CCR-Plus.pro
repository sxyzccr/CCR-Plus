TEMPLATE = subdirs

SUBDIRS += ccr-plus \
    src/tools/checker \
    src/updater
   
unix: SUBDIRS += src/tools/monitor

ccr-plus.file = src/CCR-Plus.pro
