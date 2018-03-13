#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

const char *optstring = "46f:t:p:P:";
const char *usage = "Usage: %s [OPTIONS] <hostname>\n"
"   -4                 IPv4 results only\n"
"   -6                 IPv6 results only\n"
"   -f <ai_flags>      can be used multiple times\n"
"   -t <ai_socktype>\n"
"   -p <port>          port or service name\n"
"   -P <ai_protocol>\n";

int main(int argc, char **argv) {


	const char *host = NULL;
	const char *service = NULL;
	int ret;
	char addrbuf[INET6_ADDRSTRLEN];
	struct addrinfo hint, *result, *next;
	memset(&hint, 0, sizeof(hint));
	hint.ai_family = AF_UNSPEC;

	int opt;
	while((opt = getopt(argc, argv, optstring)) != -1){
		switch(opt){
			case '4':
				hint.ai_family = AF_INET;
				break;
			case '6':
				hint.ai_family = AF_INET6;
				break;
			case 'f':
				if(!strcasecmp(optarg, "V4MAPPED"))
					hint.ai_flags |= AI_V4MAPPED;
				if(!strcasecmp(optarg, "ADDRCONFIG"))
					hint.ai_flags |= AI_ADDRCONFIG;
				if(!strcasecmp(optarg, "ALL"))
					hint.ai_flags |= AI_ALL;
				if(!strcasecmp(optarg, "CANONNAME"))
					hint.ai_flags |= AI_CANONNAME;
				if(!strcasecmp(optarg, "NUMERICHOST"))
					hint.ai_flags |= AI_NUMERICHOST;
				if(!strcasecmp(optarg, "NUMERICSERV"))
					hint.ai_flags |= AI_NUMERICSERV;
				if(!strcasecmp(optarg, "PASSIVE"))
					hint.ai_flags |= AI_PASSIVE;
				break;
			case 't':
				if(!strcasecmp(optarg, "STREAM"))
					hint.ai_socktype = SOCK_STREAM;
				if(!strcasecmp(optarg, "DGRAM"))
					hint.ai_socktype = SOCK_DGRAM;
				if(!strcasecmp(optarg, "RAW"))
					hint.ai_socktype = SOCK_RAW;
				if(!strcasecmp(optarg, "SEQPACKET"))
					hint.ai_socktype = SOCK_SEQPACKET;
				break;
			case 'P':
				if(!strcasecmp(optarg, "IP"))
					hint.ai_protocol = IPPROTO_IP;
				if(!strcasecmp(optarg, "IPV6"))
					hint.ai_protocol = IPPROTO_IPV6;
				if(!strcasecmp(optarg, "ICMP"))
					hint.ai_protocol = IPPROTO_ICMP;
				if(!strcasecmp(optarg, "TCP"))
					hint.ai_protocol = IPPROTO_TCP;
				if(!strcasecmp(optarg, "UDP"))
					hint.ai_protocol = IPPROTO_UDP;
				if(!strcasecmp(optarg, "RAW"))
					hint.ai_protocol = IPPROTO_RAW;
				break;
			case 'p':
				service = optarg;
				break;
			default:
				printf(usage, argv[0]);
				exit(EXIT_FAILURE);
		}
	}

	if (optind >= argc) {
		printf(usage, argv[0]);
		exit(EXIT_FAILURE);
    }
	host = argv[optind];


	ret = getaddrinfo(host, service, &hint, &result);
	if(ret){
		fprintf(stderr, "%s\n", gai_strerror(ret));
		exit(EXIT_FAILURE);
	}

	next = result;
	while(next != NULL){
		switch(next->ai_socktype){
			case SOCK_STREAM:
				printf("STREAM\t");
				break;
			case SOCK_DGRAM:
				printf("DGRAM\t");
				break;
			case SOCK_RAW:
				printf("RAW\t");
				break;
			case SOCK_SEQPACKET:
				printf("SEQPACKET\t");
				break;
			default:
				printf("%d\t", next->ai_socktype);
		}
		switch(next->ai_protocol){
			case IPPROTO_IP:
				printf("IP\t");
				break;
			case IPPROTO_IPV6:
				printf("IPV6\t");
				break;
			case IPPROTO_TCP:
				printf("TCP\t");
				break;
			case IPPROTO_UDP:
				printf("UDP\t");
				break;
			case IPPROTO_ICMP:
				printf("ICMP\t");
				break;
			case IPPROTO_RAW:
				printf("RAW\t");
				break;
			default:
				printf("%d\t", next->ai_protocol);
		}
		if(next->ai_addr->sa_family == AF_INET){
			struct sockaddr_in *addr = (void *)next->ai_addr;
			printf("%hu\t", ntohs(addr->sin_port));
			printf("%s\n", inet_ntop(AF_INET, &addr->sin_addr, addrbuf, sizeof(addrbuf)));
		}
		else if(next->ai_addr->sa_family == AF_INET6){
			struct sockaddr_in6 *addr = (void *)next->ai_addr;
			printf("%hu\t", ntohs(addr->sin6_port));
			printf("%s\n", inet_ntop(AF_INET6, &addr->sin6_addr, addrbuf, sizeof(addrbuf)));
		}
		else {
			fprintf(stderr,"Unknown address family %d\n", next->ai_addr->sa_family);
		}
		next = next->ai_next;
	}

	freeaddrinfo(result);
	return 0;
}