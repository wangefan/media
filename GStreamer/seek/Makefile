.PHONY: all arm clean

# Variables
TARGET = main_seek
PKG_CONFIG = `pkg-config --cflags --libs gstreamer-1.0`
ENV_SETUP_PATH = /opt/turnkey/8.293.0-eb.20240227083033/environment-setup-armv7at2hf-neon-poky-linux-gnueabi

# Default target
all: $(TARGET)

# Build for host platform
$(TARGET): $(TARGET).c
	gcc $< -o $@ $(PKG_CONFIG)

# Build for ARM platform
arm:
	@echo "Sourcing the environment setup script..."
	@sh -c '. $(ENV_SETUP_PATH) && $$CC $(TARGET).c -o $(TARGET)-arm $(PKG_CONFIG)'

# Clean build files
clean:
	rm -f $(TARGET) $(TARGET)-arm