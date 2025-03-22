CC=gcc
CFLAGS=-g -Wall
SRC=icmp-knock.c
OUT=icmp-knock
MAX_HOPS=30

all:
	@$(CC) $(CFLAGS) $(SRC) -o $(OUT)

clean:
	@rm -f $(OUT)

test:
	@if [ ! -f ./$(OUT) ]; then \
		@echo "Error: $(OUT) is not present in the currect directory."; \
		@exit 1; \
	fi
	@echo "Running icmp-knock utility..."
	@sudo ./icmp-knock $(TARGET) > icmp-knock_output.txt
	@echo "Running traceroute..."
	@sudo traceroute -I -m $(MAX_HOPS) $(TARGET) > traceroute_output.txt
	@echo "icmp-knock output:"
	@cat icmp-knock_output.txt
	@echo "traceroute output:"
	@cat traceroute_output.txt
	@rm icmp-knock_output.txt traceroute_output.txt
help:
	@echo "Makefile Targets:"
	@echo "  all         - Compile the source code to create $(OUT)."
	@echo "  clean       - Remove the compiled binary $(OUT)."
	@echo "  test        - Run the icmp-knock utility and traceroute test."
	@echo "  install     - Install $(OUT) to /usr/local/bin."
	@echo "  uninstall   - Remove $(OUT) from /usr/local/bin."
	@echo "  help        - Display this help message."

install:
	@echo "Installing $(OUT) to /usr/local/bin..."
	@sudo cp $(OUT) /usr/local/bin/$(OUT)
	@sudo chmod +x /usr/local/bin/$(OUT)
	@echo "$(OUT) installed successfully to /usr/local/bin."

uninstall:
	@echo "Uninstalling $(OUT) from /usr/local/bin..."
	@sudo rm -f /usr/local/bin/$(OUT)
	@echo "$(OUT) has been uninstalled from /usr/local/bin."

