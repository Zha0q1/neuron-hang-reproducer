#include <mpi.h>
#include <stdio.h>
#include <dlfcn.h>
#include "nccl-headers/neuron/net.h"

void logger(ncclDebugLogLevel level, unsigned long flags, const char *filefunc,
	int line, const char *fmt, ...) {
	return;
}

int main() {
	printf("hang reproducer\n");
	MPI_Init(0, 0);

	int num_ranks, my_rank;
	MPI_Comm_size(MPI_COMM_WORLD, &num_ranks);
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
	printf("I am rank %d\n", my_rank);

	void *netPluginLib = NULL;
	ncclNet_v4_t *ext_net = NULL;
	netPluginLib = dlopen("libnccom-net.so", RTLD_NOW | RTLD_LOCAL);
	ext_net = (ncclNet_v4_t*) dlsym(netPluginLib, "ncclNetPlugin_v4");
	printf("ofi-nccl loaded %p\n", ext_net);

	ext_net->init(&logger);
	int ndev=111;
	ext_net->devices(&ndev);
	printf("found %d devices\n", ndev);

	char *my_handles = calloc(num_ranks, NCCL_NET_HANDLE_MAXSIZE);
	char *peer_handles = calloc(num_ranks, NCCL_NET_HANDLE_MAXSIZE);
	char **l_comm = calloc(num_ranks, sizeof(char *));
	char **s_comm = calloc(num_ranks, sizeof(char *));
	char **r_comm = calloc(num_ranks, sizeof(char *));

	int nic_dev = 0;

	for (int rank = 0; rank < num_ranks; rank++) {
		int handle_position = rank * NCCL_NET_HANDLE_MAXSIZE;
		ext_net->listen(nic_dev,
			&my_handles[handle_position],
			(void **)&l_comm[rank]);
	}
	MPI_Alltoall(my_handles, NCCL_NET_HANDLE_MAXSIZE, MPI_BYTE,
		     peer_handles, NCCL_NET_HANDLE_MAXSIZE, MPI_BYTE,
		     MPI_COMM_WORLD);
	
	if (my_rank == 0) {
		int rank = 1;
		int handle_position = rank * NCCL_NET_HANDLE_MAXSIZE;
		printf("before connect\n");
		ext_net->connect(nic_dev,
			&peer_handles[handle_position],
			(void **)&s_comm[rank]);
		printf("after connect\n");
	} else if (my_rank == 1) {
		int rank = 0;
		int handle_position = rank * NCCL_NET_HANDLE_MAXSIZE;
		printf("before accept\n");
		ext_net->accept(l_comm[rank],
			(void **)&r_comm[rank]);
		printf("after accept\n");
	}

	printf("Done on rank %d\n", my_rank);

	MPI_Finalize();
}
