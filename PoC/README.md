# HyperCache Proof of Concept

This is a proof of concept for an application called HyperCache.


HyperCache is a horizontally scalable key-value database that runs in containerized environments like Kubernetes.


Unlike databases like Redis, it should not require a lot of configuration on the side of the cloud engineer. 
I want to accomplish this through an architecture that uses a proxy cluster and a database shard cluster. Proxies shall then route traffic based on a "route-map": Every request (GET/SET) comes with a key, which is then hashed into a small hashmap. Based on the location of the key in the hashmap, the proxy will determine the shard that the request is routed to. With this behavior, all shards can have a full 20K Slots, this is different from databases like Redis that use 16K Slots and split them over the running shards.

The actual game changer with this architecture is the fact that proxies (resp. the entries in the database) are "Stateless". The stateless nature of the proxies solves a certain issue: Instances must know each other by their addresses. For example, when using Redis, it is required that Redis instances know the other cluster instances by IP address, this often leads to problems or workarounds that are not very clean. With the proxy infrastructure, we can just put a network-loadbalancer in front of the proxies and give all instances of the database the address of the loadbalancer. Now, even if the address (IP/DNS) of a container changes, the instance will always be able to connect to the proxy cluster via NLB, which can then give the instance the necessary information to find the rest of the cluster.

Besides that, this architecture should allow it to perform consistent operations while still performing at extreme speed.

### Idea Sketch

The Figjam designfile below shows a bit what my idea is.

![Whiteboard](/PoC/whiteboard.png)
