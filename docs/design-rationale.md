# Design Rationale — EternaTest

**Document purpose:**  
This document explains *why* EternaTest is designed the way it is. It complements the architecture diagram and ADRs by capturing the guiding principles, trade‑offs, and constraints that shaped the system.

---

## 1. Problem Context and Goals

EternaTest is designed to evaluate external systems and repositories in a **repeatable, observable, and extensible** manner. The system must:

- Support multiple evaluation targets (e.g., services, repositories, APIs)
- Produce consistent, comparable metrics
- Remain simple enough for rapid iteration and code review
- Be easy to extend without destabilizing existing behavior

The primary audience includes:
- Developers integrating new evaluators
- Reviewers assessing system design and code quality
- Future maintainers evolving evaluation logic and metrics

---

## 2. Non‑Goals

The following concerns are deliberately out of scope for the current design:

- Highly optimized, low‑latency execution
- Distributed scheduling or orchestration
- Persistent long‑term storage or analytics pipelines
- End‑user UI or visualization layer

This allows the system to prioritize **clarity and correctness over premature optimization**.

---

## 3. High‑Level Architectural Philosophy

The design follows four core principles:

1. **Single Responsibility per Component** – Each component addresses exactly one concern (evaluation, aggregation, exposure).
2. **Explicit Data Flow** – Evaluation results move in a clearly defined sequence rather than via side effects.
3. **Replaceability of External Dependencies** – External integrations are isolated behind thin adapters.
4. **Reviewability and Testability** – The system is structured to be understandable during code review without requiring deep domain context.

---

## 4. Key Design Decisions and Rationale

### 4.1 Orchestration‑Driven Rather Than Event‑Driven
**Decision:** Use an explicit orchestration flow instead of implicit event propagation.

**Rationale:**
- Easier to trace execution during debugging and review
- Deterministic behavior improves reproducibility
- Lower cognitive overhead for new contributors

**Trade‑off:** Less flexible for asynchronous or high‑parallel workloads, which are not current requirements.

### 4.2 Dedicated Evaluator Component
**Decision:** Centralize evaluation logic in the Evaluator component.

**Rationale:**
- Encapsulates domain‑specific logic
- Prevents leakage of evaluation rules into API or metrics layers
- Simplifies testing with mocks and fixtures

**Trade‑off:** Evaluator may grow over time and requires discipline to remain modular internally.

### 4.3 Metrics as a First‑Class Domain Concept
**Decision:** Compute and aggregate metrics in a dedicated Metrics component.

**Rationale:**
- Raw evaluation output and derived insights are deliberately separated
- Enables consistent scoring across different evaluation sources
- Supports future changes in metric computation without changing evaluators

**Trade‑off:** Adds an extra processing step, but improves long‑term maintainability.

### 4.4 Thin API Layer
**Decision:** Expose existing metrics via a thin API without embedding business logic.

**Rationale:**
- Prevents duplication of evaluation or aggregation rules
- Keeps public interfaces stable even as internals evolve
- Simplifies API testing and documentation

**Trade‑off:** API cannot easily customize responses per client without upstream changes.

### 4.5 Adapter‑Based External Integrations
**Decision:** Access external services (e.g., GitHub API, test targets) via thin adapters.

**Rationale:**
- Shields core logic from API volatility
- Simplifies mocking external systems in tests
- Enables incremental integration of new services

**Trade‑off:** Slight increase in boilerplate, accepted in favor of long‑term stability.

---

## 5. Data Flow Summary

1. **EternaTest Orchestrator** initiates evaluation  
2. **Evaluator** interacts with external services via adapters  
3. Evaluation results are normalized and forwarded to **Metrics**  
4. **Metrics** aggregates, scores, and stores interim results  
5. **API** exposes computed metrics for consumption

This linear flow prioritizes **traceability** over maximal concurrency.

---

## 6. Error Handling and Failure Isolation

- Evaluator failures are contained and reported as structured results
- External service issues do not directly impact metrics computation
- API remains operational even when evaluations fail or degrade

---

## 7. Scalability and Evolution Considerations

The current design supports future enhancements such as:

- Parallel evaluation execution
- Pluggable metric strategies
- Persistent metric storage
- Asynchronous or scheduled evaluations

These can be introduced **without reworking core abstractions**.

---

## 8. Design Quality Attributes

| Attribute       | Design Support                                   |
|-----------------|---------------------------------------------------|
| Maintainability | Clear separation of concerns                      |
| Testability     | Isolated components, adapters                     |
| Extensibility   | Pluggable evaluators and metrics                  |
| Observability   | Explicit data flow and metrics                    |
| Reviewability   | Minimal hidden behavior                           |

---

## 9. Summary

EternaTest deliberately favors **clarity, explicitness, and extensibility** over complexity. The architecture reflects a system designed for **engineering correctness, code review, and long‑term evolution**, aligned with real‑world constraints rather than speculative scaling needs.

---

## 10. Related Documents

- **Architecture Overview** (diagram + component responsibilities):  
  architecture/overview.md

- **ADRs** (selected):  
  - adr/ADR-001-modular-evaluation-pipeline.md — explains why we separated Evaluator, Metrics, and API.  
  - adr/ADR-002-metrics-first-class.md — rationale for a dedicated metrics layer.  
  - adr/ADR-003-external-integrations.md — how we isolate vendor APIs (e.g., GitHub).