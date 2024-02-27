# PIGEON: Private Inference of Neural Networks

This branch extends HPMPC with private inference capabilities. The framework is structured as follows.

* [FlexNN](https://github.com/chart21/flexNN/tree/hpmpc): A templated neural network inference engine that performs the forward pass of a CNN generically
* Program/functions contains MPC-generic implementations of functions such as ReLU
* Protocols contains the MPC protocols that are required by the middlelayer.

The following protocols are currently fully supported by PIGEON.

3-PC: OECL (Ours, Protocol 5), TTP (Protocol 6)

4-PC: OEC-MAL (Ours, Protocol 12), TTP (Protocol 6)

## Getting Started

You can use the provided Dockerfile or set up the project manually
The only dependencies are OpenSSL and Eigen. Install on your target system, for instance via ```apt install libssl-dev libeigen3-dev```
Clone our inference engine.
> git clone https://github.com/chart21/flexNN/tree/hpmpc SimpleNN

To export a model or dataset from PyTorch use [Pygeon](https://github.com/chart21/pygeon) and save the resulting bin files to `SimpleNN/dataset` or `SimpleNN/model_zoo`.
Existing networks are defined in `SimpleNN/architectures`. `Programs/functions/NN.hpp` includes a FUNCTION_IDENTIFIER for different model architectures and datasets (for instance 70 for RestNet18 on CIFAR-10). 
You can select a protocol and function in the file `config.h`. 

The following commands are a quick way to compile the current configuration for a 3-PC protocol and run all executables locally. This compiles all player executables using g++ with -Ofast and runs all executables on localhost on the same machine.
> ./scripts/config.sh -p all3

> ./scripts/run_loally.sh -n 3

For a 4-PC protocol, you can run.

> ./scripts/config.sh -p all4

> ./scripts/run_loally.sh -n 4

## Configuration and Compilation

Most configuration is contained in the file `config.h`. Take a careful look at its documentation and different options. For instance you can specify which party should provide the model and which party should provide the data for private inference.

The Split-Roles scripts transform a protocol into a homogeneous protocol by running multiple executables with different player assignments in parallel.

The following script compiles six executables of a 3-PC protocol for player 2 (all player combinations) to run a homogeneous 3-PC protocol on three nodes using Split-Roles.
> ./scripts/split-roles-3-compile.sh -p 2

The following script compiles 18 executables of a 3-PC protocol for player 3 to run a homogeneous 3-PC protocol on four nodes using Split-Roles.
> ./scripts/split-roles-3to4-compile.sh -p 3

The following script compiles 24 executables of a 4-PC protocol for player 0 to run a homogeneous 4-PC protocol on four nodes using Split-Roles.
> ./scripts/split-roles-4-compile.sh -p 0


### Execution

In a distributed setup, you need to specify the IP addresses for each party and run one executable on each node.

Execute P0 executable.
> ./run-P0.o IP_P1 IP_P2

Execute P1 executable.
> ./run-P1.o IP_P0 IP_P2

Execute P2 executable.
> ./run-P2.o IP_P0 IP_P1


Run Split-Roles (3) executables for Player 0.
> ./scripts/split-roles-3-execute.sh -p 0 -a IP_P0 -b IP_P1 -c IP_P2 -d IP_P3

To run all players locally on one machine, omit the IP addresses or set them to 127.0.0.1, and use -p all
> ./scripts/split-roles-3-execute.sh -p all


