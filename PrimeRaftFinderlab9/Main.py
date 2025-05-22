import multiprocessing
import time
from node import RaftNode

NODES = 8
PORT_BASE = 5000
ALGORITHMS = ["trial", "fermat", "miller"]

def start_node(node_id, all_ports, algorithm):
    node = RaftNode(node_id, all_ports, algorithm)
    node.run()

if __name__ == "__main__":
    processes = []
    all_ports = [PORT_BASE + i for i in range(NODES)]
    
    try:
        # Start all nodes
        for i in range(NODES):
            algo = ALGORITHMS[i % len(ALGORITHMS)]
            p = multiprocessing.Process(target=start_node, args=(i, all_ports, algo))
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