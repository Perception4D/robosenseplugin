#!/bin/bash

# Usage: ./extract_first_udp_7788_payload_tcpdump.sh input.pcap output.json

if [ $# -ne 2 ]; then
    echo "Usage: $0 input.pcap output.json"
    exit 1
fi

PCAP="$1"
OUT="$2"

# Vérifier tcpdump est installé
if ! command -v tcpdump >/dev/null 2>&1; then
    echo "Erreur: tcpdump n'est pas installé."
    exit 1
fi

###############################################
# 1) Extraire le premier paquet brut en hex
###############################################

# -xx affiche les données en hex
# -s 0 capture tout le paquet
TCPDUMP_OUTPUT=$(mktemp)
trap 'rm -f "$BIN_TEMP" "$TCPDUMP_OUTPUT"' EXIT

tcpdump -nn -r "$PCAP" -s 0 -xx "udp dst port 7788" > "$TCPDUMP_OUTPUT" 2>/dev/null
TCPDUMP_STATUS=$?

HEX_RAW=$(awk '
    /^[[:space:]]*0x[0-9a-f]+:/ { inblock=1; print; next }
    inblock && !/^[[:space:]]*0x[0-9a-f]+:/ { exit }
' "$TCPDUMP_OUTPUT")

if [ -z "$HEX_RAW" ]; then
    echo "Aucun paquet trouvé avec udp dst port 7788 dans $PCAP"
    exit 2
fi

if [ $TCPDUMP_STATUS -ne 0 ]; then
    echo "Avertissement: tcpdump a retourné $TCPDUMP_STATUS (pcap possiblement tronqué)."
fi

printf "Hex raw extrait:\\n%s\\n" "$HEX_RAW"

###############################################
# 2) Nettoyer : garder seulement les octets hex
###############################################

# Exemple d'entrée tcpdump :
#   0x0000:  45 00 00 3c ...
# On supprime les offsets et récupère uniquement les bytes.
HEX_CLEAN=$(echo "$HEX_RAW" | sed -E 's/^[[:space:]]*0x[0-9a-f]+://; s/[ \t]+/ /g' | \
            awk '{for(i=1;i<=NF;i++) printf "%s", $i} END{print ""}')

###############################################
# 3) Extraire uniquement le payload UDP
###############################################
# Nous devons ignorer :
#  - entête Ethernet : 14 octets
#  - entête IP       : IPv4 (IHL) ou IPv6 (40 octets fixes)
#  - entête UDP      : 8 octets

BIN_TEMP=$(mktemp)
echo "$HEX_CLEAN" | xxd -r -p > "$BIN_TEMP"

ETH_HEADER_LEN=14
UDP_HEADER_LEN=8

# EtherType = bytes 12-13 après Ethernet pour distinguer IPv4/IPv6
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
        echo "Type Ethernet non supporté (0x$ETH_TYPE_HEX)"
        exit 3
        ;;
esac

PAYLOAD_OFFSET=$(( ETH_HEADER_LEN + IP_HEADER_LEN + UDP_HEADER_LEN ))

# Extraire uniquement le payload UDP en hex
PAYLOAD_HEX=$(xxd -p -s $PAYLOAD_OFFSET "$BIN_TEMP" | tr -d '\n')

if [ -z "$PAYLOAD_HEX" ]; then
    echo "Payload UDP vide ou non détecté"
    exit 4
fi

# Version base64 pour plus de confort
PAYLOAD_BASE64=$(echo "$PAYLOAD_HEX" | xxd -r -p | base64 -w0)

#rm "$BIN_TEMP"

###############################################
# 4) Générer le fichier JSON
###############################################
{
    echo "{"
    echo "  \"udp_dst_port\": 7788,"
    #echo "  \"payload_hex\": \"${PAYLOAD_HEX}\","
    echo "  \"payload_base64\": \"${PAYLOAD_BASE64}\""
    echo "}"
} > "$OUT"

echo "Payload UDP extrait et écrit dans $OUT"
