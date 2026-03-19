# ADR-002: Metrics as a First-Class Component

## Status
Accepted

## Context
Evaluation results need to be aggregated, compared, and exposed consistently across different consumers (API, reports, future tools).

## Decision
All evaluation outputs flow through a dedicated Metrics component responsible for:
- Aggregation
- Normalization
- Scoring / summarization

Business logic is not duplicated in Evaluator or API layers.

## Consequences
- Clear separation between raw evaluation and derived insights
- Consistent output format
- Metrics layer becomes a key dependency that must be kept stable

---

### Links
- Back to **Architecture Overview**: ../architecture/overview.md  
- Back to **Design Rationale**: ../design-rationale.md