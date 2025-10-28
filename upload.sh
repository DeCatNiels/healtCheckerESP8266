#!/bin/bash
# ESP8266 Upload and Monitor Script

# Configuration
SKETCH_DIR="healthChecker"
BOARD_FQBN="esp8266:esp8266:nodemcuv2"
PORT="/dev/ttyUSB0"
BAUDRATE="115200"

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Check if sketch directory is provided as argument
if [ -n "$1" ]; then
    SKETCH_DIR="$1"
fi

echo -e "${YELLOW}=== ESP8266 Upload Script ===${NC}"
echo "Sketch: $SKETCH_DIR"
echo "Board: $BOARD_FQBN"
echo "Port: $PORT"
echo ""

# Step 1: Compile
echo -e "${YELLOW}[1/3] Compiling sketch...${NC}"
if ./bin/arduino-cli compile --fqbn "$BOARD_FQBN" "$SKETCH_DIR/"; then
    echo -e "${GREEN}✓ Compilation successful${NC}\n"
else
    echo -e "${RED}✗ Compilation failed${NC}"
    exit 1
fi

# Step 2: Upload
echo -e "${YELLOW}[2/3] Uploading to ESP8266...${NC}"
if ./bin/arduino-cli upload -p "$PORT" --fqbn "$BOARD_FQBN" "$SKETCH_DIR/"; then
    echo -e "${GREEN}✓ Upload successful${NC}\n"
else
    echo -e "${RED}✗ Upload failed${NC}"
    echo -e "${YELLOW}Tip: Make sure no serial monitor is open and the board is connected${NC}"
    exit 1
fi

# Step 3: Monitor
echo -e "${YELLOW}[3/3] Opening serial monitor...${NC}"
echo -e "${GREEN}Press Ctrl+C to exit${NC}\n"
sleep 1

# Colorize serial monitor output
./bin/arduino-cli monitor -p "$PORT" -c baudrate="$BAUDRATE" | while IFS= read -r line; do
    # Color patterns
    if [[ "$line" =~ "ONLINE" ]] || [[ "$line" =~ "✓" ]] || [[ "$line" =~ "Connected" ]] || [[ "$line" =~ "success" ]]; then
        echo -e "${GREEN}${line}${NC}"
    elif [[ "$line" =~ "OFFLINE" ]] || [[ "$line" =~ "ALERT" ]] || [[ "$line" =~ "DOWN" ]] || [[ "$line" =~ "✗" ]] || [[ "$line" =~ "Failed" ]] || [[ "$line" =~ "failed" ]] || [[ "$line" =~ "Error" ]] || [[ "$line" =~ "error" ]]; then
        echo -e "${RED}${line}${NC}"
    elif [[ "$line" =~ "Checking" ]] || [[ "$line" =~ "Sending" ]] || [[ "$line" =~ "Email" ]] || [[ "$line" =~ "SMTP" ]] || [[ "$line" =~ "===" ]]; then
        echo -e "${YELLOW}${line}${NC}"
    else
        echo "$line"
    fi
done