import { test, expect } from '@playwright/test';

import {
  addLogArtifact,
  addPstackArtifact,
  analyzeHeader,
  createEvidenceLink,
  createInvestigation,
  openFirstArtifact,
  story6AppLogPath,
  story6SyslogPath,
  switchBottomTab,
  syslogPath,
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
    await expect(page.getByTestId('related-evidence-panel')).toBeVisible();

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

  test('Crash Timeline: pstack crash.summary on timeline jumps to Crash evidence', async ({ page }) => {
    await createInvestigation(page);
    await addLogArtifact(page);
    await addPstackArtifact(page);
    await openFirstArtifact(page);

    await switchBottomTab(page, 'timeline');
    const crashRow = page.locator('[data-testid="timeline-row"]', { hasText: 'Crash' });
    await expect(crashRow.first()).toBeVisible({ timeout: 15_000 });
    await expect(crashRow.first()).toContainText(/SIGSEGV|SessionManager/i);

    await crashRow.first().click();
    await expect(page.getByTestId('crash-signal')).toContainText('SIGSEGV', { timeout: 15_000 });
    await expect(page.getByTestId('crash-fault-thread')).toBeVisible();
  });

  test('Story 5: evidence link decoration, Related Evidence panel, and jump', async ({ page }) => {
    await createInvestigation(page);
    await addLogArtifact(page);
    await addLogArtifact(page, syslogPath);
    await addPstackArtifact(page);
    await openFirstArtifact(page);
    await analyzeHeader(page);

    await switchBottomTab(page, 'timeline');
    const timelineRows = page.getByTestId('timeline-row');
    await expect(timelineRows.first()).toBeVisible({ timeout: 15_000 });
    expect(await timelineRows.count()).toBeGreaterThan(1);

    await timelineRows.first().click();
    await expect(page.getByTestId('related-evidence-panel')).toBeVisible();
    await expect(page.locator('.subsection-title', { hasText: 'Related Evidence' })).toBeVisible();
    await expect(page.getByRole('columnheader', { name: 'Connections' })).toBeVisible();

    await createEvidenceLink(page);

    await expect(page.getByTestId('timeline-link-badge').first()).toBeVisible();
    expect(await page.getByTestId('timeline-link-badge').count()).toBeGreaterThanOrEqual(1);

    await expect(page.getByTestId('related-evidence-row').first()).toBeVisible();
    await page.getByTestId('related-evidence-row').first().click();
    await expect(page.getByTestId('status')).toContainText(/Opened|line|Jump/);
  });

  test('Story 6 positive: suggested connections accept creates Related Evidence link', async ({ page }) => {
    await createInvestigation(page);
    await addLogArtifact(page, story6AppLogPath);
    await addLogArtifact(page, story6SyslogPath);
    await openFirstArtifact(page);

    await switchBottomTab(page, 'timeline');
    const timelineRows = page.getByTestId('timeline-row');
    await expect(timelineRows.first()).toBeVisible({ timeout: 15_000 });

    await timelineRows.first().click();
    await expect(page.getByTestId('suggested-connections-panel')).toBeVisible();
    await expect(page.locator('.subsection-title', { hasText: 'Suggested connections' })).toBeVisible();
    await expect(page.getByTestId('suggested-connection-row').first()).toContainText('request_id=abc-123');

    await page.getByTestId('suggested-connection-accept').first().click();
    await expect(page.getByTestId('status')).toContainText('Connection added from suggestion');
    await expect(page.getByTestId('related-evidence-row').first()).toBeVisible();
    await expect(page.getByTestId('suggested-connections-panel')).toBeHidden();
  });

  test('Story 6 negative: dismiss suggestion without creating evidence link', async ({ page }) => {
    await createInvestigation(page);
    await addLogArtifact(page, story6AppLogPath);
    await addLogArtifact(page, story6SyslogPath);
    await openFirstArtifact(page);

    await switchBottomTab(page, 'timeline');
    const timelineRows = page.getByTestId('timeline-row');
    await expect(timelineRows.first()).toBeVisible({ timeout: 15_000 });
    await timelineRows.first().click();

    await expect(page.getByTestId('suggested-connection-row').first()).toBeVisible();
    await page.getByTestId('suggested-connection-dismiss').first().click();
    await expect(page.getByTestId('status')).toContainText('Suggestion dismissed');
    await expect(page.getByTestId('suggested-connections-panel')).toBeHidden();
    await expect(page.getByTestId('related-evidence-row')).toHaveCount(0);
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
