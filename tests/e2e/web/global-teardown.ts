import * as fs from 'node:fs';
import * as path from 'node:path';

const STATE_FILE = path.join(__dirname, '.playwright-server.json');

export default async function globalTeardown(): Promise<void> {
  if (!fs.existsSync(STATE_FILE)) {
    return;
  }

  const state = JSON.parse(fs.readFileSync(STATE_FILE, 'utf8')) as {
    pid?: number;
    managed?: boolean;
  };

  fs.unlinkSync(STATE_FILE);

  if (!state.managed || !state.pid) {
    return;
  }

  try {
    process.kill(state.pid);
  } catch {
    // Process may already be gone.
  }
}
