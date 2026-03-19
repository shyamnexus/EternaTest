# Architecture Overview

This document provides the system context and component responsibilities for **EternaTest**.

![Architecture Diagram](./eterna-test-architecture.png)

> Prefer SVG for diffs and scalability if available: `eterna-test-architecture.svg`.

## Components
- **Orchestrator (EternaTest):** Coordinates evaluation lifecycles and data flow.
- **Evaluator:** Runs evaluations against external systems via adapters.
- **Metrics:** Aggregates outputs, computes scores, exposes a stable domain model.
- **API:** Thin interface that serves computed metrics to clients.
- **External Services / GitHub API:** Integration points accessed via adapters.

## Why this structure?
See the **Design Rationale** for the guiding principles and trade‑offs:  
../design-rationale.md

## Decisions that shaped this design
- ../adr/ADR-001-modular-evaluation-pipeline.md
- ../adr/ADR-002-metrics-first-class.md
- ../adr/ADR-003-external-integrations.md