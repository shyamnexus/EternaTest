import socket
import uuid
import re
import struct
import select
import time
import sys

# ONVIF Discovery Parameters
MULTICAST_GROUP = '239.255.255.250'
MULTICAST_PORT = 3702
PROBE_TIMEOUT = 5  # seconds

# WS-Discovery Probe Message
PROBE_TEMPLATE = """<?xml version="1.0" encoding="utf-8"?>
<Envelope xmlns:tds="http://www.onvif.org/ver10/device/wsdl" xmlns="http://www.w3.org/2003/05/soap-envelope">
  <Header>
    <wsa:MessageID xmlns:wsa="http://schemas.xmlsoap.org/ws/2004/08/addressing">urn:uuid:{uuid}</wsa:MessageID>
    <wsa:To xmlns:wsa="http://schemas.xmlsoap.org/ws/2004/08/addressing">urn:schemas-xmlsoap-org:ws:2005:04:discovery</wsa:To>
    <wsa:Action xmlns:wsa="http://schemas.xmlsoap.org/ws/2004/08/addressing">http://schemas.xmlsoap.org/ws/2005/04/discovery/Probe</wsa:Action>
  </Header>
  <Body>
    <Probe xmlns="http://schemas.xmlsoap.org/ws/2005/04/discovery" xmlns:xsd="http://www.w3.org/2001/XMLSchema" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">
      <Types>tds:NetworkVideoTransmitter</Types>
      <Scopes />
    </Probe>
  </Body>
</Envelope>"""

def send_probe(target_ip=None):
    message_id = str(uuid.uuid4())
    probe_msg = PROBE_TEMPLATE.format(uuid=message_id)
    
    # Create UDP socket
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    
    # Bind to ephemeral port
    sock.bind(('', 0))
    
    # Set timeout
    sock.settimeout(PROBE_TIMEOUT)
    
    # Send multicast
    print(f"Sending WS-Discovery Probe (MsgID: {message_id})...")
    sock.sendto(probe_msg.encode('utf-8'), (MULTICAST_GROUP, MULTICAST_PORT))
    
    # Also send unicast if target IP provided (Unicast Probe)
    if target_ip:
        print(f"Sending Unicast Probe to {target_ip}:{MULTICAST_PORT}...")
        try:
            sock.sendto(probe_msg.encode('utf-8'), (target_ip, MULTICAST_PORT))
        except Exception as e:
            print(f"Failed to send Unicast Probe: {e}")

    start_time = time.time()
    devices = []
    
    while time.time() - start_time < PROBE_TIMEOUT:
        try:
            data, addr = sock.recvfrom(65536)
            print(f"\nReceived response from {addr}:")
            response = data.decode('utf-8', errors='ignore')
            
            # Basic parsing
            xaddrs_match = re.search(r'XAddrs>([^<]+)</', response)
            scopes_match = re.search(r'Scopes>([^<]+)</', response)
            relates_to = re.search(r'RelatesTo[^>]*>([^<]+)</', response)
            
            xaddrs = xaddrs_match.group(1).strip() if xaddrs_match else "N/A"
            scopes = scopes_match.group(1).strip() if scopes_match else "N/A"
            
            print(f"  XAddrs: {xaddrs}")
            print(f"  Scopes: {scopes[:100]}..." if len(scopes) > 100 else f"  Scopes: {scopes}")
            
            devices.append({'ip': addr[0], 'xaddrs': xaddrs, 'scopes': scopes})
            
        except socket.timeout:
            break
        except Exception as e:
            print(f"Error receiving: {e}")
            break
            
    return devices

if __name__ == "__main__":
    target = sys.argv[1] if len(sys.argv) > 1 else "192.168.0.3"
    print(f"Starting WS-Discovery verification. Unicast Target: {target}")
    found = send_probe(target)
    
    print("\n" + "="*50)
    print(f"Found {len(found)} device(s).")
    for d in found:
        print(f"- {d['ip']} | {d['xaddrs']}")

    # Validation Logic
    if not found:
        print("\n[FAIL] No devices found via WS-Discovery.")
        sys.exit(1)
        
    my_device = next((d for d in found if d['ip'] == target), None)
    if my_device:
        print(f"\n[PASS] Target device {target} discovered.")
        # Check XAddrs
        if target not in my_device['xaddrs']:
             print(f"[WARN] XAddrs {my_device['xaddrs']} does not contain device IP {target}. This is a configuration error if NAT is not involved.")
        else:
             print(f"[PASS] XAddrs matches device IP.")
    else:
        print(f"\n[FAIL] Target device {target} NOT found (others were: {[d['ip'] for d in found]}).")
