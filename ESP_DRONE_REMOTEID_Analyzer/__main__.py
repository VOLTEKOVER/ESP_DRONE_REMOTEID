"""Run as: python -m ESP_DRONE_REMOTEID_Analyzer [tool]

Tools:
  (no arg)     — Launch desktop GUI (pywebview)
  scanner      — WiFi+BLE RID receiver scanner
  mapper       — Mesh network map visualizer
  bridge       — Meshtastic LoRa bridge
  timing       — Packet interval analysis
  ble-check    — BLE advertisement validation (macOS)
  serial       — macOS serial bridge (socat wrapper)
  provision    — NVS private key provisioning
  verify       — ECDSA signature verification
"""

import sys
from .gui import main

if len(sys.argv) > 1:
    tool = sys.argv[1]
    if tool == "scanner":
        from .tools.scanner_wifi_ble import main as m; m()
    elif tool == "mapper":
        from .tools.mesh_mapper import main as m; m()
    elif tool == "bridge":
        from .tools.meshtastic_bridge import main as m; m()
    elif tool == "timing":
        from .tools.timing_analysis import main as m; m()
    elif tool == "ble-check":
        from .tools.ble_validation import main as m; m()
    elif tool == "serial":
        from .tools.serial_bridge import main as m; m()
    elif tool == "provision":
        from .tools.nvs_provisioning import main as m; m()
    elif tool == "verify":
        from .tools.public_key_verify import main as m; m()
    else:
        print(f"Unknown tool: {tool}")
        sys.exit(1)
else:
    main()
