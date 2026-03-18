# Software Bill of Materials (SBOM)
## IP Camera Firmware - Novatek NS02302

**Generated:** 22 December 2025  
**Project:** ipcamera_fw_nvt  
**Platform:** ARM64 aarch64-ca53

---

## Applications

| Component | Version | Source | License | Purpose |
|-----------|---------|--------|---------|---------|
| dhd_priv | - | External | - | Broadcom wireless driver utility |
| memtester | 4.3.0 | External | GPL-2.0 | Memory testing utility |
| iperf-3 | 3.0.12 | External | BSD-3-Clause | Network performance testing |
| nginx | 1.29.0 | External | BSD-2-Clause | Web server |
| adbd | 4.2.2 (android-tools) | External | Apache-2.0 | Android Debug Bridge daemon |
| netsnmp | 5.9.4 (net-snmp) | External | BSD-3-Clause | SNMP agent and tools |
| openssh | 10.2 | External | BSD | Secure shell server/client |
| avahi | 0.8 | External | LGPL-2.1 | mDNS/DNS-SD service discovery |

---

## Libraries

| Component | Version | Source | License | Purpose |
|-----------|---------|--------|---------|---------|
| openssl | 3.5.4 | External Lib | Apache-2.0 | Cryptography and SSL/TLS |
| libnl | 3.2.27 | External Lib | LGPL-2.1 | Netlink protocol library |
| libuvc | 0.0.6 | External Lib | BSD-3-Clause | USB Video Class driver |
| alsa-lib | 1.1.9 | External Lib | LGPL-2.1 | Advanced Linux Sound Architecture |
| sqlcipher | 4.11.0 | External Lib | BSD-3-Clause | SQLite with encryption |
| curl | 8.16.0 | External Lib | curl | HTTP/HTTPS client library |
| miniupnpc | 2.3.3 | External Lib | BSD-3-Clause | UPnP client library |
| civetweb | 1.16 | External Lib | MIT | Embedded web server library |
| freetype | 2.14.1 | External Lib | FTL/GPL-2.0 | Font rendering library |

## Dependencies Graph

```
nginx → openssl → (cryptographic operations)
curl → openssl → (HTTPS requests)
sqlcipher → openssl → (database encryption)
civetweb → openssl → (HTTPS server)
openssh → openssl → (SSH encryption)
netsnmp → openssl → (SNMP v3 encryption)

libuvc → libusb → (USB device access)
alsa-utils → alsa-lib → (audio hardware)

wpa_supplicant → libnl → (netlink communication)
hostapd → libnl → (wireless configuration)
iproute2 → libnl → (network configuration)
```
