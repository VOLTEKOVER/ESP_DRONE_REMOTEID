"""WiFi Remote ID beacon capture via Scapy with PCAP logging, channel hopping, and multi-channel scan."""

from __future__ import annotations
import os
import time
import struct
import threading
import logging
from typing import Optional, Callable
from pathlib import Path

from rid_hub.decoder import extract_odid_from_beacon, format_summary

logger = logging.getLogger(__name__)

try:
    from scapy.all import sniff, Dot11, Dot11Beacon, Dot11Elt, conf
    SCAPY_AVAILABLE = True
except ImportError:
    SCAPY_AVAILABLE = False


MONITOR_MODE_HELP = {
    "linux": (
        "Set interface to monitor mode:\n"
        "  sudo ip link set <iface> down\n"
        "  sudo iw dev <iface> set type monitor\n"
        "  sudo ip link set <iface> up"
    ),
    "darwin": (
        "Set interface to monitor mode (macOS):\n"
        "  sudo ifconfig <iface> down\n"
        "  sudo ifconfig <iface> up"
    ),
    "win32": (
        "On Windows monitor mode requires:\n"
        "  - Npcap installed with '802.11 monitor mode' option\n"
        "  - A compatible WiFi adapter\n"
        "  OR use a regular interface (beacon frames still work)\n"
        "  See: https://npcap.com/guide/npcap-tutorial.html"
    ),
}


CHANNELS_2GHZ = list(range(1, 14))


class RIDCapture:
    def __init__(self, iface: Optional[str] = None, channel: int = 6):
        self.iface = iface
        self.channel = channel
        self._running = False
        self._thread: Optional[threading.Thread] = None
        self._callback: Optional[Callable] = None
        self._sniffer = None
        self._pcap_path: Optional[str] = None
        self._pcap_packets: list = []
        self._pcap_lock = threading.Lock()
        self._channel_hop = False
        self._channels = [channel]
        self._channel_index = 0
        self._hop_interval = 2.0
        self._last_hop = 0.0

    @property
    def is_supported(self) -> bool:
        return SCAPY_AVAILABLE

    def set_callback(self, cb: Callable):
        self._callback = cb

    def set_pcap_output(self, path: str):
        self._pcap_path = path

    def set_channels(self, channels: list[int]):
        self._channels = channels if channels else [6]
        self._channel_index = 0

    def set_channel_hopping(self, enabled: bool, interval: float = 2.0):
        self._channel_hop = enabled
        self._hop_interval = interval
        self._last_hop = time.time()

    def start(self):
        if not SCAPY_AVAILABLE:
            logger.error("Scapy not installed. Run: pip install scapy")
            return
        if self._running:
            return
        self._running = True
        self._thread = threading.Thread(target=self._capture_loop, daemon=True)
        self._thread.start()
        logger.info(f"Capture started on iface={self.iface} ch={self.channel}")

    def stop(self):
        self._running = False
        if self._thread:
            self._thread.join(timeout=3)
        self._flush_pcap()
        logger.info("Capture stopped")

    def _flush_pcap(self):
        if not self._pcap_path:
            return
        if not self._pcap_packets:
            return
        try:
            from scapy.utils import wrpcap
            with self._pcap_lock:
                wrpcap(self._pcap_path, self._pcap_packets, append=os.path.exists(self._pcap_path))
                count = len(self._pcap_packets)
                self._pcap_packets.clear()
            logger.info(f"Flushed {count} packets to {self._pcap_path}")
        except Exception as e:
            logger.error(f"PCAP write error: {e}")

    def _process_packet(self, pkt):
        if not self._running:
            return
        try:
            if not pkt.haslayer(Dot11) or not pkt.haslayer(Dot11Beacon):
                return

            rssi = None
            if hasattr(pkt, "dBm_AntSignal"):
                rssi = pkt.dBm_AntSignal
            elif pkt.notdecoded and len(pkt.notdecoded) >= 4:
                try:
                    rssi = -256 + pkt.notdecoded[-4]
                except (ValueError, TypeError):
                    pass

            src_mac = pkt.addr2 if pkt.addr2 else "?"
            beacon = pkt[Dot11Beacon]
            frame_body = bytes(beacon)

            odid_msgs = extract_odid_from_beacon(frame_body)
            if not odid_msgs:
                return

            ts = time.time()
            summary = format_summary(odid_msgs)

            # Save raw packet for PCAP
            if self._pcap_path:
                with self._pcap_lock:
                    self._pcap_packets.append(pkt)
                    if len(self._pcap_packets) >= 100:
                        self._flush_pcap()

            data = {
                "timestamp": ts,
                "source_mac": src_mac,
                "rssi": rssi,
                "channel": self.channel,
                "summary": summary,
                "messages": odid_msgs,
            }

            if self._callback:
                try:
                    self._callback(data)
                except Exception as e:
                    logger.error(f"Callback error: {e}")

            if self._channel_hop and len(self._channels) > 1:
                now = time.time()
                if now - self._last_hop >= self._hop_interval:
                    self._last_hop = now
                    self._channel_index = (self._channel_index + 1) % len(self._channels)
                    new_ch = self._channels[self._channel_index]
                    self.channel = new_ch

        except Exception:
            pass

    def _capture_loop(self):
        kwargs = {"prn": self._process_packet, "store": 0}
        if self.iface:
            kwargs["iface"] = self.iface
        beacon_filter = "type mgt subtype beacon"
        kwargs["filter"] = beacon_filter

        try:
            self._sniffer = sniff(**kwargs)
        except PermissionError as e:
            logger.error(f"Permission denied: {e}. Try running as admin/root.")
        except Exception as e:
            logger.error(f"Capture error: {e}")


def list_interfaces() -> list[str]:
    if not SCAPY_AVAILABLE:
        return []
    try:
        from scapy.all import get_if_list
        return get_if_list()
    except Exception:
        return []


def check_monitor_mode(iface: str) -> bool:
    import platform
    if platform.system().lower() == "linux":
        try:
            import subprocess
            r = subprocess.run(
                ["iw", "dev", iface, "info"],
                capture_output=True, text=True, timeout=3
            )
            return "type monitor" in r.stdout
        except Exception:
            return False
    elif platform.system().lower() == "win32":
        return False
    return False


def print_interface_help(iface: str):
    import platform
    os_name = platform.system().lower()
    help_text = MONITOR_MODE_HELP.get(os_name, "See your OS documentation for monitor mode setup.")
    print(f"\nMonitor mode for '{iface}':")
    print(help_text)
    print()
