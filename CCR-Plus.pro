TEMPLATE = subdirs

SUBDIRS += ccr-plus \
    src/tools/checker
   
unix: SUBDIRS += src/tools/monitor

ccr-plus.file = src/CCR-Plus.pro
