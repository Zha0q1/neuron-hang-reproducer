# aws-ofi-plugin
latest master

# compile
mpicc -I/home/ubuntu/aws-ofi-nccl/include/ main.c -ldl

# run on 1 node -- hangs
mpirun -N 2 -x FI_EFA_USE_DEVICE_RDMA=1 -x LD_LIBRARY_PATH=/usr/local/lib:/usr/lib:/opt/aws/neuron/lib ./a.out

# run on 2 nodes -- works
mpirun --hostfile ~/hosts -N 1 -x FI_EFA_USE_DEVICE_RDMA=1 -x LD_LIBRARY_PATH=/usr/local/lib:/usr/lib:/opt/aws/neuron/lib /home/ubuntu/reproducer/a.out
