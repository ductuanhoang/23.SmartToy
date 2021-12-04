file(GLOB_RECURSE SOURCES *.c)

idf_component_register(SRCS ${SOURCES}
                       INCLUDE_DIRS .
                       REQUIRES spiffs nvs_flash fatfs peripheral)
