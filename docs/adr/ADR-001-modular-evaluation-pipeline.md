# ADR-001: Modular Evaluation Pipeline

## Status
Accepted

## Context
EternaTest needs to evaluate different external services and sources while remaining easy to extend, test, and reason about during code review.

## Decision
We separate the system into distinct modules:
- Evaluator
- Metrics
- API
- Orchestration layer (EternaTest)

Each module has a single, well-defined responsibility and minimal coupling.

## Consequences
- New evaluators or metric strategies can be added with minimal changes
- Easier unit testing and mocking
- Slight increase in module boundaries and interfaces

---

### Links
- Back to **Architecture Overview**: ../architecture/overview.md  
- Back to **Design Rationale**: ../design-rationale.md