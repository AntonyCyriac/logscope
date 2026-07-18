# GitHub Configuration

Documentation for the `.github/` directory. This file is intentionally not named `README.md` so it does not override the repository homepage.

## Purpose

This directory contains GitHub-specific configuration, automation, and workflows used to maintain the LogScope project.

These files support the project's engineering processes and help ensure consistent quality across all contributions.

---

## Default Branch

The default branch for the repository is:

```text
master
```

All pull requests should target the `master` branch.

---

## Build Status Badge

Once the Continuous Integration (CI) workflow is successfully running, add the build status badge to the top of the `README.md`.

```markdown
![CI](https://github.com/AntonyCyriac/logscope/actions/workflows/ci.yml/badge.svg)
```

The badge automatically reflects the status of the default branch (`master`) and provides a quick indication of the project's build health.

---

## Directory Structure

```text
.github/
│
├── workflows/
│   └── ci.yml
│
├── PULL_REQUEST_TEMPLATE.md
│
├── ISSUE_TEMPLATE/          (Future)
│
├── CODEOWNERS               (Future)
│
└── dependabot.yml           (Future)
```

---

## Workflows

The `workflows/` directory contains GitHub Actions pipelines.

### Current Workflow

| Workflow | Purpose |
|----------|---------|
| `ci.yml` | Configure, build and test the project on every push and pull request |

### Future Workflows

- Release Automation
- Documentation Validation
- Code Coverage
- Static Analysis
- Security Scanning

---

## Pull Request Template

The pull request template helps contributors provide consistent information when submitting changes.

The template should describe:

- Purpose of the change
- Related issue(s)
- Testing performed
- Documentation updates
- Additional notes

---

## Future Enhancements

As the project evolves, this directory may include:

- Issue templates
- CODEOWNERS
- Dependabot configuration
- Release workflows
- Security workflows
- Community health files

---

## Engineering Philosophy

GitHub automation should remain:

- Simple
- Incremental
- Reliable
- Easy to understand
- Easy to maintain

Automation should evolve alongside the project and support the engineering workflow without introducing unnecessary complexity.
