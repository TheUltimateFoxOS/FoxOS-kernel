#include <scheduling/scheduler/scheduler.h>
#include <net/net_stack.h>

#include <driver/serial.h>

void test_net(char* _) {
	new_task((void*) (void_function) []() {
		net::net_stack_t* net_stack = driver::nic::global_nic_manager->get_nic(0)->network_stack;
		
		// resolve discord.com ip and send a ping
		driver::nic::ip_u google_ip;
		google_ip.ip = net_stack->dns->resolve((char*) "www.google.com");

		driver::global_serial_driver->printf("Resolved www.google.com to %d.%d.%d.%d\n", google_ip.ip_p[0], google_ip.ip_p[1], google_ip.ip_p[2], google_ip.ip_p[3]);
	
		char http_request[] = "GET / HTTP/1.1\r\nHost: www.google.com\r\n\r\n";
		net::TcpSocket* google_http_socket = net_stack->tcp->connect(google_ip.ip, 80);
		google_http_socket->send((uint8_t*)http_request, strlen(http_request));
	});
}