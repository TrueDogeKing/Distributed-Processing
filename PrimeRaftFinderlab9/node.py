import time
from raft import RaftLogic
from prime_algos import (
    find_prime_trial_division,
    find_prime_fermat,
    find_prime_miller_rabin,
    find_prime_eratosthenes
)

class RaftNode:
    def __init__(self, node_id, all_ports, algorithm="trial", proposal_queue=None, vote_queue=None, all_vote_queues=None):
        self.node_id = node_id
        self.all_ports = all_ports
        self.algorithm = algorithm
        self.max_prime = 2
        self.proposal_queue = proposal_queue  # followers -> leader
        self.vote_queue = vote_queue          # leader -> follower
        self.all_vote_queues = all_vote_queues  # leader's dict of follower queues
        self.raft = RaftLogic(node_id, all_ports, proposal_queue)

        self.shutdown_flag = False

    def choose_algorithm(self, start):
        if self.algorithm == "trial":
            return find_prime_trial_division(start)
        elif self.algorithm == "fermat":
            return find_prime_fermat(start)
        elif self.algorithm == "miller":
            return find_prime_miller_rabin(start)
        elif self.algorithm == "eratosthenes":
            prime = find_prime_eratosthenes(start, limit=20000)
            if prime is None:
                raise ValueError("Limit for Eratosthenes sieve exceeded")
            return prime
        else:
            raise ValueError("Unknown algorithm")


    def run(self):
        if self.raft.state == 'Follower':
            # Register with leader by sending message on proposal_queue
            self.proposal_queue.put(('register', self.node_id))

            current = 3
            while current < 100000 and not self.shutdown_flag:
                # Check for voting requests from leader
                while self.vote_queue and not self.vote_queue.empty():
                    msg = self.vote_queue.get()
                    if msg[0] == 'vote_request':
                        prime_candidate = msg[1]
                        vote = self.raft.receive_candidate(prime_candidate)
                        # Send vote back to leader
                        self.proposal_queue.put(('vote_response', self.node_id, vote))

                # Compute prime candidates
                start_time = time.time()
                prime = self.choose_algorithm(current)
                duration = time.time() - start_time # Step 3 & 4: Calculate duration
                if prime > self.max_prime:
                    self.max_prime = prime
                    print(f"[Node {self.node_id}] Forwarding prime {prime} to leader")
                    self.proposal_queue.put(('proposal', prime, duration, self.node_id))  # Duration can be measured if needed

                current = prime + 1
                time.sleep(0.1)  # slow down loop for demo

            print(f"[Node {self.node_id}] Completed prime search")

        elif self.raft.state == 'Leader':
            # Register followers with their queues
            for follower_id, queue in self.all_vote_queues.items():
                self.raft.register_follower(follower_id, queue)

            print(f"[Leader {self.node_id}] Listening for messages from followers...")

            while not self.shutdown_flag:
                try:
                    msg = self.proposal_queue.get(timeout=3)
                    if msg[0] == 'proposal':
                        _, prime, duration, proposer_id = msg
                        self.raft.propose_value(prime, duration, proposer_id)
                    elif msg[0] == 'register':
                        _, follower_id = msg
                        # Already registered, but can confirm here if needed
                        print(f"[Leader {self.node_id}] Follower {follower_id} registered")
                except Exception:
                    print(f"[Leader {self.node_id}] No new messages, shutting down.")
                    break

        self.raft.shutdown()
