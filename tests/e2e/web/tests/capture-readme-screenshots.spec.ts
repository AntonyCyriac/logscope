import * as path from 'node:path';
import { test } from '@playwright/test';
import {
  addLogArtifact,
  createInvestigation,
  openFirstArtifact,
  story6AppLogPath,
  story6SyslogPath,
  switchBottomTab,
  waitForReady,
} from './helpers';

/** Regenerates docs/assets/logscope-web.png for README. Run on every release with README update. */

const REPO_ROOT = path.resolve(__dirname, '../../../../');
const ASSETS = path.join(REPO_ROOT, 'docs', 'assets');

test('capture README web screenshot (Story 6)', async ({ page }) => {
  await waitForReady(page);
  await createInvestigation(page, 'readme-screenshot');
  await addLogArtifact(page, story6AppLogPath);
  await addLogArtifact(page, story6SyslogPath);
  await openFirstArtifact(page);

  await switchBottomTab(page, 'timeline');
  const timelineRows = page.getByTestId('timeline-row');
  await timelineRows.first().waitFor({ state: 'visible', timeout: 15_000 });
  await timelineRows.first().click();
  await page.getByTestId('suggested-connections-panel').waitFor({ state: 'visible' });
  await page.getByTestId('suggested-connection-row').first().waitFor({ state: 'visible' });

  await page.screenshot({
    path: path.join(ASSETS, 'logscope-web.png'),
    fullPage: false,
  });
});
