set(INCLUDE "." "./ui/generated" "./ui/custom" "./ui/generated/guider_customer_fonts")
file(GLOB_RECURSE SOURCCES ./*.c)

idf_component_register(SRCS ${SOURCCES}
                    INCLUDE_DIRS ${INCLUDE})

spiffs_create_partition_image(html ../html FLASH_IN_PROJECT)
spiffs_create_partition_image(img ../img FLASH_IN_PROJECT)
