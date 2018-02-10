
F405_TARGETS   += $(TARGET)
FEATURES       += VCP ONBOARDFLASH

TARGET_SRC = \
            drivers/accgyro/helio_spi_imuf.c \
            drivers/accgyro/accgyro_imuf9001.c \
            drivers/max7456.c