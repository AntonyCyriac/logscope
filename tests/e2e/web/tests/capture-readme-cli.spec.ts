import { execFileSync } from 'node:child_process';
import * as fs from 'node:fs';
import * as path from 'node:path';
import { test } from '@playwright/test';

const REPO_ROOT = path.resolve(__dirname, '../../../../');
const ASSETS = path.join(REPO_ROOT, 'docs', 'assets');
const CLI = path.join(REPO_ROOT, 'build', 'apps', 'cli', 'logscope');

function cliLines(args: string[]): string {
  try {
    return execFileSync(CLI, args, { encoding: 'utf8', cwd: REPO_ROOT, maxBuffer: 2 * 1024 * 1024 });
  } catch (error) {
    const err = error as { stdout?: string; stderr?: string };
    return (err.stdout || '') + (err.stderr || '');
  }
}

test('capture README CLI screenshot', async ({ page }) => {
  const sampleLog = path.join(REPO_ROOT, 'samples', 'sample.log');
  const lines = [
    'PS> ./build/apps/cli/logscope analyze samples/sample.log',
    cliLines(['analyze', sampleLog]).trim(),
    '',
    'PS> ./build/apps/cli/logscope investigate samples/sample.log --level error',
    cliLines(['investigate', sampleLog, '--level', 'error']).trim().split('\n').slice(0, 12).join('\n'),
    '',
    'PS> ./build/apps/cli/logscope investigation timeline <id> --format table',
    'PS> ./build/apps/cli/logscope investigation links list <id>',
    '(investigation subcommands — v2.11.0)',
  ].join('\n');

  const html = `<!DOCTYPE html><html><head><meta charset="utf-8"><style>
    body{margin:0;background:#0d1117;padding:20px 24px}
    pre{font-family:Consolas,"Cascadia Mono",monospace;font-size:13px;line-height:1.42;color:#c9d1d9;white-space:pre-wrap;margin:0}
  </style></head><body><pre>${lines.replace(/&/g, '&amp;').replace(/</g, '&lt;')}</pre></body></html>`;

  await page.setContent(html, { waitUntil: 'load' });
  await page.screenshot({ path: path.join(ASSETS, 'logscope-cli.png'), fullPage: false });
});
