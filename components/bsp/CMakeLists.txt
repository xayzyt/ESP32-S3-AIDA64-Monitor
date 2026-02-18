<<<<<<< HEAD
set(INCLUDE "." )
file(GLOB_RECURSE SOURCCES ./*.c)
idf_component_register(SRCS ${SOURCCES}
                    INCLUDE_DIRS ${INCLUDE}
                    REQUIRES driver esp_timer)
=======
set(INCLUDE "." )
file(GLOB_RECURSE SOURCCES ./*.c)
idf_component_register(SRCS ${SOURCCES}
                    INCLUDE_DIRS ${INCLUDE}
                    REQUIRES driver esp_timer)
>>>>>>> 81fc6d11b1a3aa3674e1ca4f6c48bf0d832d09d3
