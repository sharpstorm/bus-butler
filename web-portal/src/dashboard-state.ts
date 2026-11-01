import {
  Api,
  BusArrival,
  BusStop,
  DisplayConfig,
  MrtRespBody,
  SourceStatus,
  StatusRespBody,
  WeatherRespBody,
} from './api'

type Section =
  | 'overview'
  | 'stops'
  | 'display'
  | 'weather'
  | 'connection'
  | 'diagnostics'

const SECTIONS: Section[] = [
  'overview',
  'stops',
  'display',
  'weather',
  'connection',
  'diagnostics',
]
const STATUS_POLL_MS = 10_000
const LOOKUP_POLL_MS = 1_500
const LOOKUP_TIMEOUT_MS = 45_000

// Official MRT line colours
const LINE_COLORS: Record<string, string> = {
  NSL: '#d42e12',
  EWL: '#009645',
  CGL: '#009645',
  NEL: '#9900aa',
  CCL: '#fa9e0d',
  CEL: '#fa9e0d',
  DTL: '#005ec4',
  TEL: '#9d5b25',
  BPL: '#748477',
  SLRT: '#748477',
  PLRT: '#748477',
}

const $ = <T extends HTMLElement = HTMLElement>(id: string) =>
  document.getElementById(id) as T

function el<K extends keyof HTMLElementTagNameMap>(
  tag: K,
  props: Partial<HTMLElementTagNameMap[K]> & { className?: string } = {},
  ...children: (Node | string | null | undefined)[]
): HTMLElementTagNameMap[K] {
  const node = document.createElement(tag)
  Object.assign(node, props)
  for (const child of children) {
    if (child === null || child === undefined) continue
    node.append(child)
  }
  return node
}

const ICONS = {
  up: '<path d="M6 15l6-6 6 6"/>',
  down: '<path d="M6 9l6 6 6-6"/>',
  remove: '<path d="M6 6l12 12M18 6L6 18"/>',
  search: '<circle cx="11" cy="11" r="6"/><path d="M20 20l-4.5-4.5"/>',
}

function iconButton(
  icon: keyof typeof ICONS,
  label: string,
  onClick: () => void,
  className = '',
) {
  const btn = el('button', {
    type: 'button',
    className: `icon-btn ${className}`.trim(),
    title: label,
  })
  btn.setAttribute('aria-label', label)
  btn.innerHTML = `<svg viewBox="0 0 24 24" aria-hidden="true">${ICONS[icon]}</svg>`
  btn.addEventListener('click', onClick)
  return btn
}

function agoText(seconds: number | null | undefined) {
  if (seconds === null || seconds === undefined) return 'never'
  if (seconds < 60) return `${seconds}s ago`
  if (seconds < 3600) return `${Math.floor(seconds / 60)}m ago`
  if (seconds < 86400) return `${Math.floor(seconds / 3600)}h ago`
  return `${Math.floor(seconds / 86400)}d ago`
}

function durationText(seconds: number) {
  const d = Math.floor(seconds / 86400)
  const h = Math.floor((seconds % 86400) / 3600)
  const m = Math.floor((seconds % 3600) / 60)
  if (d > 0) return `${d}d ${h}h`
  if (h > 0) return `${h}h ${m}m`
  return `${m}m`
}

function bytesText(bytes: number) {
  if (bytes >= 1024 * 1024) return `${(bytes / 1048576).toFixed(1)} MB`
  return `${Math.round(bytes / 1024)} KB`
}

function signalText(rssi: number) {
  if (rssi >= -60) return 'Strong'
  if (rssi >= -70) return 'Good'
  if (rssi >= -80) return 'Fair'
  return 'Weak'
}

function setStatus(
  target: HTMLElement,
  message: string,
  kind: 'success' | 'error' | 'info' | '' = '',
) {
  target.textContent = message
  target.className = `status-message ${kind}`.trim()
}

async function withBusy<T>(
  button: HTMLButtonElement,
  busyLabel: string,
  work: () => Promise<T>,
): Promise<T> {
  const label = button.textContent
  button.disabled = true
  button.textContent = busyLabel
  try {
    return await work()
  } finally {
    button.disabled = false
    button.textContent = label
  }
}

export class DashboardState {
  private current: Section = 'overview'
  private pollTimer: number | undefined
  private initialized = false

  private stops: BusStop[] = []
  private activeStop = 0
  private maxStops = 8
  private stopsDirty = false

  private serviceOrder: string[] = []
  private draggedIndex = -1

  private display: DisplayConfig | null = null
  private brightnessTimer: number | undefined

  constructor() {
    document.querySelectorAll<HTMLButtonElement>('.nav-item').forEach((btn) => {
      btn.addEventListener('click', () =>
        this.showSection(btn.dataset.section as Section),
      )
    })
    window.addEventListener('hashchange', () => this.showFromHash())
    document.addEventListener('visibilitychange', () => {
      if (!document.hidden && this.current === 'overview') this.refreshOverview()
    })

    this.bindOverview()
    this.bindStops()
    this.bindServiceOrder()
    this.bindDisplay()
    this.bindWeather()
    this.bindConnection()
    this.bindDiagnostics()
  }

  // Called once the dashboard becomes visible
  async initSettings() {
    if (this.initialized) return
    this.initialized = true
    this.showFromHash()
    this.refreshOverview()
    await Promise.allSettled([
      this.loadStops(),
      this.loadServiceOrder(),
      this.loadDisplay(),
      this.loadWeather(),
      this.loadConnection(),
    ])
  }

  // ---------------------------------------------------------------------------
  // Navigation

  private showFromHash() {
    const hash = window.location.hash.replace('#', '') as Section
    this.showSection(SECTIONS.includes(hash) ? hash : 'overview', false)
  }

  private showSection(section: Section, updateHash = true) {
    this.current = section
    document.querySelectorAll<HTMLElement>('.nav-item').forEach((btn) => {
      const active = btn.dataset.section === section
      btn.classList.toggle('active', active)
      if (active) btn.setAttribute('aria-current', 'page')
      else btn.removeAttribute('aria-current')
    })
    document.querySelectorAll<HTMLElement>('.dash-section').forEach((s) => {
      s.classList.toggle('active', s.dataset.section === section)
    })
    if (updateHash && window.location.hash !== `#${section}`) {
      history.replaceState(null, '', `#${section}`)
    }

    // Status feeds both the overview and the top bar pill, so keep polling
    // on every section.
    if (this.pollTimer === undefined) {
      this.pollTimer = window.setInterval(
        () => !document.hidden && this.refreshOverview(),
        STATUS_POLL_MS,
      )
    }
    if (section === 'overview') this.refreshOverview()
    if (section === 'diagnostics') this.renderTestStopOptions()
  }

  // ---------------------------------------------------------------------------
  // Overview

  private bindOverview() {
    const btn = $<HTMLButtonElement>('refresh-data-btn')
    btn.addEventListener('click', () =>
      withBusy(btn, 'Requested', async () => {
        await Api.refresh()
        await new Promise((r) => setTimeout(r, 4000))
        await this.refreshOverview()
      }).catch(() => undefined),
    )
  }

  private async refreshOverview() {
    const [status, mrt] = await Promise.allSettled([
      Api.getStatus(),
      Api.getMrt(),
    ])
    if (status.status === 'fulfilled') {
      this.renderStatus(status.value)
    } else {
      this.setPill('error', 'Device unreachable')
    }
    if (mrt.status === 'fulfilled') this.renderMrt(mrt.value)
  }

  private setPill(state: 'ok' | 'warn' | 'error' | 'pending', text: string) {
    const pill = $('device-pill')
    pill.dataset.state = state
    $('device-pill-text').textContent = text
  }

  private renderStatus(s: StatusRespBody) {
    $('device-sub').textContent = `${s.wifi.ip || 'busbutler.local'} · v${s.firmware}`
    const failing = Object.values(s.sources).filter(
      (src) => src.attempted && !src.ok,
    ).length
    if (failing > 0) this.setPill('warn', `${failing} data source${failing > 1 ? 's' : ''} failing`)
    else this.setPill('ok', 'Online')

    const screen = s.display.idleDimmed
      ? 'Idle-dimmed'
      : s.display.nightActive
        ? 'Night dimming'
        : 'Normal'
    const stats: [string, string, string][] = [
      ['WiFi', s.wifi.ssid || '--', `${signalText(s.wifi.rssi)} · ${s.wifi.rssi} dBm`],
      ['Address', s.wifi.ip || '--', 'busbutler.local'],
      ['Uptime', durationText(s.uptime), s.clockSynced ? 'Clock synced' : 'Clock not synced'],
      ['Screen', `${s.display.level}%`, screen],
      ['Memory', bytesText(s.freeHeap), `${bytesText(s.freePsram)} PSRAM free`],
      ['Active stop', s.activeStop || '--', this.stopName(s.activeStop)],
    ]
    $('device-stats').replaceChildren(
      ...stats.map(([label, value, sub]) =>
        el(
          'div',
          { className: 'stat' },
          el('div', { className: 'stat-label', textContent: label }),
          el('div', { className: 'stat-value', textContent: value, title: value }),
          el('div', { className: 'stat-sub', textContent: sub }),
        ),
      ),
    )

    const sources: [string, SourceStatus][] = [
      ['Bus arrivals', s.sources.bus],
      ['MRT alerts', s.sources.mrt],
      ['Forecasts', s.sources.forecast],
      ['Weather readings', s.sources.readings],
    ]
    $('source-list').replaceChildren(
      ...sources.map(([name, src]) => {
        let kind = ''
        let when = 'Waiting'
        if (src.attempted) {
          kind = src.ok ? 'ok' : 'error'
          when = src.ok
            ? agoText(src.lastSuccessAgo)
            : src.error === -11
              ? 'No API key'
              : src.error === -10
                ? 'No stop set'
                : `Failing (last ok ${agoText(src.lastSuccessAgo)})`
        }
        return el(
          'li',
          {},
          el('span', { className: `dot ${kind}` }),
          name,
          el('span', { className: 'when', textContent: when }),
        )
      }),
    )
  }

  private renderMrt(m: MrtRespBody) {
    const box = $('mrt-summary')
    if (m.status === null) {
      box.replaceChildren(
        el('p', {
          className: 'muted',
          textContent:
            m.source.error === -11
              ? 'Add an LTA API key to see MRT alerts.'
              : 'Waiting for the first check.',
        }),
      )
      return
    }
    if (m.status === 'normal') {
      box.replaceChildren(
        el('span', { className: 'chip ok', textContent: 'All lines normal' }),
        el('p', {
          className: 'hint',
          textContent: `Checked ${agoText(m.source.lastSuccessAgo)}`,
        }),
      )
      return
    }
    const nodes: Node[] = [
      el('span', { className: 'chip warn', textContent: 'Disruption' }),
    ]
    for (const seg of m.segments ?? []) {
      const bar = el('span', { className: 'mrt-bar' })
      bar.style.background = LINE_COLORS[seg.line] ?? 'var(--color-text-tertiary)'
      const stations = seg.stations.split(',').map((x) => x.trim()).filter(Boolean)
      const range =
        stations.length > 1 ? `${stations[0]} – ${stations[stations.length - 1]}` : stations[0] ?? ''
      nodes.push(
        el(
          'div',
          { className: 'mrt-line' },
          bar,
          el(
            'div',
            {},
            el('strong', { textContent: seg.lineName }),
            el('div', {
              className: 'muted',
              textContent: [range, seg.direction && seg.direction !== 'Both' ? `towards ${seg.direction}` : '']
                .filter(Boolean)
                .join(', '),
            }),
          ),
        ),
      )
    }
    for (const msg of m.messages ?? []) {
      nodes.push(el('p', { className: 'mrt-msg', textContent: msg }))
    }
    box.replaceChildren(...nodes)
  }

  // ---------------------------------------------------------------------------
  // Bus stops

  private stopName(code: string) {
    return this.stops.find((s) => s.code === code)?.name ?? ''
  }

  private bindStops() {
    $('add-stop-form').addEventListener('submit', (e) => {
      e.preventDefault()
      this.addStop()
    })
    $('save-stops-btn').addEventListener('click', () => this.saveStops())
    window.addEventListener('beforeunload', (e) => {
      if (this.stopsDirty) e.preventDefault()
    })
  }

  private async loadStops() {
    try {
      const resp = await Api.getStops()
      this.stops = resp.stops
      this.activeStop = resp.active
      this.maxStops = resp.max
    } catch {
      // Older firmware: fall back to the single stop
      const legacy = await Api.getBusStopCode().catch(() => ({ busStopCode: null }))
      this.stops = legacy.busStopCode ? [{ code: legacy.busStopCode, name: '' }] : []
      this.activeStop = 0
    }
    this.stopsDirty = false
    this.renderStops()
    this.renderTestStopOptions()
  }

  private markStopsDirty() {
    this.stopsDirty = true
    setStatus($('stops-status'), 'Unsaved changes', 'info')
  }

  private renderStops() {
    const list = $('stop-list')
    $('stops-count').textContent = `${this.stops.length} of ${this.maxStops}`
    $<HTMLInputElement>('add-stop-input').disabled = this.stops.length >= this.maxStops

    if (this.stops.length === 0) {
      list.replaceChildren(
        el('li', { className: 'empty', textContent: 'No stops yet. Add one below.' }),
      )
      return
    }

    list.replaceChildren(
      ...this.stops.map((stop, i) => {
        const radio = el('input', {
          type: 'radio',
          name: 'active-stop',
          checked: i === this.activeStop,
        })
        radio.addEventListener('change', () => {
          this.activeStop = i
          this.markStopsDirty()
          this.renderStops()
        })

        const name = el('input', {
          className: 'input stop-name',
          value: stop.name,
          placeholder: 'Looking up name...',
          maxLength: 40,
        })
        name.setAttribute('aria-label', `Name for stop ${stop.code}`)
        name.addEventListener('input', () => {
          stop.name = name.value
          this.markStopsDirty()
        })

        const label = el('label', { className: 'stop-radio', title: 'Show this stop first' },
          radio,
          el('span', { className: 'stop-code', textContent: stop.code }),
        )

        const actions = el(
          'div',
          { className: 'stop-actions' },
          iconButton('search', 'Look up name from LTA', () => this.lookupName(stop, true)),
          iconButton('up', 'Move up', () => this.moveStop(i, -1)),
          iconButton('down', 'Move down', () => this.moveStop(i, 1)),
          iconButton('remove', 'Remove stop', () => this.removeStop(i), 'danger'),
        )
        ;(actions.children[1] as HTMLButtonElement).disabled = i === 0
        ;(actions.children[2] as HTMLButtonElement).disabled = i === this.stops.length - 1

        return el('li', { className: `stop-item ${i === this.activeStop ? 'active' : ''}` },
          label, name, actions)
      }),
    )
  }

  private addStop() {
    const input = $<HTMLInputElement>('add-stop-input')
    const status = $('stops-status')
    const code = input.value.trim()
    if (!/^\d{5}$/.test(code)) {
      setStatus(status, 'Bus stop codes are 5 digits, e.g. 17179', 'error')
      return
    }
    if (this.stops.some((s) => s.code === code)) {
      setStatus(status, `Stop ${code} is already in the list`, 'error')
      return
    }
    const stop = { code, name: '' }
    this.stops.push(stop)
    input.value = ''
    this.markStopsDirty()
    this.renderStops()
    this.lookupName(stop, false)
  }

  private moveStop(index: number, delta: number) {
    const target = index + delta
    if (target < 0 || target >= this.stops.length) return
    const [stop] = this.stops.splice(index, 1)
    this.stops.splice(target, 0, stop)
    if (this.activeStop === index) this.activeStop = target
    else if (this.activeStop === target) this.activeStop = index
    this.markStopsDirty()
    this.renderStops()
  }

  private removeStop(index: number) {
    this.stops.splice(index, 1)
    if (this.activeStop >= this.stops.length) this.activeStop = 0
    else if (index < this.activeStop) this.activeStop--
    this.markStopsDirty()
    this.renderStops()
  }

  // Ask the device to look the name up in LTA's stop list (it pages through
  // ~5000 stops, so this can take a few seconds).
  private async lookupName(stop: BusStop, overwrite: boolean) {
    const status = $('stops-status')
    setStatus(status, `Looking up ${stop.code}...`, 'info')
    try {
      await Api.startStopLookup(stop.code)
      const started = Date.now()
      while (Date.now() - started < LOOKUP_TIMEOUT_MS) {
        await new Promise((r) => setTimeout(r, LOOKUP_POLL_MS))
        const res = await Api.getStopLookup(stop.code)
        if (res.status === 'pending') continue
        if (res.status === 'found' && res.name) {
          if (overwrite || !stop.name) stop.name = res.name
          this.markStopsDirty()
          this.renderStops()
          setStatus(status, `Found: ${res.name}. Save to keep it.`, 'success')
        } else if (res.status === 'not_found') {
          setStatus(status, `Stop ${stop.code} isn't in LTA's list. Check the code.`, 'error')
        } else {
          setStatus(status, 'Lookup failed. Check the API key, or type a name.', 'error')
        }
        return
      }
      setStatus(status, 'Lookup timed out. You can type a name instead.', 'error')
    } catch (err: any) {
      setStatus(status, err.message, 'error')
    }
  }

  private async saveStops() {
    const btn = $<HTMLButtonElement>('save-stops-btn')
    const status = $('stops-status')
    await withBusy(btn, 'Saving...', async () => {
      try {
        await Api.saveStops(
          this.stops.map((s) => ({ code: s.code, name: s.name.trim() })),
          this.activeStop,
        )
        this.stopsDirty = false
        setStatus(status, 'Saved. The device updates within a few seconds.', 'success')
        this.renderTestStopOptions()
      } catch (err: any) {
        setStatus(status, err.message, 'error')
      }
    })
  }

  // ---------------------------------------------------------------------------
  // Service order

  private bindServiceOrder() {
    $('fetch-current-services-btn').addEventListener('click', () =>
      this.fetchCurrentServices(),
    )
    $('add-service-btn').addEventListener('click', () => this.addServiceManually())
    $('manual-service-input').addEventListener('keydown', (e) => {
      if ((e as KeyboardEvent).key === 'Enter') {
        e.preventDefault()
        this.addServiceManually()
      }
    })
    $('save-service-order-btn').addEventListener('click', () => this.saveServiceOrder())
  }

  private async loadServiceOrder() {
    try {
      this.serviceOrder = (await Api.getBusServiceOrder()).order
    } catch {
      this.serviceOrder = []
    }
    this.renderServiceList()
  }

  private async fetchCurrentServices() {
    const btn = $<HTMLButtonElement>('fetch-current-services-btn')
    const status = $('service-order-status')
    const stop = this.stops[this.activeStop]
    if (!stop) {
      setStatus(status, 'Add a bus stop first', 'error')
      return
    }
    await withBusy(btn, 'Fetching...', async () => {
      try {
        const data = await Api.testBusArrival(stop.code)
        const added = data.services
          .map((s) => s.service)
          .filter((svc) => !this.serviceOrder.includes(svc))
        this.serviceOrder.push(...added)
        this.renderServiceList()
        setStatus(
          status,
          added.length ? `Added ${added.length} service(s). Save to keep.` : 'No new services at this stop',
          added.length ? 'info' : '',
        )
      } catch (err: any) {
        setStatus(status, err.message, 'error')
      }
    })
  }

  private addServiceManually() {
    const input = $<HTMLInputElement>('manual-service-input')
    const status = $('service-order-status')
    const svc = input.value.trim()
    if (!/^[0-9A-Za-z]{1,4}$/.test(svc)) {
      setStatus(status, 'Services are up to 4 letters or digits, e.g. 174e', 'error')
      return
    }
    if (this.serviceOrder.includes(svc)) {
      setStatus(status, `${svc} is already in the list`, 'error')
      return
    }
    this.serviceOrder.push(svc)
    input.value = ''
    this.renderServiceList()
    setStatus(status, 'Unsaved changes', 'info')
  }

  private renderServiceList() {
    const list = $('service-list')
    if (this.serviceOrder.length === 0) {
      list.replaceChildren(
        el('div', {
          className: 'empty',
          textContent: 'No order set: services show in LTA order.',
        }),
      )
      return
    }
    list.replaceChildren(
      ...this.serviceOrder.map((svc, i) => {
        const item = el(
          'div',
          { className: 'service-item', draggable: true },
          el('span', { className: 'service-index', textContent: String(i + 1) }),
          el('span', { className: 'service-number', textContent: svc }),
          iconButton('remove', `Remove ${svc}`, () => {
            this.serviceOrder.splice(i, 1)
            this.renderServiceList()
            setStatus($('service-order-status'), 'Unsaved changes', 'info')
          }, 'danger'),
        )
        item.addEventListener('dragstart', (e) => {
          this.draggedIndex = i
          item.classList.add('dragging')
          e.dataTransfer?.setData('text/plain', svc)
        })
        item.addEventListener('dragend', () => {
          item.classList.remove('dragging')
          list.querySelectorAll('.drag-over').forEach((n) => n.classList.remove('drag-over'))
        })
        item.addEventListener('dragover', (e) => {
          e.preventDefault()
          item.classList.add('drag-over')
        })
        item.addEventListener('dragleave', () => item.classList.remove('drag-over'))
        item.addEventListener('drop', (e) => {
          e.preventDefault()
          if (this.draggedIndex < 0 || this.draggedIndex === i) return
          const [moved] = this.serviceOrder.splice(this.draggedIndex, 1)
          this.serviceOrder.splice(i, 0, moved)
          this.draggedIndex = -1
          this.renderServiceList()
          setStatus($('service-order-status'), 'Unsaved changes', 'info')
        })
        return item
      }),
    )
  }

  private async saveServiceOrder() {
    const btn = $<HTMLButtonElement>('save-service-order-btn')
    const status = $('service-order-status')
    await withBusy(btn, 'Saving...', async () => {
      try {
        await Api.saveBusServiceOrder(this.serviceOrder)
        setStatus(status, 'Saved', 'success')
      } catch (err: any) {
        setStatus(status, err.message, 'error')
      }
    })
  }

  // ---------------------------------------------------------------------------
  // Display

  private bindDisplay() {
    const brightness = $<HTMLInputElement>('brightness-input')
    brightness.addEventListener('input', () => {
      $('brightness-value').textContent = `${brightness.value}%`
      // Apply live, but don't flood the device with requests
      window.clearTimeout(this.brightnessTimer)
      this.brightnessTimer = window.setTimeout(() => {
        Api.saveDisplay({ brightness: Number(brightness.value) }).catch(() => undefined)
      }, 250)
    })

    for (const id of ['idle-level', 'night-level']) {
      const range = $<HTMLInputElement>(id)
      range.addEventListener('input', () => {
        $(`${id}-value`).textContent = `${range.value}%`
      })
    }
    for (const id of ['idle-enabled', 'night-enabled', 'night-theme']) {
      $(id).addEventListener('change', () => this.updateDependents())
    }

    $('display-form').addEventListener('submit', (e) => {
      e.preventDefault()
      this.saveDisplay()
    })
  }

  private updateDependents() {
    document.querySelectorAll<HTMLElement>('[data-depends]').forEach((grid) => {
      const on = $<HTMLInputElement>(grid.dataset.depends!).checked
      grid.classList.toggle('is-off', !on)
    })
    // The time window matters if either night feature is on
    const nightOn =
      $<HTMLInputElement>('night-enabled').checked ||
      $<HTMLInputElement>('night-theme').checked
    $('night-window').classList.toggle('is-off', !nightOn)
  }

  private async loadDisplay() {
    try {
      this.display = await Api.getDisplay()
    } catch {
      return
    }
    const d = this.display
    $<HTMLInputElement>('brightness-input').value = String(d.brightness)
    $('brightness-value').textContent = `${d.brightness}%`
    $<HTMLInputElement>('idle-enabled').checked = d.idleDimEnabled
    $<HTMLInputElement>('idle-minutes').value = String(d.idleDimMinutes)
    $<HTMLInputElement>('idle-level').value = String(d.idleDimLevel)
    $('idle-level-value').textContent = `${d.idleDimLevel}%`
    $<HTMLInputElement>('night-enabled').checked = d.nightEnabled
    $<HTMLInputElement>('night-theme').checked = d.nightTheme ?? false
    $<HTMLInputElement>('night-start').value = d.nightStart
    $<HTMLInputElement>('night-end').value = d.nightEnd
    $<HTMLInputElement>('night-level').value = String(d.nightLevel)
    $('night-level-value').textContent = `${d.nightLevel}%`
    this.updateDependents()
  }

  private async saveDisplay() {
    const status = $('display-status')
    const minutes = Number($<HTMLInputElement>('idle-minutes').value)
    if (!Number.isInteger(minutes) || minutes < 1 || minutes > 120) {
      setStatus(status, 'Dim after must be 1 to 120 minutes', 'error')
      return
    }
    const start = $<HTMLInputElement>('night-start').value
    const end = $<HTMLInputElement>('night-end').value
    if (!start || !end) {
      setStatus(status, 'Set both night times', 'error')
      return
    }
    if (start === end) {
      setStatus(status, 'Night start and end must be different', 'error')
      return
    }
    try {
      await Api.saveDisplay({
        idleDimEnabled: $<HTMLInputElement>('idle-enabled').checked,
        idleDimMinutes: minutes,
        idleDimLevel: Number($<HTMLInputElement>('idle-level').value),
        nightEnabled: $<HTMLInputElement>('night-enabled').checked,
        nightTheme: $<HTMLInputElement>('night-theme').checked,
        nightStart: start,
        nightEnd: end,
        nightLevel: Number($<HTMLInputElement>('night-level').value),
      })
      setStatus(status, 'Saved', 'success')
    } catch (err: any) {
      setStatus(status, err.message, 'error')
    }
  }

  // ---------------------------------------------------------------------------
  // Weather

  private bindWeather() {
    $('weather-form').addEventListener('submit', (e) => {
      e.preventDefault()
      this.saveWeather()
    })
  }

  private async loadWeather() {
    const select = $<HTMLSelectElement>('weather-region-select')
    const [regions, region, town] = await Promise.allSettled([
      Api.getWeatherRegionList(),
      Api.getWeatherRegion(),
      Api.getWeatherTown(),
    ])
    const list =
      regions.status === 'fulfilled'
        ? regions.value.region
        : ['central', 'north', 'south', 'east', 'west']
    select.replaceChildren(
      ...list.map((r) =>
        el('option', { value: r, textContent: r.charAt(0).toUpperCase() + r.slice(1) }),
      ),
    )
    if (region.status === 'fulfilled' && region.value.region) {
      select.value = region.value.region
    }
    if (town.status === 'fulfilled' && town.value.town) {
      $<HTMLInputElement>('weather-town-input').value = town.value.town
    }

    // Town suggestions come from the live 2-hour forecast (slow-ish)
    Api.getWeatherTownList()
      .then((resp) => {
        $('weather-town-options').replaceChildren(
          ...resp.towns.map((t) => el('option', { value: t })),
        )
      })
      .catch(() => undefined)
  }

  private async saveWeather() {
    const status = $('weather-status')
    const town = $<HTMLInputElement>('weather-town-input').value.trim()
    const region = $<HTMLSelectElement>('weather-region-select').value
    const towns = Array.from(
      $('weather-town-options').querySelectorAll('option'),
    ).map((o) => (o as HTMLOptionElement).value)
    try {
      await Promise.all([Api.saveWeatherTown(town), Api.saveWeatherRegion(region)])
      if (towns.length && !towns.some((t) => t.toLowerCase() === town.toLowerCase())) {
        setStatus(status, `Saved, but "${town}" isn't an NEA forecast area. Pick one from the list.`, 'error')
      } else {
        setStatus(status, 'Saved', 'success')
      }
    } catch (err: any) {
      setStatus(status, err.message, 'error')
    }
  }

  // ---------------------------------------------------------------------------
  // Connection

  private bindConnection() {
    $('wifi-scan-btn').addEventListener('click', () => this.scanWifi())
    $('wifi-change').addEventListener('toggle', (e) => {
      if ((e.target as HTMLDetailsElement).open) this.scanWifi()
    })
    $('wifi-save-btn').addEventListener('click', () => this.saveWifi())
    $('lta-api-form').addEventListener('submit', (e) => {
      e.preventDefault()
      this.saveLtaApiKey()
    })
    this.bindNotify()
  }

  private async loadConnection() {
    const [wifi, key] = await Promise.allSettled([
      Api.getConfiguredWifiSsid(),
      Api.getSavedLtaApiKey(),
      this.loadNotify(),
    ])
    const wifiChip = $('current-wifi-status')
    if (wifi.status === 'fulfilled' && wifi.value.ssid) {
      wifiChip.textContent = wifi.value.ssid
      wifiChip.className = 'chip ok'
    } else {
      wifiChip.textContent = 'Not configured'
      wifiChip.className = 'chip warn'
    }
    const keyChip = $('lta-api-chip')
    const saved = key.status === 'fulfilled' && key.value.isApiKeySaved
    keyChip.textContent = saved ? 'Key saved' : 'No key'
    keyChip.className = saved ? 'chip ok' : 'chip error'
  }

  private async scanWifi() {
    const btn = $<HTMLButtonElement>('wifi-scan-btn')
    const select = $<HTMLSelectElement>('wifi-ssid-select')
    const status = $('wifi-status')
    await withBusy(btn, 'Scanning...', async () => {
      try {
        const resp = await Api.scanWifiNetworks()
        const seen = new Set<string>()
        const networks = resp.networks
          .filter((n) => n.ssid && !seen.has(n.ssid) && seen.add(n.ssid))
          .sort((a, b) => b.rssi - a.rssi)
        select.replaceChildren(
          ...networks.map((n) =>
            el('option', {
              value: n.ssid,
              textContent: `${n.ssid}  (${signalText(n.rssi)}${n.security === 'open' ? ', open' : ''})`,
            }),
          ),
        )
        setStatus(status, networks.length ? '' : 'No networks found', networks.length ? '' : 'error')
      } catch (err: any) {
        setStatus(status, err.message, 'error')
      }
    })
  }

  private async saveWifi() {
    const ssid = $<HTMLSelectElement>('wifi-ssid-select').value
    const passphrase = $<HTMLInputElement>('wifi-passphrase-input').value
    const status = $('wifi-status')
    if (!ssid) {
      setStatus(status, 'Pick a network first', 'error')
      return
    }
    if (
      !confirm(
        `Switch the device to "${ssid}" and restart it? If the password is wrong, the device starts its setup hotspot so you can fix it.`,
      )
    ) {
      return
    }
    try {
      await Api.saveWifiNetwork(ssid, passphrase)
      await Api.restart()
      setStatus(status, 'Restarting. Reconnect your phone to the same network, then reload this page.', 'success')
    } catch (err: any) {
      setStatus(status, err.message, 'error')
    }
  }

  private async saveLtaApiKey() {
    const input = $<HTMLInputElement>('lta-api-input')
    const status = $('lta-api-status')
    const apiKey = input.value.trim()
    if (!apiKey) {
      setStatus(status, 'Paste a key first', 'error')
      return
    }
    try {
      await Api.saveLtaApiKey(apiKey)
      input.value = ''
      setStatus(status, 'Key updated. The device uses it from the next request.', 'success')
      this.loadConnection()
    } catch (err: any) {
      setStatus(status, err.message, 'error')
    }
  }

  // ---------------------------------------------------------------------------
  // Notifications

  private bindNotify() {
    const status = $('notify-status')
    const input = $<HTMLInputElement>('notify-token-input')
    const save = async (token: string, message: string) => {
      try {
        await Api.saveNotifyToken(token)
        setStatus(status, message, 'success')
        await this.loadNotify()
      } catch (err: any) {
        setStatus(status, err.message, 'error')
      }
    }
    $('notify-save-btn').addEventListener('click', () => {
      const token = input.value.trim()
      if (!token) {
        setStatus(status, 'Paste a token first, or generate one', 'error')
        return
      }
      save(token, 'Token saved. Apps must send this one from now on.')
    })
    $('notify-off-btn').addEventListener('click', () => {
      if (!confirm('Turn notifications off? Apps will be refused until you set a token again.')) return
      input.value = ''
      save('', 'Notifications are off.')
    })
    const generate = $<HTMLButtonElement>('notify-generate-btn')
    generate.addEventListener('click', () =>
      withBusy(generate, 'Generating...', async () => {
        const { token } = await Api.generateNotifyToken()
        input.value = token
        input.select()
        setStatus(status, 'New token saved. Copy it into Sanctuary now: the device won\'t show it again.', 'success')
        await this.loadNotify()
      }).catch((err: any) => setStatus(status, err.message, 'error')),
    )
    document.querySelectorAll<HTMLButtonElement>('[data-notify-test]').forEach((btn) => {
      btn.addEventListener('click', () =>
        withBusy(btn, 'Sent', async () => {
          await Api.testNotify(btn.dataset.notifyTest || 'toast')
          await new Promise((r) => setTimeout(r, 1200))
        }).catch((err: any) => setStatus(status, err.message, 'error')),
      )
    })
  }

  private async loadNotify() {
    const chip = $('notify-chip')
    try {
      const cfg = await Api.getNotifyConfig()
      const active = cfg.full + cfg.banner + cfg.toast
      chip.textContent = cfg.tokenSet
        ? active ? `On · ${active} showing` : 'On'
        : 'Off'
      chip.className = cfg.tokenSet ? 'chip ok' : 'chip warn'
    } catch {
      chip.textContent = 'Unknown'
      chip.className = 'chip error'
    }
  }

  // ---------------------------------------------------------------------------
  // Diagnostics

  private bindDiagnostics() {
    $('fetch-bus-btn').addEventListener('click', () => this.fetchBusArrivals())
    $('fetch-weather-btn').addEventListener('click', () => this.fetchWeather())
    $('restart-btn').addEventListener('click', async () => {
      if (!confirm('Restart the device now?')) return
      await Api.restart().catch(() => undefined)
      this.setPill('pending', 'Restarting...')
    })
  }

  private renderTestStopOptions() {
    const select = $<HTMLSelectElement>('test-stop-select')
    const previous = select.value
    select.replaceChildren(
      ...this.stops.map((s) =>
        el('option', { value: s.code, textContent: s.name ? `${s.code} · ${s.name}` : s.code }),
      ),
    )
    if (previous) select.value = previous
    else if (this.stops[this.activeStop]) select.value = this.stops[this.activeStop].code
  }

  private async fetchBusArrivals() {
    const btn = $<HTMLButtonElement>('fetch-bus-btn')
    const results = $('bus-results')
    const code = $<HTMLSelectElement>('test-stop-select').value
    if (!code) {
      results.replaceChildren(el('div', { className: 'empty', textContent: 'Add a bus stop first.' }))
      return
    }
    await withBusy(btn, 'Fetching...', async () => {
      try {
        const data = await Api.testBusArrival(code)
        if (data.services.length === 0) {
          results.replaceChildren(el('div', { className: 'empty', textContent: 'No buses at this stop right now.' }))
          return
        }
        const loadClass = (load: string) =>
          /limited/i.test(load) ? 'load-LSD' : /standing/i.test(load) ? 'load-SDA' : 'load-SEA'
        const cell = (a?: BusArrival) => {
          if (!a) return el('td', { className: 'muted', textContent: '--' })
          const mins = Math.max(0, Math.floor(a.secondsLeft / 60))
          const td = el('td', {},
            el('span', {
              className: `mins ${loadClass(a.busLoad)}`,
              textContent: mins === 0 ? 'Arr' : `${mins} min`,
            }),
          )
          td.title = `${a.busType}, ${a.busLoad}${a.isWAB ? ', wheelchair accessible' : ''}`
          return td
        }
        const table = el('table', { className: 'arrival-table' },
          el('thead', {}, el('tr', {},
            el('th', { textContent: 'Service' }),
            el('th', { textContent: 'Next' }),
            el('th', { textContent: 'Then' }),
            el('th', { textContent: 'After' }),
            el('th', { textContent: 'Operator' }),
          )),
          el('tbody', {}, ...data.services.map((s) =>
            el('tr', {},
              el('td', { className: 'svc', textContent: s.service }),
              cell(s.nextBus1), cell(s.nextBus2), cell(s.nextBus3),
              el('td', { className: 'muted', textContent: s.company }),
            ),
          )),
        )
        results.replaceChildren(
          el('p', { className: 'hint', textContent: 'Colour shows crowding: green seats, yellow standing, red limited standing.' }),
          el('div', { className: 'table-wrap' }, table),
        )
      } catch (err: any) {
        results.replaceChildren(el('div', { className: 'status-message error', textContent: err.message }))
      }
    })
  }

  private async fetchWeather() {
    const btn = $<HTMLButtonElement>('fetch-weather-btn')
    const results = $('weather-results')
    await withBusy(btn, 'Fetching...', async () => {
      try {
        const data: WeatherRespBody = await Api.testWeather()
        const dl = el('dl', { className: 'kv' },
          el('dt', { textContent: 'Town' }), el('dd', { textContent: data.town || '--' }),
          el('dt', { textContent: 'Next 2 hours' }), el('dd', { textContent: data.twoHourForecast?.forecast || '--' }),
          el('dt', { textContent: 'Region' }), el('dd', { textContent: data.region || '--' }),
          ...(data.twentyFourHourForecast?.periods ?? []).flatMap((p) => [
            el('dt', { textContent: p.timePeriod }),
            el('dd', { textContent: p.forecast }),
          ]),
        )
        results.replaceChildren(dl)
      } catch (err: any) {
        results.replaceChildren(el('div', { className: 'status-message error', textContent: err.message }))
      }
    })
  }
}
