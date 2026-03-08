---
title: "Azure VM and AKS Platform Bug: When the Retry Storm Becomes the Disaster"
date: 2026-02-02 00:00:00 +0530
categories: [Cloud Infrastructure, Incident Response]
tags: [Azure, Microsoft, VMs, AKS, Kubernetes, VMSS, DevOps, GitHub-Actions, Managed-Identity, Retry-Storm, Platform-Bug, CI-CD, Control-Plane, Cascading-Failure]
author: mohitmishra786
description: "A technical examination of the February 2026 Azure multi-region platform bug that disabled VM provisioning and AKS node scaling, and how the resulting retry storm overwhelmed the managed identity service, amplifying a contained failure into a global disruption of CI/CD pipelines and developer infrastructure."
toc: true
---

## The Fix That Made It Worse

There is a failure pattern in distributed systems that engineers encounter often enough to have a name for it: the retry storm. When a component becomes unavailable, clients typically implement retry logic to handle transient failures. That retry logic, when every client activates it simultaneously against a degraded service, generates a traffic spike that can overwhelm a service that might otherwise have recovered on its own.

The Azure incident on February 2, 2026 is a textbook example of a retry storm turning a contained failure into a broader one. A platform-level bug in Azure's infrastructure caused degraded performance and control plane failures across VM provisioning, scaling, and lifecycle operations in multiple regions simultaneously. That bug was the original problem. What amplified it into a six-and-a-half-hour outage affecting developers globally was the managed identity service being overwhelmed by the retries that the bug triggered.

## What Broke and Why

Azure's managed identity service handles authentication for Azure resources that need to call other Azure services without storing credentials explicitly. When a VM boots, its managed identity agent requests a token. When an AKS node starts, it uses managed identity to authenticate with the container registry. When a DevOps pipeline runs, it uses managed identity to access Azure resources. The service is woven into essentially every automated operation in the Azure ecosystem.

The platform bug caused VM provisioning and scaling operations to fail. Failure triggers retries. The VM extension mechanism, which runs during VM startup to configure the machine, retried authentication requests to the managed identity service when the startup process failed. AKS node provisioning failures generated similar retry patterns. Azure DevOps agents and GitHub Actions runners that were trying to start new VMs for pipeline execution generated their own wave of managed identity requests.

The managed identity service, handling the aggregated retry traffic from failed VM starts across multiple regions, became overloaded. That overload caused managed identity authentication to slow and fail for workloads that were not directly affected by the original bug. Developer teams whose pipelines had nothing to do with the initial failure suddenly found their GitHub Actions workflows failing on authentication errors.

As Fred Hebert wrote in his work on distributed systems, "cascading failures happen when the safety mechanisms designed to handle partial failures interact badly with each other." The retry logic was the safety mechanism. It interacted badly with the shared managed identity service.

## Developer Impact

The practical effect for engineering teams was disorienting. Azure Virtual Machine deployments failed. VMSS scaling events did not complete. AKS clusters could not add nodes to handle increased workloads. This much was visible in Azure's status updates.

Less obvious was the secondary wave. CI/CD pipelines across Azure DevOps and GitHub Actions began failing in ways that looked like authentication problems rather than infrastructure failures. Development teams trying to diagnose their pipeline failures were investigating the wrong layer, looking at service principal configurations and IAM settings when the actual problem was that the managed identity service was being hammered by retry traffic from an unrelated VM provisioning failure.

This is the operational signature of a cascading failure: the symptoms appearing in one place while the root cause is in a completely different part of the system. Teams that had changed nothing about their pipeline configuration were debugging their authentication setup because the blast radius of the original bug had expanded through the retry storm to include services they were not using directly.

## Why This Pattern Keeps Appearing

The retry storm is one of those failure modes that every distributed systems engineer learns about early and then occasionally forgets the implications of at scale. At the scale of Azure's infrastructure, even a small percentage of the total VM fleet failing simultaneously and retrying generates absolute request volumes that can stress even well-provisioned shared services.

The managed identity service is an interesting point of concentration in the Azure ecosystem precisely because it is so broadly used. Its reliability is largely invisible during normal operations because it is fast and reliable enough that teams do not think about it. Its importance becomes apparent when it degrades, because nearly every automated operation in Azure touches it.

Michael Nygard in "Release It!" describes this pattern in detail: "Shared resources become failure nexuses." The managed identity service is exactly this kind of shared resource. It sits in the dependency graph of an enormous fraction of Azure workloads, making it both very reliable (because it must be) and very impactful when it becomes unavailable (because so many things depend on it).

## What Azure Committed to Changing

Post-incident, Azure committed to strengthening traffic throttling controls in the managed identity service. The goal is to prevent any single category of traffic, including retry storms from other failing components, from consuming enough capacity to degrade availability for other callers.

Rate limiting shared services against their callers is technically straightforward but operationally complex, because the throttling thresholds need to be set high enough that normal traffic surges do not trigger them and low enough that abnormal traffic spikes are actually contained. Getting that calibration right requires empirical data from failures like this one, which is a somewhat uncomfortable reality.

For teams architecting CI/CD infrastructure on Azure, the February incident reinforces the value of designing pipelines that can tolerate managed identity failures gracefully, with appropriate circuit breaker logic rather than unbounded retries that contribute to the problem they are trying to work around.

## References

1. Azure Service Health Dashboard, multi-region VM and AKS incident, February 2-3, 2026
2. Microsoft Azure blog, post-incident review, VM VMSS AKS managed identity disruption
3. GitHub Status, Actions runner availability incident, February 2, 2026
4. Fred Hebert, "The Hitchhiker's Guide to Concurrency," on cascading failures in distributed systems
5. Michael Nygard, "Release It! Design and Deploy Production-Ready Software," Pragmatic Programmers
