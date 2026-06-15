/**
 * godot-otel site observability
 *
 * Wide events per O'Reilly "Learning OpenTelemetry" (Young & Parker):
 * one rich span/log record per user action rather than many narrow events.
 *
 * In dev mode all signals go to the browser console via ConsoleSpanExporter
 * and ConsoleLogRecordExporter. Swap exporters here to send to a collector.
 */

import { WebTracerProvider } from '@opentelemetry/sdk-trace-web';
import { ConsoleSpanExporter, SimpleSpanProcessor } from '@opentelemetry/sdk-trace-base';
import { Resource } from '@opentelemetry/resources';
import { ATTR_SERVICE_NAME, ATTR_SERVICE_VERSION } from '@opentelemetry/semantic-conventions';
import { LoggerProvider, ConsoleLogRecordExporter, SimpleLogRecordProcessor } from '@opentelemetry/sdk-logs';
import { trace, context } from '@opentelemetry/api';
import { registerInstrumentations } from '@opentelemetry/instrumentation';
import { FetchInstrumentation } from '@opentelemetry/instrumentation-fetch';
import { DocumentLoadInstrumentation } from '@opentelemetry/instrumentation-document-load';

// ── Resource ─────────────────────────────────────────────────────────────────

const resource = new Resource({
    [ATTR_SERVICE_NAME]: 'godot-otel-site',
    [ATTR_SERVICE_VERSION]: '0.1.0',
    'site.env': window.location.hostname === 'localhost' ? 'dev' : 'prod',
    'site.path': window.location.pathname,
});

// ── Trace provider ────────────────────────────────────────────────────────────

const traceProvider = new WebTracerProvider({ resource });
traceProvider.addSpanProcessor(new SimpleSpanProcessor(new ConsoleSpanExporter()));
traceProvider.register();

// ── Log provider ──────────────────────────────────────────────────────────────

const logProvider = new LoggerProvider({ resource });
logProvider.addLogRecordProcessor(
    new SimpleLogRecordProcessor(new ConsoleLogRecordExporter())
);
const siteLogger = logProvider.getLogger('godot-otel-site');

// ── Auto-instrumentations ─────────────────────────────────────────────────────

registerInstrumentations({
    instrumentations: [
        new DocumentLoadInstrumentation(),
        new FetchInstrumentation({ propagateTraceHeaderCorsUrls: [] }),
    ],
});

// ── Tracer ────────────────────────────────────────────────────────────────────

const tracer = trace.getTracer('godot-otel-site', '0.1.0');

// ── Session metadata ──────────────────────────────────────────────────────────

function getSessionId() {
    let id = sessionStorage.getItem('otel.session.id');
    if (!id) {
        id = crypto.randomUUID();
        sessionStorage.setItem('otel.session.id', id);
    }
    return id;
}

const SESSION_ID = getSessionId();
const PAGE_LOAD_TS = Date.now();

// ── Wide event helpers ────────────────────────────────────────────────────────

function baseAttrs() {
    return {
        'session.id': SESSION_ID,
        'page.path': window.location.pathname,
        'page.title': document.title,
        'page.referrer': document.referrer || 'direct',
        'viewport.width': window.innerWidth,
        'viewport.height': window.innerHeight,
        'user.locale': navigator.language,
    };
}

function emitLog(severity, message, attrs = {}) {
    siteLogger.emit({
        severityNumber: severity,
        body: message,
        attributes: { ...baseAttrs(), ...attrs },
        timestamp: Date.now(),
    });
}

// ── Page view wide event ──────────────────────────────────────────────────────

const pageViewSpan = tracer.startSpan('page.view', {
    attributes: {
        ...baseAttrs(),
        'page.section': document.body.dataset.section || 'unknown',
    },
});
const pageViewCtx = trace.setSpan(context.active(), pageViewSpan);

emitLog(9 /* INFO */, 'page.view', {
    ...baseAttrs(),
    'page.section': document.body.dataset.section || 'unknown',
});

window.addEventListener('beforeunload', () => {
    pageViewSpan.setAttribute('page.time_on_page_ms', Date.now() - PAGE_LOAD_TS);
    pageViewSpan.end();
});

// ── CTA click tracking ────────────────────────────────────────────────────────

document.addEventListener('click', (e) => {
    const link = e.target.closest('a[data-cta]');
    if (!link) return;

    const ctaId = link.dataset.cta;
    const href = link.href;
    const text = link.innerText.trim();

    const span = tracer.startSpan('cta.click', {
        attributes: {
            ...baseAttrs(),
            'cta.id': ctaId,
            'cta.text': text,
            'cta.href': href,
            'cta.target': link.target || '_self',
        },
    }, pageViewCtx);
    span.end();

    emitLog(9 /* INFO */, 'cta.click', {
        'cta.id': ctaId,
        'cta.text': text,
        'cta.href': href,
    });
});

// ── Docs navigation tracking ──────────────────────────────────────────────────

const isDocsPage = window.location.pathname.includes('/docs');

if (isDocsPage) {
    document.querySelectorAll('.hextra-toc a, nav a').forEach((navLink) => {
        navLink.addEventListener('click', (e) => {
            const span = tracer.startSpan('docs.navigate', {
                attributes: {
                    ...baseAttrs(),
                    'nav.from': window.location.pathname,
                    'nav.to': navLink.href,
                    'nav.text': navLink.innerText.trim(),
                },
            }, pageViewCtx);
            span.end();

            emitLog(9 /* INFO */, 'docs.navigate', {
                'nav.from': window.location.pathname,
                'nav.to': navLink.href,
                'nav.text': navLink.innerText.trim(),
            });
        });
    });

    // Code copy events
    document.addEventListener('click', (evt) => {
        if (!evt.target.closest('.copy-button, button[data-copy]')) return;
        const codeBlock = evt.target.closest('pre');
        const lang = codeBlock?.dataset.lang || 'unknown';

        const span = tracer.startSpan('docs.code.copy', {
            attributes: {
                ...baseAttrs(),
                'code.language': lang,
                'code.block_id': codeBlock?.id || '',
            },
        }, pageViewCtx);
        span.end();

        emitLog(9 /* INFO */, 'docs.code.copy', { 'code.language': lang });
    });
}

// ── Scroll depth tracking ─────────────────────────────────────────────────────

const SCROLL_THRESHOLDS = [25, 50, 75, 100];
const reportedDepths = new Set();

window.addEventListener('scroll', () => {
    const scrolled = Math.round(
        (window.scrollY / (document.body.scrollHeight - window.innerHeight)) * 100
    );
    for (const threshold of SCROLL_THRESHOLDS) {
        if (scrolled >= threshold && !reportedDepths.has(threshold)) {
            reportedDepths.add(threshold);
            pageViewSpan.addEvent('scroll.depth', {
                'scroll.depth_pct': threshold,
                'scroll.time_ms': Date.now() - PAGE_LOAD_TS,
            });
        }
    }
}, { passive: true });
