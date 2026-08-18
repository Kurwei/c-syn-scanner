#define __FAVOR_BSD
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>

unsigned short csum(unsigned short *ptr, int nbytes) {
    register long sum;
    unsigned short oddbyte;
    register short answer;

    sum = 0;
    while (nbytes > 1) {
        sum += *ptr++;
        nbytes -= 2;
    }
    if (nbytes == 1) {
        oddbyte = 0;
        *((u_char*)&oddbyte) = *(u_char*)ptr;
        sum += oddbyte;
    }
    sum = (sum >> 16) + (sum & 0xffff);
    sum = sum + (sum >> 16);
    answer = (short)~sum;
    return(answer);
}


int main(int argc, char const *argv[]){
      if (argc != 5) {
    printf("Error: Invalid parameters!\n");
    printf("Usage: %s <source hostname/IP> <source port> <target hostname/IP> <target port>\n", argv[0]);
    exit(1);
  }
  u_int16_t src_port, dst_port;
  u_int32_t src_addr, dst_addr;
  src_addr = inet_addr(argv[1]);
  dst_addr = inet_addr(argv[3]);
  src_port = atoi(argv[2]);
  dst_port = atoi(argv[4]);

  int sock = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
  if(sock < 0){
    perror("There was an error with the socket don't forget to start the program with sudo\n");
    exit(1);
  }

  int one = 1;
  const int *val = &one;
  if (setsockopt(sock, IPPROTO_IP, IP_HDRINCL, val, sizeof(one)) < 0) {
      perror("setsockopt() error!\n");
      exit(1);
  }

  char datagram[4096];
  memset(datagram, 0, 4096);

  struct iphdr *iph = (struct iphdr *) datagram;
  struct tcphdr *thdr = (struct tcphdr *) (datagram + sizeof(struct iphdr));

  iph->ihl = 5;
  iph->version = 4;
  iph->tos = 0;
  iph->tot_len = sizeof(struct iphdr) + sizeof(struct tcphdr);
  iph->protocol = IPPROTO_TCP;
  iph->id = htons(42420);
  iph->frag_off = 0;
  iph->ttl = 255;
  iph->check = 0;
  iph->saddr = src_addr;
  iph->daddr = dst_addr;

  thdr->source = htons(src_port);
  thdr->dest = htons(dst_port);

  thdr->seq = htonl(424242420);
  thdr->ack_seq = 0;
  thdr->doff = 5;

  thdr->fin = 0;
  thdr->syn = 1;
  thdr->rst = 0;
  thdr->psh = 0;
  thdr->ack = 0;
  thdr->urg = 0;

  thdr->window = htons(1010);
  thdr->check = 0;
  thdr->urg_ptr = 0;

  struct sockaddr_in sin;
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = iph->daddr;
  sin.sin_port = thdr->dest;

  struct pseudo_header {
        u_int32_t source_address;
        u_int32_t dest_address;
        u_int8_t placeholder;
        u_int8_t protocol;
        u_int16_t tcp_length;
    };

  struct pseudo_header psh;
  
  psh.source_address = src_addr;
  psh.dest_address = dst_addr;
  psh.placeholder = 0;
  psh.protocol = IPPROTO_TCP;
  psh.tcp_length = htons(sizeof(struct tcphdr));

  int psize = sizeof(struct pseudo_header) + sizeof(struct tcphdr);
  char *pseudogram = malloc(psize);

  memcpy(pseudogram, (char *)&psh, sizeof(struct pseudo_header));
  memcpy(pseudogram + sizeof(struct pseudo_header), thdr, sizeof(struct tcphdr));

  thdr->check = csum((unsigned short *)pseudogram, psize);

  if(sendto(sock, datagram, iph->tot_len, 0, (struct sockaddr *) &sin, sizeof(sin)) < 0){
    perror("Packet couldn't send!\n");
  } else {
    printf("Succesful\n");
  }
  free(pseudogram);

  char recv_buffer[4096];
  struct sockaddr_in saddr;
  socklen_t saddr_len = sizeof(saddr);
  while(1){
    ssize_t data_size = recvfrom(sock, recv_buffer, 4096, 0, (struct sockaddr *)&saddr, &saddr_len);
    if(data_size < 0){
      perror("Packet read error");
      break;
    }
    struct iphdr *recv_iph = (struct iphdr *)recv_buffer;
    if (recv_iph->saddr == dst_addr){
      int ip_hdr_len = recv_iph->ihl * 4;
      struct tcphdr *recv_tcph = (struct tcphdr *)(recv_buffer + ip_hdr_len);
      if (ntohs(recv_tcph->dest) == src_port) {
        if (recv_tcph->syn == 1 && recv_tcph->ack == 1){
          printf("[+] Port %d is open", dst_port);
          break;
        }
        else if(recv_tcph->rst == 1){
          printf("[-] Port %d is closed", dst_port);
          break;
        }
      }
    }
  }
  close(sock);  
  return(0);

}