import random
import time
from raft import RaftLogic  # For package-relative imports
from prime_algos import (
    find_prime_trial_division,
    find_prime_fermat,
    find_prime_miller_rabin
)

class RaftNode:
    def __init__(self, node_id, all_ports, algorithm="trial"):
        self.node_id = node_id
        self.all_ports = all_ports
        self.algorithm = algorithm
        self.max_prime = 2
        self.raft = RaftLogic(node_id, all_ports) 
        self.shutdown_flag = False

    def choose_algorithm(self, start):
        if self.algorithm == "trial":
            return find_prime_trial_division(start)
        elif self.algorithm == "fermat":
            return find_prime_fermat(start)
        elif self.algorithm == "miller":
            return find_prime_miller_rabin(start)
        else:
            raise ValueError("Unknown algorithm")

    def run(self):
        current = 3
        while current < 10000 and not self.shutdown_flag:
            start_time = time.time()
            prime = self.choose_algorithm(current)
            duration = time.time() - start_time
            
            if prime > self.max_prime:
                self.max_prime = prime
                self.raft.propose_value(prime, duration, self.node_id)
            
            current = prime + 1
        
        # Shutdown when done
        self.raft.shutdown()
        print(f"Node {self.node_id} completed prime search")