const { contextBridge } = require("electron");

// Expose a safe WS_URL to the renderer
contextBridge.exposeInMainWorld("WS_CONFIG", "ws://127.0.0.1:8765");
