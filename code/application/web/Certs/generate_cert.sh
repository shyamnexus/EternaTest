#!/bin/bash
# Generate self-signed SSL certificate with Subject Alternative Names
# This script is used at build time to generate default certificates

set -e

# Directory where this script is located
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CERT_DIR="${SCRIPT_DIR}"

# Output files
CERT_FILE="${CERT_DIR}/Server.crt"
KEY_FILE="${CERT_DIR}/Server.key"
CONFIG_FILE="${CERT_DIR}/san.cnf"

# Certificate parameters (can be overridden by environment variables)
VALIDITY_DAYS="${CERT_VALIDITY_DAYS:-365}"
KEY_BITS="${CERT_KEY_BITS:-2048}"
DEFAULT_IP="${CERT_DEFAULT_IP:-192.168.1.2}"

echo "=== SSL Certificate Generation ==="
echo "Certificate Directory: ${CERT_DIR}"
echo "Validity: ${VALIDITY_DAYS} days"
echo "Key Size: ${KEY_BITS} bits"
echo "Default IP: ${DEFAULT_IP}"
echo

# Create or update san.cnf with default IP if it doesn't exist or needs updating
if [ ! -f "${CONFIG_FILE}" ]; then
    echo "Creating OpenSSL configuration file..."
    cat > "${CONFIG_FILE}" <<EOF
[req]
default_bits = ${KEY_BITS}
prompt = no
default_md = sha256
distinguished_name = dn
req_extensions = v3_req

[dn]
CN = ${DEFAULT_IP}

[v3_req]
subjectAltName = @alt_names

[alt_names]
IP.1 = ${DEFAULT_IP}
DNS.1 = localhost
EOF
    echo "Created ${CONFIG_FILE}"
fi

# Generate the certificate and key
echo "Generating SSL certificate and private key..."
openssl req -x509 -nodes \
    -days "${VALIDITY_DAYS}" \
    -newkey "rsa:${KEY_BITS}" \
    -keyout "${KEY_FILE}" \
    -out "${CERT_FILE}" \
    -config "${CONFIG_FILE}" \
    -extensions v3_req

if [ $? -eq 0 ]; then
    echo
    echo "✓ Certificate generation successful!"
    echo "  Certificate: ${CERT_FILE}"
    echo "  Private Key: ${KEY_FILE}"
    echo
    
    # Display certificate information
    echo "=== Certificate Information ==="
    openssl x509 -in "${CERT_FILE}" -noout -subject -issuer -dates
    echo
    echo "=== Subject Alternative Names ==="
    openssl x509 -in "${CERT_FILE}" -noout -text | grep -A 2 "Subject Alternative Name"
    echo
else
    echo "✗ Certificate generation failed!"
    exit 1
fi
