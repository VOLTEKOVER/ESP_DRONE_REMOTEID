"""RID Hub — Ground Station + Analyzer + Tools.

Usage:
  python -m rid_hub serve     — Start WebSocket server + capture (for Electron)
  python -m rid_hub gui       — Launch desktop GUI (pywebview, legacy)
  python -m rid_hub [tool]    — Run a CLI tool
"""

import sys

def serve():
    from .rid_cli import cmd_serve
    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument("--port", type=int, default=8765)
    parser.add_argument("--bind", default="127.0.0.1")
    parser.add_argument("--iface", default=None)
    parser.add_argument("--channel", type=int, default=6)
    args = parser.parse_args()
    cmd_serve(args)

TOOLS = {
    "serve": serve,
    "gui": lambda: __import__("sys").exit("Use 'serve' or launch via Electron"),
    "scanner": "scanner_wifi_ble",
    "mapper": "mesh_mapper",
    "bridge": "meshtastic_bridge",
    "timing": "timing_analysis",
    "ble-check": "ble_validation",
    "serial": "serial_bridge",
    "provision": "nvs_provisioning",
    "verify": "public_key_verify",
}

if len(sys.argv) > 1:
    tool = sys.argv[1]
    if tool in TOOLS:
        handler = TOOLS[tool]
        if callable(handler):
            handler()
        else:
            mod = __import__(f"rid_hub.tools.{handler}", fromlist=["main"])
            mod.main()
    elif tool == "gui":
        from .gui import main; main()
    else:
        print(f"Unknown tool: {tool}\nAvailable: {', '.join(TOOLS)}")
        sys.exit(1)
else:
    print("RID Hub — Ground Station for Open DroneID")
    print("Usage: python -m rid_hub <serve|gui|scanner|mapper|bridge|...>")
    print("Try:   python -m rid_hub serve")
