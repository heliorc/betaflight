
F405_TARGETS   += $(TARGET)
FEATURES       += VCP ONBOARDFLASH

TARGET_SRC = \
            drivers/accgyro/accgyro_imuf9001.c \
            drivers/accgyro/accgyro_fake.c \
            drivers/max7456.c