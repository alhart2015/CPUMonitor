all: gui

#
# VARS
#

CC=clang
CFLAGS=-g -O0 -Wall -Werror

LDFLAGS=

BUILD_DIR=build

BACK_DIR=back
GUI_DIR = gui

VPATH=$(BACK_DIR)

#
# BUILD
#

clean:
	rm -rdf $(BUILD_DIR)

$(BUILD_DIR):
	mkdir $@

$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ -c $<

#
# UTIL
#

FORCE:

#
# TEST
#

#
# FRONT
#

=======
gui: FORCE # TODO: do we really need this FORCE here?
	@python gui/cpu_monitor_gui.py

#
# BACK
#

run-back: $(BUILD_DIR)/back
	@./$<

BACK_SOURCE=$(BACK_DIR)/main.c

BACK_OBJ=$(addprefix $(BUILD_DIR)/, $(notdir $(BACK_SOURCE:.c=.o)))

$(BUILD_DIR)/back: $(BACK_OBJ) | $(BUILD_DIR)
	$(CC) $(LDFLAGS) -o $@ $^
