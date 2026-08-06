(function () {
  'use strict';

  const state = {
    sessionId: null,
    analyzed: false,
    activeInvestigationId: null,
    activeArtifactId: null,
    tailTimer: null,
    timelineEvents: [],
    timelineOffset: 0,
    timelineTruncated: false,
    highlightLineNumber: null,
    activeTimelineEventId: null,
    activeCrashArtifactId: null,
    crashReport: null,
    crashNotSupported: false,
    pstackBody: null,
    activeFaultThreadId: null,
  };

  const TIMELINE_PAGE_SIZE = 100;

  const statusEl = document.getElementById('status');
  const summaryEl = document.getElementById('summary');
  const tailOutputEl = document.getElementById('tailOutput');
  const tableBody = document.querySelector('#resultsTable tbody');
  const extensionsList = document.getElementById('extensionsList');
  const investigationsList = document.getElementById('investigationsList');
  const artifactList = document.getElementById('artifactList');
  const fileInput = document.getElementById('fileInput');
  const analyzeBtn = document.getElementById('analyzeBtn');
  const investigateBtn = document.getElementById('investigateBtn');
  const exportBtn = document.getElementById('exportBtn');
  const askBtn = document.getElementById('askBtn');
  const searchInput = document.getElementById('searchInput');
  const filterInput = document.getElementById('filterInput');
  const askInput = document.getElementById('askInput');
  const investigationNameInput = document.getElementById('investigationNameInput');
  const createInvestigationBtn = document.getElementById('createInvestigationBtn');
  const saveInvestigationBtn = document.getElementById('saveInvestigationBtn');
  const addLogArtifactBtn = document.getElementById('addLogArtifactBtn');
  const logArtifactInput = document.getElementById('logArtifactInput');
  const noteTitleInput = document.getElementById('noteTitleInput');
  const noteBodyInput = document.getElementById('noteBodyInput');
  const addNoteBtn = document.getElementById('addNoteBtn');
  const addPstackBtn = document.getElementById('addPstackBtn');
  const pstackInput = document.getElementById('pstackInput');
  const tailStartBtn = document.getElementById('tailStartBtn');
  const tailStopBtn = document.getElementById('tailStopBtn');
  const timelineList = document.getElementById('timelineList');
  const timelineEmpty = document.getElementById('timelineEmpty');
  const timelineWarnings = document.getElementById('timelineWarnings');
  const timelineTable = document.getElementById('timelineTable');
  const timelineLoadMoreBtn = document.getElementById('timelineLoadMoreBtn');
  const crashEmpty = document.getElementById('crashEmpty');
  const crashWarnings = document.getElementById('crashWarnings');
  const crashContent = document.getElementById('crashContent');
  const crashStatusBadge = document.getElementById('crashStatusBadge');
  const crashSignal = document.getElementById('crashSignal');
  const crashSummaryText = document.getElementById('crashSummaryText');
  const crashThreadList = document.getElementById('crashThreadList');
  const crashObservations = document.getElementById('crashObservations');
  const pstackViewerTitle = document.getElementById('pstackViewerTitle');
  const pstackViewer = document.getElementById('pstackViewer');

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

  function setInvestigationActionsEnabled(enabled) {
    saveInvestigationBtn.disabled = !enabled;
    addLogArtifactBtn.disabled = !enabled;
    addNoteBtn.disabled = !enabled;
    addPstackBtn.disabled = !enabled;
  }

  function isCrashAnalyzableType(type) {
    return type === 'pstack' || type === 'core';
  }

  function findPreferredCrashArtifact(artifacts) {
    const pstack = artifacts.find(function (artifact) {
      return artifact.type === 'pstack';
    });

    if (pstack) {
      return pstack;
    }

    return artifacts.find(function (artifact) {
      return artifact.type === 'core';
    }) || null;
  }

  async function fetchCrashAnalysis(investigationId, artifactId) {
    const response = await api(
      '/api/v1/investigations/' + investigationId + '/artifacts/' + artifactId + '/crash-analysis'
    );
    const text = await response.text();
    let payload;

    try {
      payload = JSON.parse(text);
    } catch (error) {
      payload = { raw: text };
    }

    if (response.status === 409) {
      return { notSupported: true, payload: payload };
    }

    if (!response.ok) {
      throw new Error('HTTP ' + response.status + ': ' + text);
    }

    return { notSupported: false, payload: payload };
  }

  function crashReportFromPayload(payload) {
    const data = payload.data || payload;
    return data.report || null;
  }

  function renderCrashStatusBadge(status) {
    crashStatusBadge.textContent = status || 'unknown';
    crashStatusBadge.className = 'crash-status-badge crash-status-badge--' + (status || 'unknown');
  }

  function renderCrashThreads(threads, artifactType) {
    crashThreadList.innerHTML = '';
    const threadList = threads || [];
    const faultThreadId = state.crashReport && state.crashReport.faultThreadId;
    const canJump = artifactType === 'pstack';

    threadList.forEach(function (thread) {
      const item = document.createElement('li');
      const isFault = thread.isFaultThread
        || (faultThreadId != null && String(thread.id) === String(faultThreadId));
      item.className = 'crash-thread' + (isFault ? ' crash-thread--fault' : '');

      if (canJump) {
        item.classList.add('crash-thread--clickable');
        item.tabIndex = 0;
      }

      if (state.activeFaultThreadId != null && String(thread.id) === String(state.activeFaultThreadId)) {
        item.classList.add('crash-thread--active');
      }

      const header = document.createElement('div');
      header.className = 'crash-thread-header';
      header.textContent = thread.name || ('Thread ' + thread.id);

      if (isFault) {
        const faultLabel = document.createElement('span');
        faultLabel.className = 'crash-thread-fault-label';
        faultLabel.textContent = 'fault thread';
        header.appendChild(faultLabel);
      }

      item.appendChild(header);

      const frameList = document.createElement('ul');
      frameList.className = 'crash-frame-list';
      (thread.frames || []).forEach(function (frame) {
        const frameItem = document.createElement('li');
        const location = frame.location ? ' at ' + frame.location : '';
        const module = frame.module ? ' [' + frame.module + ']' : '';
        frameItem.textContent = '#' + frame.index + '  ' + (frame.address || '')
          + ' in ' + (frame.symbol || '?') + module + location;
        frameList.appendChild(frameItem);
      });
      item.appendChild(frameList);

      if (canJump) {
        const activate = function () {
          jumpToFaultThread(thread).catch(function (error) {
            setStatus('Pstack jump error: ' + error.message);
          });
        };
        item.addEventListener('click', activate);
        item.addEventListener('keydown', function (keyEvent) {
          if (keyEvent.key === 'Enter' || keyEvent.key === ' ') {
            keyEvent.preventDefault();
            activate();
          }
        });
      }

      crashThreadList.appendChild(item);
    });
  }

  function renderCrashObservations(observations) {
    crashObservations.innerHTML = '';
    (observations || []).forEach(function (observation) {
      const item = document.createElement('li');
      item.textContent = observation;
      crashObservations.appendChild(item);
    });
  }

  function splitPstackThreads(text) {
    const lines = String(text || '').split(/\r?\n/);
    const blocks = [];
    let current = null;

    lines.forEach(function (line) {
      const match = line.match(/^Thread\s+(\d+)\b/);

      if (match) {
        if (current) {
          blocks.push(current);
        }

        current = {
          id: match[1],
          lines: [line],
        };
        return;
      }

      if (current) {
        current.lines.push(line);
      }
    });

    if (current) {
      blocks.push(current);
    }

    return blocks.map(function (block) {
      return {
        id: block.id,
        text: block.lines.join('\n'),
      };
    });
  }

  function renderPstackViewer(text, highlightThreadId) {
    pstackViewer.innerHTML = '';
    const blocks = splitPstackThreads(text);

    if (blocks.length === 0) {
      const pre = document.createElement('pre');
      pre.className = 'crash-pstack-thread';
      pre.textContent = text || '';
      pstackViewer.appendChild(pre);
      return null;
    }

    let highlightedEl = null;

    blocks.forEach(function (block) {
      const pre = document.createElement('pre');
      pre.className = 'crash-pstack-thread';
      pre.dataset.threadId = block.id;
      pre.textContent = block.text;

      if (highlightThreadId != null && String(block.id) === String(highlightThreadId)) {
        pre.classList.add('crash-pstack-thread--highlight');
        highlightedEl = pre;
      }

      pstackViewer.appendChild(pre);
    });

    return highlightedEl;
  }

  async function loadPstackBody(artifactId) {
    if (!state.activeInvestigationId || !artifactId) {
      return null;
    }

    const response = await api(
      '/api/v1/investigations/' + state.activeInvestigationId + '/artifacts/' + artifactId
    );

    if (!response.ok) {
      throw new Error('Could not load pstack artifact body');
    }

    const payload = await response.json();
    const data = payload.data || payload;
    const body = data.body || (data.artifact && data.artifact.body) || '';

    state.pstackBody = body;
    return body;
  }

  async function jumpToFaultThread(thread) {
    if (!thread || !state.crashReport || state.crashReport.artifactType !== 'pstack') {
      return;
    }

    state.activeFaultThreadId = thread.id;
    renderCrashThreads(state.crashReport.threads || [], state.crashReport.artifactType);

    const artifactId = state.activeCrashArtifactId;
    let body = state.pstackBody;

    if (!body && artifactId) {
      body = await loadPstackBody(artifactId);
    }

    pstackViewerTitle.hidden = false;
    pstackViewer.hidden = false;

    const highlightedEl = renderPstackViewer(body || '', thread.id);

    if (highlightedEl) {
      highlightedEl.scrollIntoView({ block: 'center', behavior: 'smooth' });
    }

    setStatus('Jumped to pstack thread ' + (thread.name || thread.id));
  }

  function renderCrashPanel() {
    crashWarnings.textContent = '';
    crashWarnings.hidden = true;
    crashContent.hidden = true;
    pstackViewerTitle.hidden = true;
    pstackViewer.hidden = true;
    pstackViewer.innerHTML = '';

    if (!state.activeInvestigationId) {
      crashEmpty.textContent = 'Select an investigation with a pstack or core artifact.';
      crashEmpty.hidden = false;
      return;
    }

    if (!state.activeCrashArtifactId) {
      crashEmpty.textContent = 'Add a pstack or core artifact to analyze the crash.';
      crashEmpty.hidden = false;
      return;
    }

    if (state.crashNotSupported) {
      crashEmpty.textContent = 'Crash analysis is not supported for this artifact type.';
      crashEmpty.hidden = false;
      renderCrashStatusBadge('not_supported');
      return;
    }

    const report = state.crashReport;

    if (!report) {
      crashEmpty.textContent = 'Could not load crash report.';
      crashEmpty.hidden = false;
      return;
    }

    crashEmpty.hidden = true;
    crashContent.hidden = false;

    renderCrashStatusBadge(report.status);
    crashSignal.textContent = report.signal ? 'Signal: ' + report.signal : '';
    crashSummaryText.textContent = report.summary || '';

    const warnings = (report.warnings || []).slice();

    if (report.status === 'unavailable') {
      warnings.unshift('Crash analysis is unavailable for this artifact.');
    } else if (report.status === 'failed') {
      warnings.unshift('Crash analysis failed for this artifact.');
    }

    if (warnings.length > 0) {
      crashWarnings.textContent = warnings.join('; ');
      crashWarnings.hidden = false;
    }

    renderCrashThreads(report.threads || [], report.artifactType);
    renderCrashObservations(report.observations || []);

    if (report.artifactType === 'pstack' && state.pstackBody) {
      pstackViewerTitle.hidden = false;
      pstackViewer.hidden = false;
      renderPstackViewer(state.pstackBody, state.activeFaultThreadId);
    }
  }

  async function selectCrashArtifact(artifactId, artifactType) {
    if (!state.activeInvestigationId || !artifactId) {
      state.activeCrashArtifactId = null;
      state.crashReport = null;
      state.crashNotSupported = false;
      state.pstackBody = null;
      state.activeFaultThreadId = null;
      renderCrashPanel();
      return;
    }

    state.activeCrashArtifactId = artifactId;
    state.crashNotSupported = false;
    state.crashReport = null;
    state.pstackBody = null;
    state.activeFaultThreadId = null;

    try {
      const result = await fetchCrashAnalysis(state.activeInvestigationId, artifactId);

      if (result.notSupported) {
        state.crashNotSupported = true;
        const report = crashReportFromPayload(result.payload);

        if (report) {
          state.crashReport = report;
        }
      } else {
        state.crashReport = crashReportFromPayload(result.payload);
      }

      if (artifactType === 'pstack' && state.crashReport) {
        await loadPstackBody(artifactId);
      }
    } catch (error) {
      crashEmpty.textContent = 'Could not load crash report: ' + error.message;
      crashEmpty.hidden = false;
      crashContent.hidden = true;
      await refreshArtifactList();
      return;
    }

    await refreshArtifactList();
    renderCrashPanel();
  }

  async function refreshCrashAnalysis() {
    if (!state.activeInvestigationId) {
      state.activeCrashArtifactId = null;
      state.crashReport = null;
      state.crashNotSupported = false;
      state.pstackBody = null;
      state.activeFaultThreadId = null;
      renderCrashPanel();
      return;
    }

    const response = await api('/api/v1/investigations/' + state.activeInvestigationId);
    const payload = await response.json();
    const investigation = (payload.data && payload.data.investigation) || payload.data || payload;
    const artifacts = investigation.artifacts || [];
    const preferred = findPreferredCrashArtifact(artifacts);

    if (!preferred) {
      state.activeCrashArtifactId = null;
      state.crashReport = null;
      state.crashNotSupported = false;
      state.pstackBody = null;
      state.activeFaultThreadId = null;
      renderCrashPanel();
      return;
    }

    const keepSelection = state.activeCrashArtifactId
      && artifacts.some(function (artifact) {
        return artifact.id === state.activeCrashArtifactId && isCrashAnalyzableType(artifact.type);
      });

    const targetId = keepSelection ? state.activeCrashArtifactId : preferred.id;
    const targetArtifact = artifacts.find(function (artifact) {
      return artifact.id === targetId;
    });

    await selectCrashArtifact(targetId, targetArtifact ? targetArtifact.type : preferred.type);
  }

  async function refreshArtifactList() {
    artifactList.innerHTML = '';

    if (!state.activeInvestigationId) {
      return;
    }

    const response = await api('/api/v1/investigations/' + state.activeInvestigationId);
    const payload = await response.json();
    const investigation = (payload.data && payload.data.investigation) || payload.data || payload;
    const artifacts = investigation.artifacts || [];

    artifacts.forEach(function (artifact) {
      const item = document.createElement('li');

      if (artifact.id === state.activeCrashArtifactId && isCrashAnalyzableType(artifact.type)) {
        item.classList.add('crash-artifact-active');
      }

      const label = artifact.name + ' (' + artifact.type + ')';

      if (artifact.isEntry) {
        item.textContent = label + ' [entry] ';
      } else {
        item.textContent = label + ' ';
      }

      if (artifact.metadata && artifact.metadata.role) {
        item.textContent += '[' + artifact.metadata.role + '] ';
      }

      if (artifact.type === 'log') {
        const switchBtn = document.createElement('button');
        switchBtn.type = 'button';
        switchBtn.textContent = state.activeArtifactId === artifact.id ? 'Active' : 'Switch';
        switchBtn.disabled = state.activeArtifactId === artifact.id;
        switchBtn.addEventListener('click', function () {
          switchLogArtifact(artifact.id).catch(function (error) {
            setStatus('Switch error: ' + error.message);
          });
        });
        item.appendChild(switchBtn);
      }

      if (isCrashAnalyzableType(artifact.type)) {
        const analyzeBtn = document.createElement('button');
        analyzeBtn.type = 'button';
        analyzeBtn.textContent = state.activeCrashArtifactId === artifact.id ? 'Analyzing' : 'Analyze crash';
        analyzeBtn.disabled = state.activeCrashArtifactId === artifact.id;
        analyzeBtn.addEventListener('click', function () {
          selectCrashArtifact(artifact.id, artifact.type).catch(function (error) {
            setStatus('Crash analysis error: ' + error.message);
          });
        });
        item.appendChild(analyzeBtn);
      }

      artifactList.appendChild(item);
    });
  }

  function renderTimelineList(events) {
    timelineList.innerHTML = '';

    events.forEach(function (event) {
      const row = document.createElement('tr');
      row.className = 'timeline-event timeline-event--clickable';

      if (event.id && event.id === state.activeTimelineEventId) {
        row.classList.add('timeline-event--active');
      }

      row.innerHTML = '<td>' + escapeHtml(event.timestamp || '') + '</td>'
        + '<td>' + escapeHtml(event.eventType || '') + '</td>'
        + '<td>' + escapeHtml((event.source && event.source.artifactName) || '') + '</td>'
        + '<td>' + escapeHtml(event.message || '') + '</td>';

      row.addEventListener('click', function () {
        jumpToTimelineEvent(event).catch(function (error) {
          setStatus('Jump error: ' + error.message);
        });
      });

      row.addEventListener('keydown', function (keyEvent) {
        if (keyEvent.key === 'Enter' || keyEvent.key === ' ') {
          keyEvent.preventDefault();
          jumpToTimelineEvent(event).catch(function (error) {
            setStatus('Jump error: ' + error.message);
          });
        }
      });

      timelineList.appendChild(row);
    });
  }

  async function refreshTimeline(append) {
    timelineList.innerHTML = '';
    timelineWarnings.textContent = '';
    timelineWarnings.hidden = true;
    timelineLoadMoreBtn.hidden = true;

    if (!state.activeInvestigationId) {
      timelineEmpty.textContent = 'Select an investigation to view its timeline.';
      timelineEmpty.hidden = false;
      timelineTable.hidden = true;
      state.timelineEvents = [];
      state.timelineOffset = 0;
      state.timelineTruncated = false;
      return;
    }

    const offset = append ? state.timelineOffset : 0;
    const path = '/api/v1/investigations/' + state.activeInvestigationId
      + '/timeline?limit=' + TIMELINE_PAGE_SIZE + '&offset=' + offset + '&order=asc';

    const response = await api(path);

    if (!response.ok) {
      timelineEmpty.textContent = 'Could not load timeline.';
      timelineEmpty.hidden = false;
      timelineTable.hidden = true;
      return;
    }

    const payload = await response.json();
    const data = payload.data || payload;
    const events = data.events || [];

    if (!append) {
      state.timelineEvents = events;
      state.timelineOffset = events.length;
    } else {
      state.timelineEvents = state.timelineEvents.concat(events);
      state.timelineOffset += events.length;
    }

    state.timelineTruncated = !!(data.pagination && data.pagination.truncated);
    renderTimelineList(state.timelineEvents);

    if (state.timelineEvents.length === 0) {
      timelineEmpty.textContent = 'No timeline events yet. Add timestamped logs or notes.';
      timelineEmpty.hidden = false;
      timelineTable.hidden = true;
    } else {
      timelineEmpty.hidden = true;
      timelineTable.hidden = false;
    }

    if (data.warnings && data.warnings.length > 0) {
      timelineWarnings.textContent = data.warnings.join('; ');
      timelineWarnings.hidden = false;
    }

    timelineLoadMoreBtn.hidden = !state.timelineTruncated;
  }

  async function jumpToTimelineEvent(event) {
    if (!state.activeInvestigationId || !event) {
      return;
    }

    state.activeTimelineEventId = event.id || null;
    renderTimelineList(state.timelineEvents);

    const source = event.source || {};
    const artifactType = source.artifactType || '';
    const artifactId = source.artifactId || event.artifactId;

    if (artifactType === 'log' && artifactId) {
      await openInvestigation(state.activeInvestigationId, artifactId, { skipTimelineRefresh: true });
      state.highlightLineNumber = source.lineNumber != null ? Number(source.lineNumber) : null;

      if (state.analyzed) {
        const payload = await apiJson('/api/v1/investigate', {});
        renderInvestigation(payload, state.highlightLineNumber);
      }

      setStatus('Opened ' + (source.artifactName || 'log')
        + (source.lineNumber != null ? ' at line ' + source.lineNumber : ''));
      return;
    }

    setStatus((event.message || event.eventType || 'Event')
      + ' — ' + (source.artifactName || artifactType || 'artifact'));
  }

  function renderInvestigation(payload, highlightLineNumber) {
    const investigation = payload.investigation || payload.data || payload;
    const lines = investigation.matchingLines || [];
    const count = investigation.matchingLineCount != null
      ? investigation.matchingLineCount
      : lines.length;
    const lineToHighlight = highlightLineNumber != null
      ? highlightLineNumber
      : state.highlightLineNumber;

    summaryEl.textContent = JSON.stringify({
      matchingLineCount: count,
      indexedLineCount: investigation.indexedLineCount,
      totalLines: investigation.totalLines,
    }, null, 2);

    tableBody.innerHTML = '';
    let highlightedRow = null;

    lines.slice(0, 200).forEach(function (line) {
      const row = document.createElement('tr');
      const lineNumber = line.lineNumber != null ? Number(line.lineNumber) : null;

      if (lineToHighlight != null && lineNumber === Number(lineToHighlight)) {
        row.classList.add('results-row--highlight');
        highlightedRow = row;
      }

      row.innerHTML = '<td>' + (line.lineNumber || '') + '</td>'
        + '<td>' + (line.level || '') + '</td>'
        + '<td>' + escapeHtml(line.message || line.content || '') + '</td>';
      tableBody.appendChild(row);
    });

    if (highlightedRow) {
      highlightedRow.scrollIntoView({ block: 'center', behavior: 'smooth' });
    }
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

  async function refreshInvestigations() {
    const response = await api('/api/v1/investigations');
    const payload = await response.json();
    const investigations = (payload.data && payload.data.investigations) || [];
    investigationsList.innerHTML = '';

    investigations.forEach(function (investigation) {
      const item = document.createElement('li');
      const openBtn = document.createElement('button');
      openBtn.type = 'button';
      openBtn.textContent = 'Open';
      openBtn.addEventListener('click', function () {
        openInvestigation(investigation.id).catch(function (error) {
          setStatus('Open error: ' + error.message);
        });
      });

      item.textContent = investigation.name + ' (' + investigation.id.slice(0, 8) + '…) ';
      item.appendChild(openBtn);
      investigationsList.appendChild(item);
    });
  }

  async function openInvestigation(investigationId, artifactId, options) {
    const opts = options || {};
    const body = artifactId ? { artifactId: artifactId } : {};
    const payload = await apiJson('/api/v1/investigations/' + investigationId + '/open', body, 'POST');
    state.activeInvestigationId = investigationId;
    state.activeArtifactId = (payload.data && payload.data.artifactId) || artifactId || null;
    state.analyzed = !!(payload.data && payload.data.summary && payload.data.summary.hasModel);
    investigateBtn.disabled = !state.analyzed;
    exportBtn.disabled = !state.analyzed;
    askBtn.disabled = !state.analyzed;
    setInvestigationActionsEnabled(true);
    summaryEl.textContent = JSON.stringify(payload.data || payload, null, 2);
    await refreshArtifactList();

    if (!opts.skipTimelineRefresh) {
      await refreshTimeline(false);
    }

    await refreshCrashAnalysis();

    setStatus('Opened investigation ' + investigationId);
  }

  async function switchLogArtifact(artifactId) {
    if (!state.activeInvestigationId) {
      throw new Error('No investigation selected');
    }

    await openInvestigation(state.activeInvestigationId, artifactId);
    setStatus('Switched to artifact ' + artifactId);
  }

  async function createInvestigation() {
    const name = investigationNameInput.value.trim() || ('investigation-' + Date.now());
    const payload = await apiJson('/api/v1/investigations', {
      name: name,
      captureSession: state.analyzed,
    });
    state.activeInvestigationId = payload.data && payload.data.id;
    setInvestigationActionsEnabled(!!state.activeInvestigationId);
    await refreshInvestigations();
    await refreshArtifactList();
    await refreshTimeline(false);
    await refreshCrashAnalysis();
    setStatus('Created investigation ' + name);
  }

  async function saveInvestigation() {
    if (!state.activeInvestigationId) {
      throw new Error('No investigation selected');
    }

    await apiJson('/api/v1/sessions/save', { investigationId: state.activeInvestigationId });
    await refreshInvestigations();
    setStatus('Saved investigation snapshot');
  }

  async function addLogArtifact(file) {
    if (!state.activeInvestigationId) {
      throw new Error('No investigation selected');
    }

    const uploadPayload = await uploadFileForPath(file);
    const sourcePath = uploadPayload.data && uploadPayload.data.sourcePath;

    if (!sourcePath) {
      throw new Error('Upload did not return sourcePath');
    }

    await apiJson('/api/v1/investigations/' + state.activeInvestigationId + '/artifacts', {
      type: 'log',
      sourcePath: sourcePath,
      name: file.name,
    });
    await refreshInvestigations();
    await refreshArtifactList();
    await refreshTimeline(false);
    await refreshCrashAnalysis();
    setStatus('Added log artifact');
  }

  async function addNoteArtifact() {
    if (!state.activeInvestigationId) {
      throw new Error('No investigation selected');
    }

    const title = noteTitleInput.value.trim();
    const body = noteBodyInput.value.trim();

    if (!title) {
      throw new Error('Note title is required');
    }

    await apiJson('/api/v1/investigations/' + state.activeInvestigationId + '/artifacts', {
      type: 'note',
      title: title,
      body: body,
    });
    await refreshInvestigations();
    await refreshArtifactList();
    await refreshTimeline(false);
    await refreshCrashAnalysis();
    setStatus('Added note artifact');
  }

  async function addPstackArtifact(file) {
    if (!state.activeInvestigationId) {
      throw new Error('No investigation selected');
    }

    const uploadPayload = await uploadFileForPath(file);
    const sourcePath = uploadPayload.data && uploadPayload.data.sourcePath;

    if (!sourcePath) {
      throw new Error('Upload did not return sourcePath');
    }

    await apiJson('/api/v1/investigations/' + state.activeInvestigationId + '/artifacts', {
      type: 'pstack',
      sourcePath: sourcePath,
      name: file.name,
    });
    await refreshInvestigations();
    await refreshArtifactList();
    await refreshTimeline(false);
    await refreshCrashAnalysis();
    setStatus('Added pstack artifact');
  }

  async function uploadFileForPath(file) {
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
    return response.json();
  }

  async function uploadFile(file) {
    const payload = await uploadFileForPath(file);
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
    addLogArtifactBtn.disabled = !state.activeInvestigationId;
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

  createInvestigationBtn.addEventListener('click', function () {
    createInvestigation().catch(function (error) {
      setStatus('Create error: ' + error.message);
    });
  });

  saveInvestigationBtn.addEventListener('click', function () {
    saveInvestigation().catch(function (error) {
      setStatus('Save error: ' + error.message);
    });
  });

  addLogArtifactBtn.addEventListener('click', function () {
    logArtifactInput.click();
  });

  logArtifactInput.addEventListener('change', function () {
    const file = logArtifactInput.files && logArtifactInput.files[0];
    if (!file) {
      return;
    }
    addLogArtifact(file).catch(function (error) {
      setStatus('Add log error: ' + error.message);
    });
    logArtifactInput.value = '';
  });

  addNoteBtn.addEventListener('click', function () {
    addNoteArtifact().catch(function (error) {
      setStatus('Add note error: ' + error.message);
    });
  });

  addPstackBtn.addEventListener('click', function () {
    pstackInput.click();
  });

  pstackInput.addEventListener('change', function () {
    const file = pstackInput.files && pstackInput.files[0];
    if (!file) {
      return;
    }
    addPstackArtifact(file).catch(function (error) {
      setStatus('Add pstack error: ' + error.message);
    });
    pstackInput.value = '';
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

  timelineLoadMoreBtn.addEventListener('click', function () {
    refreshTimeline(true).catch(function (error) {
      setStatus('Timeline error: ' + error.message);
    });
  });

  createWorkspace()
    .then(loadNoopConfig)
    .then(refreshExtensions)
    .then(refreshInvestigations)
    .then(function () {
      setStatus('Ready — session ' + state.sessionId);
    })
    .catch(function (error) {
      setStatus('Init error: ' + error.message);
    });
})();
