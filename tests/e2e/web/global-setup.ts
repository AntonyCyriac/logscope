import { spawn, type ChildProcess } from 'node:child_process';
import * as fs from 'node:fs';
import * as path from 'node:path';
import { setTimeout as delay } from 'node:timers/promises';

const REPO_ROOT = path.resolve(__dirname, '../../..');
const STATE_FILE = path.join(__dirname, '.playwright-server.json');
const BASE_URL = process.env.LOGSCOPE_WEB_BASE_URL ?? 'http://127.0.0.1:8080';

function resolveWebBinary(): string {
  if (process.env.LOGSCOPE_WEB_BIN) {
    return process.env.LOGSCOPE_WEB_BIN;
  }

  const candidates = [
    path.join(REPO_ROOT, 'build', 'apps', 'web', 'logscope-web'),
    path.join(REPO_ROOT, 'build', 'apps', 'web', 'logscope-web.exe'),
    path.join(REPO_ROOT, 'build', 'apps', 'web', 'Release', 'logscope-web.exe'),
    path.join(REPO_ROOT, 'build', 'apps', 'web', 'Debug', 'logscope-web.exe'),
  ];

  for (const candidate of candidates) {
    if (fs.existsSync(candidate)) {
      return candidate;
    }
  }

  throw new Error(
    'logscope-web binary not found. Build with cmake --build build --target logscope-web '
      + 'or set LOGSCOPE_WEB_BIN.',
  );
}

async function waitForServer(url: string, timeoutMs = 30_000): Promise<void> {
  const deadline = Date.now() + timeoutMs;

  while (Date.now() < deadline) {
    try {
      const response = await fetch(url);
      if (response.ok || response.status === 404) {
        return;
      }
    } catch {
      // Server not ready yet.
    }

    await delay(250);
  }

  throw new Error(`Timed out waiting for ${url}`);
}

export default async function globalSetup(): Promise<void> {
  if (process.env.LOGSCOPE_WEB_E2E_EXTERNAL === '1') {
    await waitForServer(BASE_URL);
    return;
  }

  const binary = resolveWebBinary();
  const configPath = path.join(REPO_ROOT, 'samples', 'demo-story-gate.properties');
  const uiDir = path.join(REPO_ROOT, 'apps', 'web', 'ui', 'dist');

  const child: ChildProcess = spawn(binary, ['--config', configPath], {
    cwd: REPO_ROOT,
    env: {
      ...process.env,
      LOGSCOPE_WEB_UI_DIR: uiDir,
    },
    stdio: ['ignore', 'pipe', 'pipe'],
  });

  if (!child.pid) {
    throw new Error('Failed to start logscope-web');
  }

  fs.writeFileSync(
    STATE_FILE,
    JSON.stringify({ pid: child.pid, managed: true }),
    'utf8',
  );

  child.stdout?.on('data', (chunk) => {
    process.stdout.write(`[logscope-web] ${chunk}`);
  });
  child.stderr?.on('data', (chunk) => {
    process.stderr.write(`[logscope-web] ${chunk}`);
  });

  await waitForServer(BASE_URL);
}
