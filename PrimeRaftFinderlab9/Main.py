from multiprocessing import Process, Queue
import time

from node import RaftNode

NODES = 8
PORT_BASE = 5000
ALGORITHMS = ["trial", "fermat", "miller","eratosthenes"]

def start_node(node_id, all_ports, algorithm, proposal_queue, vote_queue=None, all_vote_queues=None):
    node = RaftNode(node_id, all_ports, algorithm, proposal_queue, vote_queue, all_vote_queues)
    node.run()

if __name__ == "__main__":
    all_ports = [PORT_BASE + i for i in range(NODES)]
    proposal_queue = Queue()  # Followers -> Leader
    vote_queues = {i: Queue() for i in range(1, NODES)}  # Leader -> Followers

    processes = []
    for i in range(NODES):
        algo = ALGORITHMS[i % len(ALGORITHMS)]
        if i == 0:
            # Leader process gets all follower vote queues
            p = Process(target=start_node, args=(i, all_ports, algo, proposal_queue, None, vote_queues))
        else:
            # Followers get their own vote queue to listen for voting requests
            p = Process(target=start_node, args=(i, all_ports, algo, proposal_queue, vote_queues[i], None))
        processes.append(p)
        p.start()

    try:
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
