all: gui

#
# VARS
#

CC=clang
CFLAGS=-g -O0 -Wall -Werror -I.

LDFLAGS=

BUILD_DIR=build

BACK_DIR=back
GUI_DIR = gui
TEST_DIR = test

VPATH=$(BACK_DIR) $(TEST_DIR)

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

test: run-back-test ipc_test

#
# FRONT
#

gui: FORCE # TODO: do we really need this FORCE here?
	@python gui/cpu_monitor_gui.py

ipc_test:
	@python gui/ipc_test.py

#
# BACK
#

run-back: $(BUILD_DIR)/back
	@./$<

BACK_SOURCE=$(BACK_DIR)/main.c

BACK_OBJ=$(addprefix $(BUILD_DIR)/, $(notdir $(BACK_SOURCE:.c=.o)))

$(BUILD_DIR)/back: $(BACK_OBJ) | $(BUILD_DIR)
	$(CC) $(LDFLAGS) -o $@ $^

BACK_TEST_FILES=$(TEST_DIR)/back-test.c $(BACK_DIR)/ipc.c

$(BUILD_DIR)/back-test: CFLAGS += -DBACK_TEST
$(BUILD_DIR)/back-test: $(addprefix $(BUILD_DIR)/,$(notdir $(BACK_TEST_FILES:.c=.o)))
	$(CC) $(LDFLAGS) -o $@ $^

run-back-test: $(BUILD_DIR)/back-test
	./$<
