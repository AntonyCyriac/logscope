import * as path from 'node:path';
import { test } from '@playwright/test';
import {
  addLogArtifact,
  addPstackArtifact,
  analyzeHeader,
  createEvidenceLink,
  createInvestigation,
  openFirstArtifact,
  sampleLogPath,
  switchBottomTab,
  syslogPath,
  waitForReady,
} from './helpers';

const REPO_ROOT = path.resolve(__dirname, '../../../../');
const ASSETS = path.join(REPO_ROOT, 'docs', 'assets');

test('capture README web screenshot (Story 5)', async ({ page }) => {
  await waitForReady(page);
  await createInvestigation(page, 'readme-screenshot');
  await addLogArtifact(page);
  await addLogArtifact(page, syslogPath);
  await addPstackArtifact(page);
  await openFirstArtifact(page);
  await analyzeHeader(page);

  await switchBottomTab(page, 'timeline');
  await page.getByTestId('timeline-row').first().click();
  await createEvidenceLink(page);
  await page.getByTestId('timeline-link-badge').first().waitFor({ state: 'visible' });

  await page.screenshot({
    path: path.join(ASSETS, 'logscope-web.png'),
    fullPage: false,
  });
});
