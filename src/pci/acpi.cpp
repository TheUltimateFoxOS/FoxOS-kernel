#include <pci/acpi.h>

using namespace pci;
using namespace pci::acpi;

//#pci::acpi::find_table_xsdt-doc: Find a table in the ACPI namespace using the xsdt header.
void* pci::acpi::find_table_xsdt(sdt_header_t* sdt_header, char* signature) {
	int entries = (sdt_header->length - sizeof(sdt_header_t)) / 8;

	for(int t = 0; t < entries; t++) {
		sdt_header_t* new_header = (sdt_header_t*) * (uint64_t*) ((uint64_t) sdt_header + sizeof(sdt_header_t) + (t * 8));
		for(int i = 0; i < 4; i++) {
			if(new_header->signature[i] != signature[i]) {
				break;
			}
			if(i == 3) {
				return new_header;
			}
		}
	}
	return 0;
}

//#pci::acpi::find_table_rsdt-doc: Find a table in the ACPI namespace using the rsdt header.
void* pci::acpi::find_table_rsdt(sdt_header_t* sdt_header, char* signature) {
	int entries = (sdt_header->length - sizeof(sdt_header_t)) / 4;

	for(int t = 0; t < entries; t++) {
		sdt_header_t* new_header = (sdt_header_t*) (uint64_t) * (uint32_t*) ((uint64_t) sdt_header + sizeof(sdt_header_t) + (t * 4));
		for(int i = 0; i < 4; i++) {
			if(new_header->signature[i] != signature[i]) {
				break;
			}
			if(i == 3) {
				return new_header;
			}
		}
	}
	return 0;
}