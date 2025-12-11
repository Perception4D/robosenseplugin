#!/bin/bash

# Usage: ./extract_calibration.sh input.pcap output.json

if [ $# -ne 2 ]; then
    echo "Calibration packet extraction from Robosense PCAP file"
    echo "Usage: $0 input.pcap output-calibration.json"
    exit 1
fi

PCAP="$1"
OUT="$2"

# Check tcpdump is installed
if ! command -v tcpdump >/dev/null 2>&1; then
    echo "Error: tcpdump is not installed."
    echo "Please install tcpdump to use this script: $ sudo apt-get install tcpdump"
    exit 1
fi

###############################################
# 1) Extract the first raw packet in hex
###############################################

# -xx prints data in hex
# -s 0 captures the full packet
TCPDUMP_OUTPUT=$(mktemp)
trap 'rm -f "$BIN_TEMP" "$TCPDUMP_OUTPUT"' EXIT

tcpdump -nn -r "$PCAP" -s 0 -xx "udp dst port 7788" > "$TCPDUMP_OUTPUT" 2>/dev/null
TCPDUMP_STATUS=$?

HEX_RAW=$(awk '
    /^[[:space:]]*0x[0-9a-f]+:/ { inblock=1; print; next }
    inblock && !/^[[:space:]]*0x[0-9a-f]+:/ { exit }
' "$TCPDUMP_OUTPUT")

if [ -z "$HEX_RAW" ]; then
    echo "No packet found with UDP destination port 7788 in $PCAP"
    exit 2
fi

if [ $TCPDUMP_STATUS -ne 0 ]; then
    echo "Warning: tcpdump returned $TCPDUMP_STATUS (pcap may be truncated)."
fi

printf "Extracted raw hex:\\n%s\\n" "$HEX_RAW"

###############################################
# 2) Clean: keep only hex bytes
###############################################

# tcpdump example input:
#   0x0000:  45 00 00 3c ...
# Remove offsets and keep only bytes.
HEX_CLEAN=$(echo "$HEX_RAW" | sed -E 's/^[[:space:]]*0x[0-9a-f]+://; s/[ \t]+/ /g' | \
            awk '{for(i=1;i<=NF;i++) printf "%s", $i} END{print ""}')

###############################################
# 3) Extract only the UDP payload
###############################################
# We must skip:
#  - Ethernet header: 14 bytes
#  - IP header      : IPv4 (IHL) or IPv6 (fixed 40 bytes)
#  - UDP header     : 8 bytes

BIN_TEMP=$(mktemp)
echo "$HEX_CLEAN" | xxd -r -p > "$BIN_TEMP"

ETH_HEADER_LEN=14
UDP_HEADER_LEN=8

# EtherType = bytes 12-13 after Ethernet to distinguish IPv4/IPv6
ETH_TYPE_HEX=$(xxd -p -l 2 -s 12 "$BIN_TEMP")

case "$ETH_TYPE_HEX" in
    0800) # IPv4
        IHL_HEX=$(xxd -p -l 1 -s $ETH_HEADER_LEN "$BIN_TEMP")
        IHL=$(( 0x$IHL_HEX & 0x0F ))
        IP_HEADER_LEN=$(( IHL * 4 ))
        ;;
    86dd|86DD) # IPv6
        IP_HEADER_LEN=40
        ;;
    *)
        echo "Unsupported EtherType (0x$ETH_TYPE_HEX)"
        exit 3
        ;;
esac

PAYLOAD_OFFSET=$(( ETH_HEADER_LEN + IP_HEADER_LEN + UDP_HEADER_LEN ))

# Extract only the UDP payload in hex
PAYLOAD_HEX=$(xxd -p -s $PAYLOAD_OFFSET "$BIN_TEMP" | tr -d '\n')

if [ -z "$PAYLOAD_HEX" ]; then
    echo "UDP payload empty or not detected"
    exit 4
fi

# Generate the Base64 version for JSON writing
PAYLOAD_BASE64=$(echo "$PAYLOAD_HEX" | xxd -r -p | base64 -w0)

rm "$BIN_TEMP"

###############################################
# 4) Generate the JSON file
###############################################
{
    echo "{"
    echo "  \"udp_dst_port\": 7788,"
    echo "  \"payload_base64\": \"${PAYLOAD_BASE64}\""
    echo "}"
} > "$OUT"

echo "Done. Calibration payload extracted and written to file $OUT"
