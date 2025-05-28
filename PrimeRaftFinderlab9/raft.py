import threading
import time
from pathlib import Path
from datetime import datetime

class RaftLogic:
    def __init__(self, node_id, all_ports):
        self.node_id = node_id
        self.all_ports = all_ports
        self.state = 'Leader' if node_id == 0 else 'Follower'
        self.leader_id = 0 if node_id != 0 else None
        self.lock = threading.Lock()
        self.shutdown_flag = False
        self.log = []
        self.known_primes = set()
        self.highest_prime = 0
        self.log_file = "primes.txt"
        self.followers = {}  # Mock registry of other nodes for simulation
        self.pending_votes = {}  # Track ongoing vote processes

        if self.state == 'Leader':
            Path(self.log_file).write_text("")

    def register_follower(self, follower_node):
        if self.state == 'Leader':
            self.followers[follower_node.node_id] = follower_node

    def propose_value(self, value, duration, proposer_id):
        with self.lock:
            if self.state != 'Leader':
                print(f"[Node {self.node_id}] Forwarding prime {value} to leader {self.leader_id}")
                return False

            
            # Leader broadcasts the proposed value to followers for voting
            print(f"[Leader {self.node_id}] Broadcasting prime {value} for voting")
            print(f"[Leader {self.node_id}] Registered followers: {list(self.followers.keys())}")

            votes = 1  # Leader votes yes
            total = 1  # Including self
            for follower in self.followers.values():
                total += 1
                print(f"[Leader {self.node_id}] Asking follower {follower.node_id} to vote on prime {value}")
                vote = follower.receive_candidate(value)
                print(f"[Leader {self.node_id}] Follower {follower.node_id} voted {'YES' if vote else 'NO'} (votes so far: {votes}/{total})")
                if vote:
                    votes += 1


            if votes > total // 2:
                self._commit_prime(value, duration, proposer_id)
                return True
            else:
                print(f"[Leader {self.node_id}] Prime {value} rejected (votes: {votes}/{total})")
                return False

    def receive_candidate(self, value):
        """Follower checks if it considers the value a new biggest prime"""
        with self.lock:
            if value > self.highest_prime:
                print(f"[Follower {self.node_id}] Accepting new candidate prime {value}")
                return True
            print(f"[Follower {self.node_id}] Rejecting candidate prime {value} (current max: {self.highest_prime})")
            return False

    def _commit_prime(self, value, duration, proposer_id):
        """Commits the prime to the log after consensus"""
        entry = {
            'value': value,
            'proposer': proposer_id,
            'duration': duration,
            'timestamp': datetime.now().isoformat()
        }
        self.log.append(entry)
        self.highest_prime = max(self.highest_prime, value)
        self.known_primes.add(value)
        self._write_to_log(entry, proposer_id, self.node_id)

        print(f"[Leader {self.node_id}] ✅ Committed prime {value} "
            f"(proposed by Node {proposer_id}) after majority agreement")

    def _write_to_log(self, entry, proposer_id, leader_id):
        """Atomic write to output file, with proposer info clearly noted"""
        with open(self.log_file, "a") as f:
            f.write(
                f"# Proposed by Node {proposer_id}\n"
                f"Prime: {entry['value']} | "
                f"Committed by: Leader {leader_id} | "
                f"Time: {entry['duration']:.8f}s | "
                f"Timestamp: {entry['timestamp']}\n\n"
            )


    def _write_to_log(self, entry, proposer_id, leader_id):
        with open(self.log_file, "a") as f:
            f.write(
                f"Prime: {entry['value']} | "
                f"Proposed by: Node {proposer_id} | "
                f"Committed by: Leader {leader_id} | "
                f"Time: {entry['duration']:.8f}s | "
                f"Timestamp: {entry['timestamp']}\n"
            )

    def shutdown(self):
        with self.lock:
            self.shutdown_flag = True
