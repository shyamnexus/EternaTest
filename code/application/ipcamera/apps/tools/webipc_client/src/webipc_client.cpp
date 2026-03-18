// Simple test client for the web IPC UNIX socket
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <iostream>
#include <string>
#include <vector>
#include <cstring>

static int write_n(int fd, const void *buf, int n) {
  const char *p = (const char *)buf;
  int w = 0;
  while (w < n) {
    int ret = write(fd, p + w, n - w);
    if (ret <= 0) return ret <= 0 ? ret : -1;
    w += ret;
  }
  return w;
}

static int read_n(int fd, void *buf, int n) {
  char *p = (char *)buf;
  int r = 0;
  while (r < n) {
    int ret = read(fd, p + r, n - r);
    if (ret <= 0) return ret <= 0 ? ret : -1;
    r += ret;
  }
  return r;
}

// CLI-style helpers (match server cli protocol)
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

// reuse existing read_n/write_n above

#define SOCKERR_IO -1
#define SOCKERR_CLOSED -2
#define SOCKERR_INVARG -3
#define SOCKERR_TIMEOUT -4
#define SOCKERR_OK 0

#define CS_PATH "/var/tmp/webipc"

int sock_write(int fd, const void *buff, int count) {
  const char *pts = (const char *)buff;
  int status = 0, n;
  if (count < 0) return SOCKERR_OK;
  while (status != count) {
    n = write(fd, (void *)&pts[status], count - status);
    if (n < 0) {
      if (errno == EPIPE) return SOCKERR_CLOSED;
      else if (errno == EINTR) continue;
      else return SOCKERR_IO;
    }
    status += n;
  }
  return status;
}

int sock_read(int fd, void *buff, int count) {
  char *pts = (char *)buff;
  int status = 0, n;
  if (count <= 0) return SOCKERR_OK;
  while (status != count) {
    n = read(fd, (void *)&pts[status], count - status);
    if (n < 0) {
      if (errno == EINTR) continue;
      else return SOCKERR_IO;
    }
    if (n == 0) return SOCKERR_CLOSED;
    status += n;
  }
  return status;
}

int cli_connect(const char *name) {
  int ret;
  int fd, len;
  struct sockaddr_un unix_addr;
  if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) return -1;
  memset(&unix_addr, 0, sizeof(unix_addr));
  unix_addr.sun_family = AF_UNIX;
  strncpy(unix_addr.sun_path, name, sizeof(unix_addr.sun_path)-1);
  len = sizeof(unix_addr.sun_family) + strlen(unix_addr.sun_path);
  if (connect(fd, (struct sockaddr *)&unix_addr, len) < 0) goto error;
  if (sock_read(fd, &ret, sizeof(int)) == SOCKERR_CLOSED) return -1;
  return fd;
error:
  close(fd);
  return -1;
}

int cli_begin(char *name) {
  int fd;
  int len = 0;
  fd = cli_connect(CS_PATH);
  if (fd < 0) return fd;
  len = strlen(name) + 1;
  sock_write(fd, &len, sizeof(int));
  sock_write(fd, name, len);
  return fd;
}

int cli_end(int fd) {
  int ret = 0;
  sock_read(fd, &ret, sizeof(int));
  close(fd);
  return 0;
}

static void print_buf(const char *label, const char *buf) {
  if (buf) std::cout << label << ": " << buf << std::endl;
}

int test_hon_network_ipv4_get(const char *interface) {
  int fd = cli_begin((char *)"hon_network_ipv4_get");
  if (fd < 0) { std::cerr << "cli_begin failed" << std::endl; return -1; }
  int len = 0;
  // write interface
  len = strlen(interface) + 1;
  sock_write(fd, &len, sizeof(int));
  sock_write(fd, interface, len);
  // read method
  sock_read(fd, &len, sizeof(int));
  std::vector<char> method(len+1);
  sock_read(fd, method.data(), len);
  method[len] = '\0';
  // address
  sock_read(fd, &len, sizeof(int));
  std::vector<char> address(len+1);
  sock_read(fd, address.data(), len);
  address[len] = '\0';
  // netmask
  sock_read(fd, &len, sizeof(int));
  std::vector<char> netmask(len+1);
  sock_read(fd, netmask.data(), len);
  netmask[len] = '\0';
  // gateway
  sock_read(fd, &len, sizeof(int));
  std::vector<char> gateway(len+1);
  sock_read(fd, gateway.data(), len);
  gateway[len] = '\0';
  int ret = 0;
  sock_read(fd, &ret, sizeof(int));
  cli_end(fd);
  std::cout << "hon_network_ipv4_get -> ret=" << ret << std::endl;
  print_buf("method", method.data());
  print_buf("address", address.data());
  print_buf("netmask", netmask.data());
  print_buf("gateway", gateway.data());
  return ret;
}

int test_hon_network_ipv4_set(const char *interface, const char *method, const char *address, const char *netmask, const char *gateway) {
  int fd = cli_begin((char *)"hon_network_ipv4_set");
  if (fd < 0) { std::cerr << "cli_begin failed" << std::endl; return -1; }
  int len = 0;
  // interface
  len = strlen(interface) + 1;
  sock_write(fd, &len, sizeof(int));
  sock_write(fd, interface, len);
  // method
  if (method) { len = strlen(method) + 1; sock_write(fd, &len, sizeof(int)); sock_write(fd, method, len); } else { len = 0; sock_write(fd, &len, sizeof(int)); }
  // address
  if (address) { len = strlen(address) + 1; sock_write(fd, &len, sizeof(int)); sock_write(fd, address, len); } else { len = 0; sock_write(fd, &len, sizeof(int)); }
  // netmask
  if (netmask) { len = strlen(netmask) + 1; sock_write(fd, &len, sizeof(int)); sock_write(fd, netmask, len); } else { len = 0; sock_write(fd, &len, sizeof(int)); }
  // gateway
  if (gateway) { len = strlen(gateway) + 1; sock_write(fd, &len, sizeof(int)); sock_write(fd, gateway, len); } else { len = 0; sock_write(fd, &len, sizeof(int)); }
  int ret = 0;
  sock_read(fd, &ret, sizeof(int));
  cli_end(fd);
  std::cout << "hon_network_ipv4_set -> ret=" << ret << std::endl;
  return ret;
}

int test_hon_network_dns_get() {
  int fd = cli_begin((char *)"hon_network_dns_get");
  if (fd < 0) { std::cerr << "cli_begin failed" << std::endl; return -1; }
  int len = 0;
  sock_read(fd, &len, sizeof(int));
  std::vector<char> dns1(len+1);
  sock_read(fd, dns1.data(), len);
  dns1[len] = '\0';
  sock_read(fd, &len, sizeof(int));
  std::vector<char> dns2(len+1);
  sock_read(fd, dns2.data(), len);
  dns2[len] = '\0';
  int ret = 0; sock_read(fd, &ret, sizeof(int));
  cli_end(fd);
  std::cout << "hon_network_dns_get -> ret=" << ret << std::endl;
  print_buf("dns1", dns1.data());
  print_buf("dns2", dns2.data());
  return ret;
}

int test_hon_network_dns_set(const char *dns1, const char *dns2) {
  int fd = cli_begin((char *)"hon_network_dns_set");
  if (fd < 0) { std::cerr << "cli_begin failed" << std::endl; return -1; }
  int len = 0;
  len = strlen(dns1) + 1; sock_write(fd, &len, sizeof(int)); sock_write(fd, dns1, len);
  len = strlen(dns2) + 1; sock_write(fd, &len, sizeof(int)); sock_write(fd, dns2, len);
  int ret = 0; sock_read(fd, &ret, sizeof(int)); cli_end(fd);
  std::cout << "hon_network_dns_set -> ret=" << ret << std::endl;
  return ret;
}

int test_hon_network_get_mac(const char *ifname) {
  int fd = cli_begin((char *)"hon_network_get_mac");
  if (fd < 0) { std::cerr << "cli_begin failed" << std::endl; return -1; }
  int len = 0; len = strlen(ifname) + 1; sock_write(fd, &len, sizeof(int)); sock_write(fd, ifname, len);
  sock_read(fd, &len, sizeof(int)); std::vector<char> mac(len+1); sock_read(fd, mac.data(), len); mac[len] = '\0';
  int ret = 0; sock_read(fd, &ret, sizeof(int)); cli_end(fd);
  std::cout << "hon_network_get_mac -> ret=" << ret << std::endl; print_buf("mac", mac.data()); return ret;
}

// --- C-style hon_client_* wrappers matching the reference API ---
int hon_client_get_int_by_id(char *func, int id, int *value) {
  int fd;
  int ret = 0;

  fd = cli_begin(func);
  if (fd < 0) return -1;
  /* Transmission parameters */
  sock_write(fd, &id, sizeof(int));
  sock_read(fd, value, sizeof(int));
  sock_read(fd, &ret, sizeof(int));
  /* End transmission parameters */
  ret = cli_end(fd);

  return ret;
}

int hon_client_set_int_by_id(char *func, int id, int value) {
  int fd;
  int ret = 0;

  fd = cli_begin(func);
  if (fd < 0) return -1;
  /* Transmission parameters */
  sock_write(fd, &id, sizeof(int));
  sock_write(fd, &value, sizeof(int));
  sock_read(fd, &ret, sizeof(int));
  /* End transmission parameters */
  ret = cli_end(fd);

  return ret;
}

int hon_client_get_int(char *func, int *value) {
  int fd;
  int ret = 0;

  fd = cli_begin(func);
  if (fd < 0) return -1;
  /* Transmission parameters */
  sock_read(fd, value, sizeof(int));
  sock_read(fd, &ret, sizeof(int));
  /* End transmission parameters */
  ret = cli_end(fd);

  return ret;
}

int hon_client_set_int(char *func, int value) {
  int fd;
  int ret = 0;

  fd = cli_begin(func);
  if (fd < 0) return -1;
  /* Transmission parameters */
  sock_write(fd, &value, sizeof(int));
  sock_read(fd, &ret, sizeof(int));
  /* End transmission parameters */
  ret = cli_end(fd);

  return ret;
}

int hon_client_get_string_by_id(char *func, int id, char **value) {
  int fd, len;
  int ret = 0;

  fd = cli_begin(func);
  if (fd < 0) return -1;
  /* Transmission parameters */
  sock_write(fd, &id, sizeof(id));
  sock_read(fd, &len, sizeof(int));
  sock_read(fd, *value, len);
  memset(*value + len, '\0', 1); // set terminator
  sock_read(fd, &ret, sizeof(int));
  /* End transmission parameters */
  ret = cli_end(fd);

  return ret;
}

int hon_client_set_string_by_id(char *func, int id, const char *value) {
  int fd, len;
  int ret = 0;

  fd = cli_begin(func);
  if (fd < 0) return -1;
  /* Transmission parameters */
  sock_write(fd, &id, sizeof(id));
  len = strlen(value) + 1;
  sock_write(fd, &len, sizeof(int));
  sock_write(fd, value, len);
  sock_read(fd, &ret, sizeof(int));
  /* End transmission parameters */
  ret = cli_end(fd);

  return ret;
}

int hon_client_get_string(char *func, char **value) {
  int fd, len;
  int ret = 0;

  fd = cli_begin(func);
  if (fd < 0) return -1;
  /* Transmission parameters */
  sock_read(fd, &len, sizeof(int));
  sock_read(fd, *value, len);
  memset(*value + len, '\0', 1); // set terminator
  sock_read(fd, &ret, sizeof(int));
  /* End transmission parameters */
  ret = cli_end(fd);

  return ret;
}

int hon_client_set_string(char *func, const char *value) {
  int fd, len;
  int ret = 0;

  fd = cli_begin(func);
  if (fd < 0) return -1;
  /* Transmission parameters */
  len = strlen(value) + 1;
  sock_write(fd, &len, sizeof(int));
  sock_write(fd, value, len);
  sock_read(fd, &ret, sizeof(int));
  /* End transmission parameters */
  ret = cli_end(fd);

  return ret;
}

int main(int argc, char **argv) {
  const char *iface = argc > 1 ? argv[1] : "eth0";
  test_hon_network_ipv4_get(iface);
  test_hon_network_ipv4_set((const char *)iface, "manual", "192.168.5.55", "255.255.255.0", "192.168.5.1");
  test_hon_network_dns_get();
  test_hon_network_dns_set("8.8.8.8", "8.8.4.4");
  test_hon_network_get_mac(iface);
  return 0;
}
