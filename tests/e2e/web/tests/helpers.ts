import * as path from 'node:path';
import { expect, type Page } from '@playwright/test';

const REPO_ROOT = path.resolve(__dirname, '../../..');

export const sampleLogPath = path.join(REPO_ROOT, 'samples', 'sample.log');
export const pstackPath = path.join(REPO_ROOT, 'samples', 'pstack.txt');

export async function waitForReady(page: Page): Promise<void> {
  await page.goto('/');
  await expect(page.getByTestId('status')).toContainText('Ready', { timeout: 20_000 });
}

export async function createInvestigation(page: Page, name = 'e2e-investigation'): Promise<void> {
  await page.locator('#investigationNameInput').fill(name);
  await page.getByTestId('header-new-investigation').click();
  await expect(page.getByTestId('status')).toContainText('Created investigation');
  await expect(page.getByTestId('add-log-artifact')).toBeEnabled();
}

export async function addLogArtifact(page: Page, filePath = sampleLogPath): Promise<void> {
  const fileChooserPromise = page.waitForEvent('filechooser');
  await page.getByTestId('add-log-artifact').click();
  const fileChooser = await fileChooserPromise;
  await fileChooser.setFiles(filePath);
  await expect(page.getByTestId('status')).toContainText('Added log artifact');
  await expect(page.getByTestId('artifact-item')).toHaveCount(1);
}

export async function addPstackArtifact(page: Page, filePath = pstackPath): Promise<void> {
  const fileChooserPromise = page.waitForEvent('filechooser');
  await page.getByTestId('add-pstack-artifact').click();
  const fileChooser = await fileChooserPromise;
  await fileChooser.setFiles(filePath);
  await expect(page.getByTestId('status')).toContainText('Added pstack artifact');
}

export async function openFirstArtifact(page: Page): Promise<void> {
  await page.getByTestId('artifact-item').first().getByTestId('artifact-action-open').click();
}

export async function analyzeHeader(page: Page): Promise<void> {
  await page.getByTestId('header-analyze').click();
  await expect(page.getByTestId('status')).toContainText('Analyzed', { timeout: 30_000 });
}

export async function switchBottomTab(page: Page, tab: 'timeline' | 'crash' | 'ai' | 'results'): Promise<void> {
  await page.getByTestId(`bottom-tab-${tab}`).click();
}
