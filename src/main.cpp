#include <util.h>
#include <bootinfo.h>
#include <kernel_info.h>
#include <config.h>

#include <renderer/font_renderer.h>
#include <renderer/mouse_renderer.h>

#include <paging/page_frame_allocator.h>

#include <driver/driver.h>
#include <driver/keyboard.h>
#include <driver/mouse.h>
#include <driver/serial.h>
#include <driver/pc_speaker.h>
#include <driver/driver.h>
#include <driver/disk/ata.h>
#include <driver/disk/disk.h>

#include <shell/shell.h>

#include <scheduling/scheduler/scheduler.h>
#include <init/init_procces.h>

#include <apic/apic.h>

#include <fs/fat32/vfs.h>
#include <fs/stivale/vfs.h>
#include <fs/vfs/vfs.h>

#include <stivale2.h>
#include <cmdline.h>
#include <kmod.h>

#include <driver/nic/nic.h>
#include <net/etherframe.h>
#include <net/arp.h>
#include <net/ipv4.h>
#include <net/icmp.h>
#include <net/udp.h>
#include <net/dhcp.h>
#include <net/dns.h>
#include <net/tcp.h>

#include "examples/examples.h"

class PrintfKeyboardEventHandler : public driver::KeyboardEventHandler {
	public:
		//#KeyDown-doc: Handle keyboard events.
		void KeyDown(char c) {
			shell::global_shell->keypress(c);
		}

		//#SpecialKeyDown-doc: Handle a special key being pressed.
		void SpecialKeyDown(driver::special_key key) {
			
		}

		//#SpecialKeyUp-doc: Handle a special key being released.
		void SpecialKeyUp(driver::special_key key) {
			
		}
};

class MouseRendererMouseEventHandler : public driver::MouseEventHandler {
	public:
		//#OnMouseDown-doc: Mouse down event handler.
		void OnMouseDown(uint8_t button) {
			renderer::global_mouse_renderer->on_mouse_down(button);
		}

		//#OnMouseMove-doc: Mouse move event handler.
		void OnMouseMove(uint8_t mouse_packet[4]) {
			renderer::global_mouse_renderer->on_mouse_move(mouse_packet);
		}
};

class TcpMsgHandler : public net::TcpHandler {
	public:
		//#onTcpMessage-doc: Handle a message from a TCP socket just here for testing.
		bool onTcpMessage(net::TcpSocket* socket, uint8_t* data, size_t size) {
			driver::global_serial_driver->printf("TCP PACKET: ");
			for(int i = 0; i < size; i++) {
				driver::global_serial_driver->printf("%c", data[i]);
			}
			driver::global_serial_driver->printf("\n");

			return true;
		}
};

int crashc = 0;

//#crash-doc: Test function to crash the kernel it is recursive to test the stack tracing.
void crash() {
	if(crashc == 100) {
		*((uint32_t*) 0xff00ff00ff00) = 0;
	} else {
		crashc++;
		crash();
	}
}

//#kernel_main-doc: Main function of the kernel.
extern "C" void kernel_main(stivale2_struct* bootinfo) {
	global_bootinfo = bootinfo;

	KernelInfo kernel_info = init_kernel(bootinfo);
	PageTableManager* page_table_manager = kernel_info.page_table_manager;

	//Keyboard driver
	PrintfKeyboardEventHandler kbhandler;
	driver::KeyboardDriver keyboard_driver(&kbhandler);
	driver::global_driver_manager->add_driver(&keyboard_driver);

	//Mouse driver
	MouseRendererMouseEventHandler mhandler;
	driver::MouseDriver mouse_driver(&mhandler);
	driver::global_driver_manager->add_driver(&mouse_driver);

	//PC speaker driver
	driver::PcSpeakerDriver pc_speaker_driver;
	driver::global_driver_manager->add_driver(&pc_speaker_driver);

	//Activate drivers
	renderer::global_font_renderer->printf("\n");
	driver::global_driver_manager->activate_all(false);

	renderer::global_font_renderer->printf("\nFoxOS version %s %d Copyright (C) 2021 %s\n", RELEASE_T, VERSION, VENDOR);
	renderer::global_font_renderer->printf("This program comes with ABSOLUTELY NO WARRANTY.\n");
	renderer::global_font_renderer->printf("This is free software, and you are welcome to redistribute it.\n\n");

	if (driver::disk::global_disk_manager->num_disks != 0) {

		vfs_mount* fat_mount = initialise_fat32(0);
		mount(fat_mount, (char*) "root");
		
	} else {
		renderer::global_font_renderer->printf("No physical disks found!\n");
	}

	vfs_mount* stivale_mount = initialise_stivale_modules(bootinfo);
	mount(stivale_mount, (char*) "stivale");


	//Command line parser to run tests
	CmdLineParser cmd_line_parser;

	cmd_line_parser.add_handler((char*) "--patch-test", test_patch);
	cmd_line_parser.add_handler((char*) "--disk-test", disk_test);
	cmd_line_parser.add_handler((char*) "--fat32-test", fat32_test);
	cmd_line_parser.add_handler((char*) "--fat32-old-test", fat32_old_test);
	cmd_line_parser.add_handler((char*) "--syscall-test", syscall_test);
	cmd_line_parser.add_handler((char*) "--scheduler-test", test_scheduler);
	cmd_line_parser.add_handler((char*) "--sound-test", test_sound);
	cmd_line_parser.add_handler((char*) "--layer-test", layer_test);
	cmd_line_parser.add_handler((char*) "--vfs-test", vfs_test);
	cmd_line_parser.add_handler((char*) "--autoexec", set_autoexec);
	cmd_line_parser.add_handler((char*) "--no-smp", set_no_smp_shed);
	cmd_line_parser.add_handler((char*) "--load-mod", load_module);

	stivale2_struct_tag_cmdline* cmdline = stivale2_tag_find<stivale2_struct_tag_cmdline>(bootinfo, STIVALE2_STRUCT_TAG_CMDLINE_ID);
	cmd_line_parser.parse((char*) cmdline->cmdline);

	if (!NO_SMP_SHED) {
		start_all_cpus(bootinfo);
	}

	//font_renderer_test();
	//renderer::global_font_renderer->printf("RSDP: %f0x%x%r\n", 0xffff00ff, bootinfo->rsdp);
	
	//fe_test();
	//test_patch();
	//disk_test();
	//fat32_test();
	//syscall_test();
	//test_scheduler();

	//layer_test(bootinfo);
	//test_sound();

	for (int i = 0; i < driver::nic::global_nic_manager->num_Nics; i++) {
		renderer::global_font_renderer->printf("Configuring NIC %d... ", i);

		net::EtherFrameProvider* ether = new net::EtherFrameProvider(i);
		net::AddressResolutionProtocol* arp = new net::AddressResolutionProtocol(ether);
		net::Ipv4Provider* ipv4 = new net::Ipv4Provider(ether, arp, 0xffffffff, 0xffffffff);
		net::IcmpProvider* icmp = new net::IcmpProvider(ipv4);
		net::UdpProvider* udp = new net::UdpProvider(ipv4);
		net::TcpProvider* tcp = new net::TcpProvider(ipv4);


		net::UdpSocket* dhcp_socket = udp->connect(0xffffffff, 67);
		net::DhcpProtocol* dhcp = new net::DhcpProtocol(dhcp_socket);
		udp->bind(dhcp_socket, dhcp);

		dhcp->request();

		driver::nic::global_nic_manager->get_nic(i)->set_ip(dhcp->ip);
		ipv4->gateway_ip_be = dhcp->gateway;
		ipv4->subnet_mask_be = dhcp->subnet;

		delete dhcp;
		delete dhcp_socket;

		arp->broadcast_mac(ipv4->gateway_ip_be);

		driver::nic::ip_u ip;
		ip.ip = driver::nic::global_nic_manager->get_nic(i)->get_ip();

		driver::nic::ip_u gateway;
		gateway.ip = ipv4->gateway_ip_be;

		driver::nic::ip_u subnet;
		subnet.ip = ipv4->subnet_mask_be;

		driver::nic::ip_u dns_ip;
		dns_ip.ip = dhcp->dns;

		net::UdpSocket* dns_socket = udp->connect(dns_ip.ip, 53);
		net::DomainNameServiceProvider* dns = new net::DomainNameServiceProvider(dns_socket);
		udp->bind(dns_socket, dns);

		renderer::global_font_renderer->printf("%fDone%r. ip: %d.%d.%d.%d, gateway: %d.%d.%d.%d, dns: %d.%d.%d.%d", 0xff00ff00, ip.ip_p[0], ip.ip_p[1], ip.ip_p[2], ip.ip_p[3], gateway.ip_p[0], gateway.ip_p[1], gateway.ip_p[2], gateway.ip_p[3], dns_ip.ip_p[0], dns_ip.ip_p[1], dns_ip.ip_p[2], dns_ip.ip_p[3]);
		renderer::global_font_renderer->printf("\n");

		// uint32_t ip_of_google = dns->resolve((char*) "google.com");
		// driver::nic::ip_u ip_of_google_u;
		// ip_of_google_u.ip = ip_of_google;

		// driver::global_serial_driver->printf("Resolved google.com to %d.%d.%d.%d\n", ip_of_google_u.ip_p[0], ip_of_google_u.ip_p[1], ip_of_google_u.ip_p[2], ip_of_google_u.ip_p[3]);

		// icmp->send_echo_request(ip_of_google);

		// net::TcpSocket* socket = tcp->connect(ip_of_google_u.ip, 80);
		// TcpMsgHandler tcphandler;
		// tcp->bind(socket, &tcphandler);
		// const char* http = "GET / HTTP/1.1\r\nHost: www.google.com\r\n\r\n";
		// socket->send((uint8_t*)http, strlen((char*)http));
	}

	run_on_ap([]() {
		driver::global_serial_driver->printf("Hello ap world!\n");
	});

	task* init_procces_task = new_task((void*) init_procces);

	//run_on_ap(crash);

	wait_for_aps();

	init_sched();

	while (true) {
		asm ("hlt");
	}
}
