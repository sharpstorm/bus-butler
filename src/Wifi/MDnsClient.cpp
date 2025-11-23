#include "MDnsClient.h"

#include "mdns.h"

void MDnsClient::start() {
  mdns_init();
  mdns_hostname_set(MDNS_HOSTNAME);
  mdns_instance_name_set(MDNS_HOSTNAME);
  mdns_service_add(MDNS_HOSTNAME, "_http", "_tcp", 80, NULL, 0);
}

void MDnsClient::stop() { mdns_free(); }
