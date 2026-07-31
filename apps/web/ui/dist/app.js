(function () {
  'use strict';

  const state = {
    sessionId: null,
    analyzed: false,
    activeWorkspaceId: null,
    tailTimer: null,
  };

  const statusEl = document.getElementById('status');
  const summaryEl = document.getElementById('summary');
  const tailOutputEl = document.getElementById('tailOutput');
  const tableBody = document.querySelector('#resultsTable tbody');
  const extensionsList = document.getElementById('extensionsList');
  const sharedWorkspacesList = document.getElementById('sharedWorkspacesList');
  const fileInput = document.getElementById('fileInput');
  const analyzeBtn = document.getElementById('analyzeBtn');
  const investigateBtn = document.getElementById('investigateBtn');
  const exportBtn = document.getElementById('exportBtn');
  const askBtn = document.getElementById('askBtn');
  const searchInput = document.getElementById('searchInput');
  const filterInput = document.getElementById('filterInput');
  const askInput = document.getElementById('askInput');
  const workspaceNameInput = document.getElementById('workspaceNameInput');
  const createWorkspaceBtn = document.getElementById('createWorkspaceBtn');
  const saveWorkspaceBtn = document.getElementById('saveWorkspaceBtn');
  const tailStartBtn = document.getElementById('tailStartBtn');
  const tailStopBtn = document.getElementById('tailStopBtn');

  function setStatus(text) {
    statusEl.textContent = text;
  }

  function sessionHeaders(extra) {
    const headers = Object.assign({ 'X-LogScope-Session': state.sessionId }, extra || {});
    return headers;
  }

  async function api(path, options) {
    const response = await fetch(path, Object.assign({}, options, {
      headers: sessionHeaders(options && options.headers),
    }));

    const sessionHeader = response.headers.get('X-LogScope-Session');
    if (sessionHeader) {
      state.sessionId = sessionHeader;
    }

    return response;
  }

  async function apiJson(path, body, method) {
    const response = await api(path, {
      method: method || 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify(body || {}),
    });

    const text = await response.text();

    if (!response.ok) {
      throw new Error('HTTP ' + response.status + ': ' + text);
    }

    try {
      return JSON.parse(text);
    } catch (error) {
      return { raw: text };
    }
  }

  function renderInvestigation(payload) {
    const investigation = payload.investigation || payload.data || payload;
    const lines = investigation.matchingLines || [];
    const count = investigation.matchingLineCount != null
      ? investigation.matchingLineCount
      : lines.length;

    summaryEl.textContent = JSON.stringify({
      matchingLineCount: count,
      indexedLineCount: investigation.indexedLineCount,
      totalLines: investigation.totalLines,
    }, null, 2);

    tableBody.innerHTML = '';
    lines.slice(0, 200).forEach(function (line) {
      const row = document.createElement('tr');
      row.innerHTML = '<td>' + (line.lineNumber || '') + '</td>'
        + '<td>' + (line.level || '') + '</td>'
        + '<td>' + escapeHtml(line.message || line.content || '') + '</td>';
      tableBody.appendChild(row);
    });
  }

  function escapeHtml(value) {
    return String(value)
      .replace(/&/g, '&amp;')
      .replace(/</g, '&lt;')
      .replace(/>/g, '&gt;');
  }

  async function createWorkspace() {
    const response = await api('/api/v1/sessions/workspace', { method: 'POST' });
    if (!response.ok) {
      throw new Error('Failed to create workspace');
    }
    await response.json();
    setStatus('Session: ' + state.sessionId);
  }

  async function loadNoopConfig() {
    await apiJson('/api/v1/config/load', { path: 'samples/ai-noop.properties' });
  }

  async function refreshExtensions() {
    const response = await api('/api/v1/extensions');
    const payload = await response.json();
    const extensions = (payload.data || []);
    extensionsList.innerHTML = '';
    extensions.forEach(function (ext) {
      const item = document.createElement('li');
      item.textContent = ext.id + ' — ' + (ext.description || ext.status || '');
      extensionsList.appendChild(item);
    });
  }

  async function refreshSharedWorkspaces() {
    const response = await api('/api/v1/workspaces');
    const payload = await response.json();
    const workspaces = (payload.data && payload.data.workspaces) || [];
    sharedWorkspacesList.innerHTML = '';

    workspaces.forEach(function (workspace) {
      const item = document.createElement('li');
      const openBtn = document.createElement('button');
      openBtn.type = 'button';
      openBtn.textContent = 'Open';
      openBtn.addEventListener('click', function () {
        openSharedWorkspace(workspace.id).catch(function (error) {
          setStatus('Open shared error: ' + error.message);
        });
      });

      item.textContent = workspace.name + ' (' + workspace.id.slice(0, 8) + '…) ';
      item.appendChild(openBtn);
      sharedWorkspacesList.appendChild(item);
    });
  }

  async function openSharedWorkspace(workspaceId) {
    const payload = await apiJson('/api/v1/workspaces/' + workspaceId + '/open', {}, 'POST');
    state.activeWorkspaceId = workspaceId;
    state.analyzed = !!(payload.data && payload.data.summary && payload.data.summary.hasModel);
    investigateBtn.disabled = !state.analyzed;
    exportBtn.disabled = !state.analyzed;
    askBtn.disabled = !state.analyzed;
    saveWorkspaceBtn.disabled = false;
    tailStartBtn.disabled = false;
    summaryEl.textContent = JSON.stringify(payload.data || payload, null, 2);
    setStatus('Opened shared workspace ' + workspaceId);
  }

  async function createSharedWorkspace() {
    const name = workspaceNameInput.value.trim() || ('workspace-' + Date.now());
    const payload = await apiJson('/api/v1/workspaces', {
      name: name,
      captureSession: state.analyzed,
    });
    state.activeWorkspaceId = payload.data && payload.data.id;
    saveWorkspaceBtn.disabled = !state.activeWorkspaceId;
    await refreshSharedWorkspaces();
    setStatus('Created shared workspace ' + name);
  }

  async function saveSharedWorkspace() {
    if (!state.activeWorkspaceId) {
      throw new Error('No shared workspace selected');
    }

    await apiJson('/api/v1/sessions/save', { workspaceId: state.activeWorkspaceId });
    await refreshSharedWorkspaces();
    setStatus('Saved to shared workspace');
  }

  async function uploadFile(file) {
    const formData = new FormData();
    formData.append('file', file);
    const response = await api('/api/v1/sources/upload', {
      method: 'POST',
      body: formData,
    });
    if (!response.ok) {
      const text = await response.text();
      throw new Error('Upload failed: ' + text);
    }
    await response.json();
    analyzeBtn.disabled = false;
    tailStartBtn.disabled = false;
    setStatus('Uploaded ' + file.name);
  }

  async function pollAnalyzeJob(jobId) {
    for (let attempt = 0; attempt < 120; attempt += 1) {
      const response = await api('/api/v1/jobs/' + jobId);
      const payload = await response.json();
      const data = payload.data || payload;

      if (data.status === 'completed') {
        return data.result || data;
      }

      if (data.status === 'failed') {
        throw new Error((data.error && data.error.message) || 'Analyze job failed');
      }

      setStatus('Analyze job running…');
      await new Promise(function (resolve) { setTimeout(resolve, 500); });
    }

    throw new Error('Analyze job timed out');
  }

  async function analyze() {
    const response = await api('/api/v1/analyze', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: '{}',
    });

    if (response.status === 202) {
      const payload = await response.json();
      const jobId = payload.data && payload.data.jobId;
      const data = await pollAnalyzeJob(jobId);
      summaryEl.textContent = JSON.stringify(data, null, 2);
      state.analyzed = true;
    } else if (!response.ok) {
      const text = await response.text();
      throw new Error('HTTP ' + response.status + ': ' + text);
    } else {
      const payload = await response.json();
      const data = payload.data || payload;
      summaryEl.textContent = JSON.stringify(data, null, 2);
      state.analyzed = true;
    }

    investigateBtn.disabled = false;
    exportBtn.disabled = false;
    askBtn.disabled = false;
    saveWorkspaceBtn.disabled = !state.activeWorkspaceId;
    setStatus('Analyzed');
  }

  async function investigate() {
    const body = {};
    if (searchInput.value.trim()) {
      body.search = searchInput.value.trim();
    }
    if (filterInput.value.trim()) {
      body.filter = filterInput.value.trim();
    }
    const payload = await apiJson('/api/v1/investigate', body);
    renderInvestigation(payload);
    setStatus('Investigate complete');
  }

  async function exportHtml() {
    const response = await api('/api/v1/export', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ format: 'html' }),
    });
    if (!response.ok) {
      throw new Error('Export failed');
    }
    const blob = await response.blob();
    const url = URL.createObjectURL(blob);
    const anchor = document.createElement('a');
    anchor.href = url;
    anchor.download = 'logscope-report.html';
    anchor.click();
    URL.revokeObjectURL(url);
    setStatus('Exported HTML report');
  }

  async function askAi() {
    const payload = await apiJson('/api/v1/agent/investigate', {
      ask: askInput.value.trim() || 'errors',
    });
    renderInvestigation(payload.data || payload);
    setStatus('AI ask complete');
  }

  async function pollTail() {
    const response = await api('/api/v1/tail/poll');
    if (response.status === 409) {
      stopTailPolling();
      tailOutputEl.textContent = 'Tail inactive.';
      return;
    }

    if (!response.ok) {
      return;
    }

    const payload = await response.json();
    const data = payload.data || payload;
    const lines = data.lines || [];

    if (lines.length > 0) {
      tailOutputEl.textContent += (tailOutputEl.textContent ? '\n' : '') + lines.join('\n');
    }
  }

  function stopTailPolling() {
    if (state.tailTimer) {
      clearInterval(state.tailTimer);
      state.tailTimer = null;
    }
    tailStartBtn.disabled = false;
    tailStopBtn.disabled = true;
  }

  async function startTail() {
    await apiJson('/api/v1/tail/start', {}, 'POST');
    tailOutputEl.textContent = 'Tail active…\n';
    tailStartBtn.disabled = true;
    tailStopBtn.disabled = false;

    if (state.tailTimer) {
      clearInterval(state.tailTimer);
    }

    state.tailTimer = setInterval(function () {
      pollTail().catch(function () {
        stopTailPolling();
      });
    }, 500);
    setStatus('Tail started');
  }

  async function stopTail() {
    await apiJson('/api/v1/tail/stop', {}, 'POST');
    stopTailPolling();
    setStatus('Tail stopped');
  }

  fileInput.addEventListener('change', function () {
    const file = fileInput.files && fileInput.files[0];
    if (!file) {
      return;
    }
    uploadFile(file).catch(function (error) {
      setStatus('Upload error: ' + error.message);
    });
  });

  analyzeBtn.addEventListener('click', function () {
    analyze().catch(function (error) {
      setStatus('Analyze error: ' + error.message);
    });
  });

  investigateBtn.addEventListener('click', function () {
    investigate().catch(function (error) {
      setStatus('Investigate error: ' + error.message);
    });
  });

  exportBtn.addEventListener('click', function () {
    exportHtml().catch(function (error) {
      setStatus('Export error: ' + error.message);
    });
  });

  askBtn.addEventListener('click', function () {
    askAi().catch(function (error) {
      setStatus('Ask error: ' + error.message);
    });
  });

  createWorkspaceBtn.addEventListener('click', function () {
    createSharedWorkspace().catch(function (error) {
      setStatus('Create shared error: ' + error.message);
    });
  });

  saveWorkspaceBtn.addEventListener('click', function () {
    saveSharedWorkspace().catch(function (error) {
      setStatus('Save shared error: ' + error.message);
    });
  });

  tailStartBtn.addEventListener('click', function () {
    startTail().catch(function (error) {
      setStatus('Tail start error: ' + error.message);
    });
  });

  tailStopBtn.addEventListener('click', function () {
    stopTail().catch(function (error) {
      setStatus('Tail stop error: ' + error.message);
    });
  });

  createWorkspace()
    .then(loadNoopConfig)
    .then(refreshExtensions)
    .then(refreshSharedWorkspaces)
    .then(function () {
      setStatus('Ready — session ' + state.sessionId);
    })
    .catch(function (error) {
      setStatus('Init error: ' + error.message);
    });
})();
