# ADR-003: External Integrations via Thin Adapters

## Status
Accepted

## Context
EternaTest integrates with external systems such as GitHub or test targets, which may change APIs or behavior over time.

## Decision
All external interactions are implemented behind thin adapter interfaces. Core evaluation and metrics logic remains unaware of vendor-specific details.

## Consequences
- External services can be swapped or mocked easily
- Reduced impact of API changes
- Slight upfront effort to define adapter interfaces

---

### Links
- Back to **Architecture Overview**: ../architecture/overview.md  
- Back to **Design Rationale**: ../design-rationale.md