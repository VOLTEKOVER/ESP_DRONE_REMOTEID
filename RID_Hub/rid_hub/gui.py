"""pywebview native window — standalone desktop app (Electron recommended)."""

from __future__ import annotations
import sys
import time
import threading
import logging
from pathlib import Path

logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s [%(levelname)s] %(name)s: %(message)s",
)
logger = logging.getLogger("rid_gui")

WEB_DIR = Path(__file__).parent / "web"


def run_backend(ws_port: int, capture_iface: str, capture_channel: int):
    from rid_hub.server import BroadcastServer
    from rid_hub.capture import RIDCapture

    server = BroadcastServer(port=ws_port)
    server.start()
    time.sleep(0.3)

    capturer = RIDCapture(iface=capture_iface or None, channel=capture_channel)
    if not capturer.is_supported:
        logger.warning("Scapy not available. Install: pip install scapy")

    capturer.set_callback(server.on_capture)
    capturer.start()
    time.sleep(0.5)

    return server, capturer


def main():
    import argparse

    parser = argparse.ArgumentParser(description="RID Hub — Ground Station")
    parser.add_argument("--port", type=int, default=8765, help="WebSocket port (default: 8765)")
    parser.add_argument("--iface", default=None, help="WiFi interface for capture")
    parser.add_argument("--channel", type=int, default=6, help="WiFi channel (default: 6)")
    parser.add_argument("--no-gui", action="store_true", help="Run without GUI (headless)")
    parser.add_argument("--list-ifaces", action="store_true", help="List available WiFi interfaces and exit")
    args = parser.parse_args()

    if args.list_ifaces:
        from rid_hub.capture import list_interfaces
        ifaces = list_interfaces()
        print("Available interfaces:")
        for i in ifaces:
            print(f"  {i}")
        sys.exit(0)

    server, capturer = run_backend(args.port, args.iface, args.channel)

    if args.no_gui:
        logger.info("Headless mode. WebSocket on ws://127.0.0.1:%d", args.port)
        try:
            while True:
                time.sleep(1)
        except KeyboardInterrupt:
            pass
    else:
        try:
            import webview
        except ImportError:
            print("pywebview not installed. Run: pip install pywebview")
            capturer.stop()
            server.stop()
            sys.exit(1)

        index_path = WEB_DIR / "index.html"
        window = webview.create_window(
            title="RID Hub",
            url=str(index_path),
            width=1280,
            height=860,
            resizable=True,
            text_select=True,
        )
        webview.start(private_mode=True)

    capturer.stop()
    server.stop()


if __name__ == "__main__":
    main()
