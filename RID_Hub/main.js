const { app, BrowserWindow, dialog } = require("electron");
const { spawn } = require("child_process");
const path = require("path");
const fs = require("fs");

let mainWindow = null;
let pythonProcess = null;
const WS_PORT = 8765;

function findPython() {
  return process.platform === "win32" ? "python" : "python3";
}

function startPythonBackend() {
  const pythonDir = path.join(__dirname);
  const pythonExe = findPython();

  pythonProcess = spawn(pythonExe, [
    "-m", "rid_hub",
    "serve",
    "--port", String(WS_PORT),
    "--bind", "127.0.0.1",
  ], {
    cwd: pythonDir,
    stdio: ["ignore", "pipe", "pipe"],
    env: { ...process.env, PYTHONUNBUFFERED: "1" },
  });

  pythonProcess.stdout.on("data", (data) => {
    console.log(`[rid-hub-backend] ${data.toString().trim()}`);
  });

  pythonProcess.stderr.on("data", (data) => {
    console.error(`[rid-hub-backend] ${data.toString().trim()}`);
  });

  pythonProcess.on("exit", (code) => {
    console.log(`[rid-hub-backend] exited with code ${code}`);
    pythonProcess = null;
  });

  pythonProcess.on("error", (err) => {
    console.error(`[rid-hub-backend] failed to start: ${err.message}`);
    dialog.showErrorBox(
      "RID Hub — Backend Error",
      `Could not start the Python backend:\n\n${err.message}\n\nMake sure Python 3 is installed and in your PATH.`
    );
  });
}

function stopPythonBackend() {
  if (pythonProcess) {
    pythonProcess.kill();
    pythonProcess = null;
  }
}

function createWindow() {
  const webPath = path.join(__dirname, "rid_hub", "web", "index.html");

  mainWindow = new BrowserWindow({
    width: 1280,
    height: 860,
    minWidth: 800,
    minHeight: 600,
    title: "RID Hub",
    webPreferences: {
      preload: path.join(__dirname, "preload.js"),
      contextIsolation: true,
      nodeIntegration: false,
    },
  });

  mainWindow.loadFile(webPath);

  mainWindow.on("closed", () => {
    mainWindow = null;
  });
}

app.whenReady().then(() => {
  startPythonBackend();

  // Give Python a moment to start the WebSocket server
  setTimeout(createWindow, 1000);

  app.on("activate", () => {
    if (BrowserWindow.getAllWindows().length === 0) {
      createWindow();
    }
  });
});

app.on("window-all-closed", () => {
  stopPythonBackend();
  if (process.platform !== "darwin") {
    app.quit();
  }
});

app.on("before-quit", () => {
  stopPythonBackend();
});
