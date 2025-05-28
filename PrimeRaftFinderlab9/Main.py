from multiprocessing import Process, Queue
import time
from node import RaftNode

NODES = 8
PORT_BASE = 5000
ALGORITHMS = ["trial", "fermat", "miller"]

def start_node(node_id, all_ports, algorithm, proposal_queue):
    node = RaftNode(node_id, all_ports, algorithm, proposal_queue)
    node.run()


if __name__ == "__main__":
    processes = []
    all_ports = [PORT_BASE + i for i in range(NODES)]
    proposal_queue = Queue()
    
    # Create all nodes first
    nodes = []
    for i in range(NODES):
        algo = ALGORITHMS[i % len(ALGORITHMS)]
        node = RaftNode(i, all_ports, algo, proposal_queue)
        nodes.append(node)
    
    # Set leader reference in followers
    leader_node = nodes[0]  # Assuming node 0 is leader
    for node in nodes[1:]:
        node.leader_node = leader_node
    
    try:
        # Start all nodes
        for node in nodes:
            p = Process(target=node.run)
            processes.append(p)
            p.start()
        
        # Monitor for completion
        while any(p.is_alive() for p in processes):
            time.sleep(1)
            
    except KeyboardInterrupt:
        print("\nTerminating processes...")
        for p in processes:
            p.terminate()
    finally:
        for p in processes:
            p.join()
        print("All nodes shutdown")