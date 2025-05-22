import threading
import time
from pathlib import Path
from datetime import datetime

class RaftLogic:
    def __init__(self, node_id, all_ports):
        self.node_id = node_id
        self.all_ports = all_ports
        self.state = 'Leader' if node_id == 0 else 'Follower'  # Node 0 is always initial leader
        self.leader_id = 0 if node_id != 0 else None
        self.lock = threading.Lock()
        self.shutdown_flag = False
        self.log = []
        self.log_file = "primes.txt"
        
        # Initialize log file if leader
        if self.state == 'Leader':
            Path(self.log_file).write_text("")

    def propose_value(self, value, duration, proposer_id):
        with self.lock:
            if self.state != 'Leader':
                print(f"[Node {self.node_id}] Forwarding prime {value} to leader {self.leader_id}")
                return False

            # Leader processing
            entry = {
                'value': value,
                'proposer': proposer_id,
                'duration': duration,
                'timestamp': datetime.now().isoformat()
            }
            self.log.append(entry)
            self._write_to_log(entry, proposer_id, self.node_id)  # Fixed this line
            print(f"[Leader {self.node_id}] Committed prime {value} (proposed by Node {proposer_id})")
            return True

    def _write_to_log(self, entry, proposer_id, leader_id):
        """Atomic write to output file"""
        with open(self.log_file, "a") as f:
            f.write(
                f"Prime: {entry['value']} | "
                f"Proposed by: Node {proposer_id} | "
                f"Committed by: Leader {leader_id} | "
                f"Time: {entry['duration']:.8f}s | "
                f"Timestamp: {entry['timestamp']}\n"
            )

    def shutdown(self):
        """Clean shutdown"""
        with self.lock:
            self.shutdown_flag = True