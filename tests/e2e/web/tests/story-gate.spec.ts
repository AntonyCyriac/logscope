import { test, expect } from '@playwright/test';

import {
  addLogArtifact,
  addPstackArtifact,
  analyzeHeader,
  createInvestigation,
  openFirstArtifact,
  switchBottomTab,
  waitForReady,
} from './helpers';

test.describe('Story Gate — investigation workflow', () => {
  test.beforeEach(async ({ page }) => {
    await waitForReady(page);
  });

  test('Story 1-2: create investigation, add log artifact, open in center viewer', async ({ page }) => {
    await createInvestigation(page);
    await addLogArtifact(page);
    await openFirstArtifact(page);

    await expect(page.getByTestId('center-artifact-title')).toContainText('sample.log');
    await expect(page.getByTestId('artifact-viewer')).toBeVisible();
    await expect(page.getByTestId('artifact-viewer')).toContainText('Application started');
  });

  test('Story 3: timeline shows events; click event highlights results', async ({ page }) => {
    await createInvestigation(page);
    await addLogArtifact(page);
    await openFirstArtifact(page);
    await analyzeHeader(page);

    await switchBottomTab(page, 'timeline');
    const timelineRows = page.getByTestId('timeline-row');
    await expect(timelineRows.first()).toBeVisible({ timeout: 15_000 });
    expect(await timelineRows.count()).toBeGreaterThan(0);

    await timelineRows.first().click();
    await expect(page.getByTestId('status')).toContainText(/Opened|line/);

    await switchBottomTab(page, 'results');
    await expect(page.getByTestId('results-row').first()).toBeVisible();
  });

  test('Story 4: crash tab shows SIGSEGV and fault thread jump', async ({ page }) => {
    await createInvestigation(page);
    await addLogArtifact(page);
    await addPstackArtifact(page);

    await switchBottomTab(page, 'crash');
    await expect(page.getByTestId('crash-signal')).toContainText('SIGSEGV', { timeout: 15_000 });
    await expect(page.getByTestId('crash-fault-thread')).toBeVisible();

    await page.getByTestId('crash-fault-thread').click();
    await expect(page.getByTestId('status')).toContainText(/Jumped to pstack thread/);
    await expect(page.locator('[data-testid="pstack-thread"].crash-pstack-thread--highlight')).toBeVisible();
  });

  test('Investigate: analyze populates results for default error search', async ({ page }) => {
    await createInvestigation(page);
    await addLogArtifact(page);
    await openFirstArtifact(page);
    await analyzeHeader(page);

    await switchBottomTab(page, 'results');
    const rows = page.getByTestId('results-row');
    await expect(rows.first()).toBeVisible();
    expect(await rows.count()).toBeGreaterThan(0);
  });

  test('AI: ask errors returns matches with noop config', async ({ page }) => {
    await createInvestigation(page);
    await addLogArtifact(page);
    await openFirstArtifact(page);
    await analyzeHeader(page);

    await switchBottomTab(page, 'ai');
    await page.getByTestId('ask-input').fill('errors');
    await page.getByTestId('ask-btn').click();

    await expect(page.getByTestId('status')).toContainText('AI ask complete', { timeout: 20_000 });
    await switchBottomTab(page, 'results');
    await expect(page.getByTestId('results-row').first()).toBeVisible();
    expect(await page.getByTestId('results-row').count()).toBeGreaterThan(0);
  });
});
