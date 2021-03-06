#include <stdio.h>
#include <arpa/inet.h>
#include <pcap.h>
#include <sys/time.h>
#include <time.h>

int main(int argc, char *argv[])
{
	char *dev;
	char  errbuf[PCAP_ERRBUF_SIZE];
	bpf_u_int32 mask;
	bpf_u_int32 net;
	struct in_addr addr;
	char *net_c;
	char *mask_c;
	struct pcap_pkthdr header;
	const u_char *packet;
	pcap_t *handle;

	struct tm *now_tm;
	time_t now;

	struct bpf_program fp;
 	char filter_exp[] = "tcp port 80";

	// dev = pcap_lookupdev(errbuf);
	// test on my computer
	dev = "eth1";
	if (dev == NULL) {
		fprintf(stderr, "Couldn't find default device: %s\n", errbuf);
		return(2);
	}
	printf("Device: %s\n", dev);

	if (pcap_lookupnet(dev, &net, &mask, errbuf) == -1) {
		fprintf(stderr, "Can't get netmask for device %s\n", dev);
		net_c = 0;
		mask_c = 0;
		return(2);
 	}
	addr.s_addr = net;
	net_c = inet_ntoa(addr);
	printf("Net: %s\n", net_c);

	addr.s_addr = mask;
		mask_c = inet_ntoa(addr);
	printf("Mask: %s\n",mask_c);

	printf("==================================================\n");

	handle = pcap_open_live(dev, BUFSIZ, 1, 0, errbuf);
	if (handle == NULL) {
		fprintf(stderr, "Couldn't open device %s: %s\n", dev, errbuf);
		return(2);
	}

	if (pcap_compile(handle, &fp, filter_exp, 0, net) == -1) {
		fprintf(stderr, "Couldn't parse filter %s: %s\n", filter_exp, pcap_geterr(handle));
		return(2);
	}
	if (pcap_setfilter(handle, &fp) == -1) {
	 	fprintf(stderr, "Couldn't install filter %s: %s\n", filter_exp, pcap_geterr(handle));
	 	return(2);
 	}
	packet = pcap_next(handle, &header);

	printf("Jacked one packet at %s\n", filter_exp);
	/*
	 * Each packet in the dump file is prepended with this generic header.
	 * This gets around the problem of different headers for different
	 * packet interfaces.
	 * struct pcap_pkthdr {
 	 *         struct timeval ts;      * time stamp *
	 *         bpf_u_int32 caplen;     * length of portion present *
	 *         bpf_u_int32 len;        * length this packet (off wire) *
	 * };
	 */
	printf("Length of this packet is [%d] bytes\n", header.len);

	now = header.ts.tv_sec;
	now_tm = localtime(&now);
	printf("Time Stamp of this packet is [%d/%2d/%2d %2d:%2d:%2d]\n",
		now_tm->tm_year+1900, now_tm->tm_mon+1, now_tm->tm_mday,
		now_tm->tm_hour, now_tm->tm_min, now_tm->tm_sec);

	printf("Recieved at ..... %s\n",ctime((const time_t*)&header.ts.tv_sec));

	pcap_close(handle);
	return(0);
}
