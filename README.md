# Anna Cache TCC

[![License](https://img.shields.io/badge/license-Apache--2.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)

The CacheTCC from FaaSTCC was based on the [Anna Cache](https://github.com/hydro-project/anna-cache) work by [RISE Lab](https://rise.cs.berkeley.edu) at [UC Berkeley](https://berkeley.edu). 

The Anna cache is designed to be a local (i.e., on machine) cache that interacts with a cluster-based deployment of Anna. It interacts with applications (primarily [Cloudburst](https://github.com/hydro-project/cloudburst)) over IPC channels, and it is designed to periodically receive updates propagated from the underlying KVS during the periodic multicast protocol. There are currently two cache implementations:

1. A default cache that supports all of Anna's lattice types except for causal lattices.
2. A causal cache that supports single- and mult-object causal consistency, automatically resolving dependencies before making data available.

The implementation of the default cache is simple; the causal cache is more complicated, and we are working on a more detailed description of its design.

The cache is currently not designed to be a stand-alone data store, but it could be easily extended for that purpose. If this is of interest to you, please open an issue, and we would be happy to discuss this in more detail.

## License

FaaSTCC is licensed under the [Apache v2 License](LICENSE).
