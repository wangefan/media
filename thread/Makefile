.PHONY: all arm clean

# Variables
TARGET = main_produce_consume_sem

# Default target
all: $(TARGET)

# Build for host platform
$(TARGET): $(TARGET).cpp
	g++ $< -o $@ -lpthread

# Clean build files
clean:
	rm -f $(TARGET)