import threading
from pathlib import Path
from datetime import datetime

class RaftLogic:
    def __init__(self, node_id, all_ports, proposal_queue=None):
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
        self.followers = {}  # follower_id -> Queue for voting requests
        self.proposal_queue = proposal_queue  # For leader to receive messages (vote responses, proposals)

        if self.state == 'Leader':
            Path(self.log_file).write_text("")  # clear log file at start

    def register_follower(self, follower_id, response_queue):
        with self.lock:
            self.followers[follower_id] = response_queue
            print(f"[Leader {self.node_id}] Registered follower {follower_id}")
            print(f"[Leader {self.node_id}] Current followers: {list(self.followers.keys())}")

    def propose_value(self, value, duration, proposer_id):
        with self.lock:
            if self.state != 'Leader':
                print(f"[Node {self.node_id}] Not leader, ignoring propose_value")
                return False

            print(f"[Leader {self.node_id}] Broadcasting prime {value} for voting")
            votes = 1  # leader votes yes
            total = 1

            # Send vote requests to all followers
            for follower_id, follower_queue in self.followers.items():
                total += 1
                follower_queue.put(('vote_request', value))
                print(f"[Leader {self.node_id}] Sent vote_request to follower {follower_id}")

            votes_received = 0
            votes_yes = 1

            # Wait for votes from followers
            while votes_received < (total - 1):
                try:
                    msg = self.proposal_queue.get(timeout=5)
                except:
                    print(f"[Leader {self.node_id}] Timeout waiting for votes")
                    break

                if msg[0] == 'vote_response':
                    _, follower_id, vote = msg
                    if follower_id in self.followers:
                        votes_received += 1
                        if vote:
                            votes_yes += 1
                        print(f"[Leader {self.node_id}] Received vote from follower {follower_id}: {'YES' if vote else 'NO'}")

            if votes_yes > total // 2:
                self._commit_prime(value, duration, proposer_id)
                return True
            else:
                print(f"[Leader {self.node_id}] Prime {value} rejected (votes: {votes_yes}/{total})")
                return False

    def receive_candidate(self, value):
        with self.lock:
            if value > self.highest_prime:
                print(f"[Follower {self.node_id}] Accepting new candidate prime {value}")
                return True
            print(f"[Follower {self.node_id}] Rejecting candidate prime {value} (current max: {self.highest_prime})")
            return False

    def _commit_prime(self, value, duration, proposer_id):
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

        print(f"[Leader {self.node_id}] ✅ Committed prime {value} (proposed by Node {proposer_id}) after majority agreement")

    def _write_to_log(self, entry, proposer_id, leader_id):
        with open(self.log_file, "a") as f:
            f.write(
                f"Prime: {entry['value']} | Proposed by: Node {proposer_id} | Committed by: Leader {leader_id} | "
                f"Time: {entry['duration']:.8f}s | Timestamp: {entry['timestamp']}\n"
            )

    def shutdown(self):
        with self.lock:
            self.shutdown_flag = True
