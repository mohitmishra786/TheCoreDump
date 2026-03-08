---
title: "AWS CloudFront DNS Failure: One CDN Layer, Twenty Platforms, Zero Warning"
date: 2026-02-10 00:00:00 +0530
categories: [Cloud Infrastructure, Incident Response]
tags: [AWS, CloudFront, DNS, NXDOMAIN, Route53, ACM, Lambda-Edge, CDN, Outage, Cascade, Salesforce, Discord, Healthcare, EdTech, Concentration-Risk]
author: mohitmishra786
description: "A technical examination of the February 2026 AWS CloudFront DNS failure, where NXDOMAIN responses propagated across 8 interconnected AWS services, simultaneously disabling 20+ platforms spanning healthcare, education, HR, CRM, AI, and developer tools."
toc: true
---

## The NXDOMAIN That Ate the Internet

DNS failures are particularly cruel because of what they look like to end users. The service does not return an error page. It returns nothing. The connection times out. No error message to report, no HTTP status code to log, just absence. That is what began happening for a wide swath of internet services on the morning of February 10, 2026, when AWS CloudFront started returning NXDOMAIN responses for customer distributions.

NXDOMAIN means "this domain does not exist." It is the DNS equivalent of saying the address you're looking for was never built. When CloudFront infrastructure began telling the internet that customer distributions did not exist, every service relying on those distributions became unreachable. Not degraded. Unreachable.

The failure originated within CloudFront's DNS resolution infrastructure and propagated through a chain of eight interconnected AWS services: CloudFront itself, Route 53, ACM (certificate management), Lambda@Edge, and several adjacent control plane components. The acute phase lasted approximately one hour, but full propagation recovery took closer to seven hours as DNS TTLs expired and new records established across the global resolver network.

## Twenty Platforms, Zero Common Thread

What made this incident particularly illustrative of cloud concentration risk was the cross-industry blast radius. The services that went offline simultaneously had essentially nothing in common with each other except their shared dependency on AWS CloudFront.

Salesforce lost availability for customers routing through affected CloudFront distributions. Adobe's services showed degradation. Discord went offline for a significant portion of its user base. DrFirst, a healthcare technology platform used by clinicians for medication management, stopped responding. McGraw Hill's educational platform became inaccessible. Clever, an edtech single sign-on used by millions of K-12 students, failed. UKG, an HR and workforce management platform used by enterprises, went down. TestRail, a test case management tool used by development teams, stopped functioning. Anthropic's Claude AI became temporarily unavailable.

There is no organizational relationship between a healthcare medication platform and a Discord gaming community server. Their shared moment of unavailability was purely architectural, both happened to rely on the same CDN DNS layer.

John Allspaw, who spent years thinking about these problems at Etsy and Flickr, observed that "the failure modes of complex systems are often invisible until they manifest." The dependency on CloudFront was invisible to DrFirst's clinician users until the moment it mattered.

## What IsDown Saw First

A noteworthy operational detail from this incident: IsDown, an independent outage monitoring service, detected the CloudFront failure 23 minutes before AWS published its first update to the service health dashboard. That 23-minute gap, where customer-facing platforms were experiencing failures but the provider had not acknowledged the issue, is a recurring frustration in cloud incidents.

For incident response teams at the affected platforms, the first 20 minutes of this kind of failure are often the most chaotic. Teams see their services down, check their own infrastructure, find nothing wrong, look at third-party dependencies, and eventually, if they know where to look, find that a CDN layer they had not changed is causing the problem. The provider's silence during that window means teams are working without confirmation that the issue is external and outside their control.

This pattern, where external monitoring services detect CDN failures before the CDN provider acknowledges them, is not unique to this incident. It reflects the difficulty of timely status page communication at the scale and complexity of AWS's infrastructure.

## The Concentration Risk Problem Is Not New, Just Larger

The February 10 incident is an excellent data point for a conversation that the cloud industry has been having, somewhat unproductively, for several years. When a significant fraction of internet services route through a small number of CDN providers, the failure blast radius of any individual provider expands proportionally.

This is not a criticism of any particular architectural decision. CloudFront, Cloudflare, Akamai, and Fastly all provide genuine value that justifies their use. The issue is systemic, not vendor-specific. Consolidation of internet traffic through a small number of intermediaries creates correlated failure risk across organizations that consider themselves entirely independent.

For any service where downtime has direct patient, student, or user safety implications, like a healthcare medication platform, the question of CDN redundancy should be part of the architecture conversation, not an afterthought discovered during an incident.

Multi-CDN strategies exist and work. They add operational complexity and cost. The February 10 incident provides a concrete business case for teams that need one.

## The Recovery Wrinkle

One aspect of DNS failures that software infrastructure teams sometimes underestimate is the recovery timeline. Fixing the underlying CDN issue does not instantly restore service. DNS records have TTL values, and resolvers across the internet cache those records according to those TTLs.

When CloudFront began returning NXDOMAIN, many resolvers cached that negative response. Clearing those caches requires waiting for TTLs to expire or, in some configurations, actively purging resolver caches. This is why the acute failure phase of about one hour translated into a seven-hour full recovery window. The infrastructure was fixed, but the internet's memory of the failure persisted until DNS propagation completed.

Teams that had set long TTL values on their CloudFront distributions experienced longer recovery tails. It is a design decision that is easy to optimize incorrectly in the direction of caching efficiency at the expense of recovery speed.

## References

1. AWS Service Health Dashboard, CloudFront incident, February 10-11, 2026
2. IsDown blog, "CloudFront failure detected 23 minutes before AWS status update," February 2026
3. The Register, "AWS CloudFront DNS issue takes down 20+ platforms," February 11, 2026
4. TechRadar, "Cloudfront NXDOMAIN failure hits healthcare, edtech, Discord simultaneously," February 2026
5. NetworkWorld, "CDN concentration risk: February 2026 AWS incident analysis," February 2026
