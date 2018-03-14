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

typedef struct {
	const char *name;
	int value;
} tuple;

const tuple flag_table[] = {
	{"PASSIVE", AI_PASSIVE},
	{"CANONNAME", AI_CANONNAME},
	{"NUMERICHOST",AI_NUMERICHOST},
	{"V4MAPPED", AI_V4MAPPED},
	{"ALL ",AI_ALL},
	{"ADDRCONFIG",AI_ADDRCONFIG},
	{"NUMERICSERV",AI_NUMERICSERV},
	{NULL, 0}
};

const tuple type_table[] = {
	{"STREAM" ,SOCK_STREAM},
	{"DGRAM",	SOCK_DGRAM},
	{"RAW", SOCK_RAW},
	{"RDM", SOCK_RDM},
	{"SEQPACKET",SOCK_SEQPACKET},
	{"DCCP",SOCK_DCCP},
	{"PACKET", SOCK_PACKET},
	{"CLOEXEC",SOCK_CLOEXEC},
	{"NONBLOCK",SOCK_NONBLOCK},
	{NULL, 0}
};

const tuple proto_table[] = {
	{"IP", IPPROTO_IP},
	{"ICMP", IPPROTO_ICMP},
	{"IGMP", IPPROTO_IGMP},
	{"IPIP", IPPROTO_IPIP},
	{"TCP", IPPROTO_TCP},
	{"EGP", IPPROTO_EGP},
	{"PUP", IPPROTO_PUP},
	{"UDP", IPPROTO_UDP},
	{"IDP", IPPROTO_IDP},
	{"TP", IPPROTO_TP},
	{"DCCP", IPPROTO_DCCP},
	{"IPV6", IPPROTO_IPV6},
	{"RSVP", IPPROTO_RSVP},
	{"GRE", IPPROTO_GRE},
	{"ESP", IPPROTO_ESP},
	{"AH", IPPROTO_AH},
	{"MTP", IPPROTO_MTP},
	{"BEETPH", IPPROTO_BEETPH},
	{"ENCAP", IPPROTO_ENCAP},
	{"PIM", IPPROTO_PIM},
	{"COMP", IPPROTO_COMP},
	{"SCTP", IPPROTO_SCTP},
	{"UDPLITE", IPPROTO_UDPLITE},
	{"MPLS", IPPROTO_MPLS},
	{"RAW", IPPROTO_RAW},
	{NULL, 0}
};

const char *tostring(const tuple table[], int value)
{
    size_t i = 0;
	while(table[i].name != NULL){
		if(value == table[i].value)
			return table[i].name;
		i++;
	}
	return "UNKNOWN";
}

int tovalue(const tuple table[], const char *name)
{
	size_t i = 0;
	while(table[i].name != NULL){
		if(!strcasecmp(name, table[i].name))
			return table[i].value;
		i++;
	}
	return 0;
}

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
                hint.ai_flags |= tovalue(flag_table, optarg);
				break;
			case 't':
				hint.ai_socktype = tovalue(type_table, optarg);
				break;
			case 'P':
				hint.ai_protocol = tovalue(proto_table, optarg);
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
		printf("%s\t", tostring(type_table,next->ai_socktype));
		printf("%s\t", tostring(proto_table,next->ai_protocol));
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