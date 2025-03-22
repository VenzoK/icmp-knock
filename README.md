icmp-knock: ICMP-based Utility for OpenWRT and Linux

icmp-knock is a utility designed to trace the network path to a destination using raw ICMP Echo Request packets (ping). It is similar to the traditional traceroute tool, providing detailed information about intermediate hops, including round-trip time (RTT) for each hop.

This utility works on both Linux and OpenWRT.

Overview:

icmp-knock works by sending ICMP Echo Request packets to a destination, incrementing the TTL (Time-To-Live) for each packet. This allows it to trace the network route and measure the RTT for each hop. It also attempts to resolve Fully Qualified Domain Names (FQDNs) for each hop when possible.

---

Features:
- Traces network routes using ICMP Echo Requests.
- Measures round-trip time (RTT) for each hop.
- Resolves FQDNs for hops (if resolvable).
- Configurable interface binding for OpenWRT.
- Supports both Linux and OpenWRT systems.

---

Installation:

Linux:
1. Clone the repository:
   git clone https://github.com/VenzoK/icmp-knock.git
   cd icmp-knock

2. Build the utility:
   make

3. To clean the build:
   make clean

After building, the executable icmp-knock will be in the current directory.

OpenWRT:
1. Clone the repository:
   git clone https://github.com/VenzoK/icmp-knock.git
   cd icmp-knock

2. Copy the openwrt/Makefile and icmp-knock.c into the OpenWRT package/icmp-knock/ directory.

3. Build the package using OpenWRT's build system:
   make package/icmp-knock/compile

4. After building, the icmp-knock binary will be available in your OpenWRT bin directory.

---

Usage:

Linux Usage:
To run icmp-knock, use the following command:
   ./icmp-knock <destination_FQDN> [interface]

   - <destination_FQDN>: The Fully Qualified Domain Name (FQDN) or IP address of the target (e.g., google.com).
   - [interface]: (Optional) The network interface to bind to, like eth0 or wlan0. If not specified, the default interface is used.

Example:
   ./icmp-knock google.com

OpenWRT Usage:
To run on OpenWRT, use the same command:
   icmp-knock <destination_FQDN> [interface]

Example:
   icmp-knock google.com

To specify a network interface:
   icmp-knock google.com eth0

---

Test Script:

A test script is available to compare the output of icmp-knock with traceroute.

To run the test script:
1. Ensure icmp-knock and traceroute are installed.
2. Run the test script with a target destination:
   ./test/traceroute_test.sh <target_FQDN>

The script runs both utilities and outputs their results for comparison.

---