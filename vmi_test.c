#include <xenstat.h>
#include <stdio.h>
#include <malloc.h>

int main(int argc, char *argv[])
{
    xenstat_domain **domains;
    unsigned int num_domains = 0;
    xenstat_handle *xhandle = NULL;
    xhandle = xenstat_init();
    
    xenstat_node *cur_node = xenstat_get_node(xhandle, XENSTAT_ALL);
    
    num_domains = xenstat_node_num_domains(cur_node);
    domains = calloc(num_domains, sizeof(xenstat_domain *));
    if(domains == NULL)
	fprintf(stderr, "Failed to allocate memory\n");
    int k;
    for (k=0; k < num_domains; k++)
		domains[k] = xenstat_node_domain_by_index(cur_node, k);
//    struct xenstat_node node;
//    i = xenstat_node_cpu_hz(cur_node);
//    printf("cpu_hz:%u\n", i);

    // Network
    xenstat_network *network;
    unsigned num_networks = 0;
    unsigned int first_domain_index = 0;
    int i,j;
    for (i = first_domain_index; i < num_domains; i++)
    {
	printf("Domain %d:--->> %s\n", i, xenstat_domain_name(domains[i]));
	unsigned long long ns = xenstat_domain_cpu_ns(domains[i]);
	printf("cpu_ns:%u\n", ns);
	unsigned long long ol = xenstat_vcpu_online(domains[i]->vcpu);
	num_networks = xenstat_domain_num_networks(domains[i]);
	
	int j;
	for (j=0; j < num_networks; j++)
	{
	    network = xenstat_domain_network(domains[i],j);
	    unsigned long long net_num = xenstat_network_rbytes(network);
	    printf("network %d: %u\n", j, net_num);
	}
	printf("\n");
    }
    return 0;
}
