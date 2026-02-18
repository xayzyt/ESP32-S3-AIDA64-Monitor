set(INCLUDE "." )
file(GLOB_RECURSE SOURCCES ./*.c)
idf_component_register(SRCS ${SOURCCES}
                    INCLUDE_DIRS ${INCLUDE}
                    REQUIRES driver esp_timer)
