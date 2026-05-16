ROOT := ../../..
BUILD := build

CC := sdcc
CFLAGS := -mmcs51 --std-sdcc11 \
	-I$(ROOT)/core \
	-I$(ROOT)/hal \
	-I$(ROOT)/drivers \
	-I$(ROOT)/board/stc8h1k08_tssop20_demo \
	-DSTC8H_CONFIG_INCLUDE=\"board_config.h\" \
	-DSTC8H_PINS_INCLUDE=\"board_pins.h\"

SRCS := \
	main.c \
	$(ROOT)/core/stc8h_delay.c \
	$(ROOT)/hal/stc8h_gpio.c \
	$(ROOT)/hal/stc8h_gpio_toggle.c \
	$(ROOT)/hal/stc8h_uart.c \
	$(ROOT)/hal/stc8h_i2c.c \
	$(ROOT)/board/stc8h1k08_tssop20_demo/board_init.c

TARGET := $(BUILD)/i2c_scan.ihx
OBJS := $(patsubst %.c,$(BUILD)/%.rel,$(notdir $(SRCS)))
VPATH := \
	. \
	$(ROOT)/core \
	$(ROOT)/hal \
	$(ROOT)/board/stc8h1k08_tssop20_demo

.PHONY: all clean

all: $(TARGET)

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/%.rel: %.c | $(BUILD)
	$(CC) $(CFLAGS) -c -o $@ $<

$(TARGET): $(OBJS) | $(BUILD)
	$(CC) $(CFLAGS) --out-fmt-ihx -o $(TARGET) $(OBJS)

clean:
	rm -rf $(BUILD)
