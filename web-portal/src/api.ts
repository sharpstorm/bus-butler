export type WifiNetworkMetadata = {
  ssid: string
  rssi: number
  security: string
}

export type ScanWifiNetworksRespBody = {
  networks: WifiNetworkMetadata[]
}

export type GetWifiNetworkRespBody = {
  ssid: string | null
}

export type GetSavedLtaApiKeyBody = {
  isApiKeySaved: boolean
}

export type BusStopCodeRespBody = {
  busStopCode: string | null
}

export type BusServiceOrderRespBody = {
  order: string[]
}

export type WeatherTownRespBody = {
  town: string | null
}

export type WeatherRegionRespBody = {
  region: string | null
}

export type WeatherTownListRespBody = {
  towns: string[]
}

export type WeatherRegionListRespBody = {
  region: string[]
}

export type BusArrival = {
  arrival: number
  secondsLeft: number
  busType: string
  busLoad: string
  isWAB: boolean
}

export type BusService = {
  service: string
  company: string
  nextBus1?: BusArrival
  nextBus2?: BusArrival
  nextBus3?: BusArrival
}

export type BusArrivalRespBody = {
  busStop: string
  timestamp: number
  services: BusService[]
}

export type WeatherPeriod = {
  timePeriod: string
  forecast: string
}

export type WeatherRespBody = {
  town: string
  region: string
  twoHourForecast: {
    forecast: string
  }
  twentyFourHourForecast: {
    periods: WeatherPeriod[]
  }
}

export type BusStop = {
  code: string
  name: string
}

export type StopsRespBody = {
  stops: BusStop[]
  active: number
  max: number
}

export type StopLookupRespBody = {
  status: 'pending' | 'found' | 'not_found' | 'error' | 'unknown'
  name?: string
}

export type DisplayConfig = {
  brightness: number // percent
  idleDimEnabled: boolean
  idleDimMinutes: number
  idleDimLevel: number // percent
  nightEnabled: boolean
  nightTheme: boolean
  nightStart: string // HH:MM
  nightEnd: string // HH:MM
  nightLevel: number // percent
}

export type SourceStatus = {
  attempted: boolean
  ok: boolean
  error: number
  lastSuccessAgo: number | null
}

export type StatusRespBody = {
  firmware: string
  uptime: number
  clockSynced: boolean
  freeHeap: number
  freePsram: number
  wifi: { ssid: string; rssi: number; ip: string }
  display: { level: number; idleDimmed: boolean; nightActive: boolean }
  activeStop: string
  sources: {
    bus: SourceStatus
    mrt: SourceStatus
    forecast: SourceStatus
    readings: SourceStatus
  }
}

export type MrtSegment = {
  line: string
  lineName: string
  direction: string
  stations: string
  freeBus: string
  freeShuttle: string
}

export type MrtRespBody = {
  source: SourceStatus
  status: 'normal' | 'disrupted' | null
  segments?: MrtSegment[]
  messages?: string[]
}

async function request<T>(url: string, init?: RequestInit): Promise<T> {
  const resp = await fetch(url, init)
  let body: any = null
  try {
    body = await resp.json()
  } catch {
    // Some endpoints return an empty body on error
  }
  if (!resp.ok) {
    throw new Error(body?.error || `Request failed (${resp.status})`)
  }
  return body as T
}

const postJson = <T>(url: string, body?: unknown) =>
  request<T>(url, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: body === undefined ? undefined : JSON.stringify(body),
  })

export class Api {
  // Bus stops
  static getStops() {
    return request<StopsRespBody>('/api/config/stops')
  }

  static saveStops(stops: BusStop[], active: number) {
    return postJson<{ success: boolean }>('/api/config/stops', { stops, active })
  }

  static startStopLookup(code: string) {
    return postJson<StopLookupRespBody>('/api/stops/lookup', { code })
  }

  static getStopLookup(code: string) {
    return request<StopLookupRespBody>(
      `/api/stops/lookup?code=${encodeURIComponent(code)}`,
    )
  }

  // Display
  static getDisplay() {
    return request<DisplayConfig>('/api/config/display')
  }

  static saveDisplay(config: Partial<DisplayConfig>) {
    return postJson<{ success: boolean }>('/api/config/display', config)
  }

  // Device
  static getStatus() {
    return request<StatusRespBody>('/api/status')
  }

  static getMrt() {
    return request<MrtRespBody>('/api/mrt')
  }

  static refresh() {
    return postJson<{ success: boolean }>('/api/refresh')
  }

  static restart() {
    return postJson<{ success: boolean }>('/api/restart')
  }

  // WiFi APIs
  static async scanWifiNetworks() {
    const resp = await fetch('/api/wifi/scan').then((x) => x.json())
    return resp as ScanWifiNetworksRespBody
  }

  static async getConfiguredWifiSsid() {
    const resp = await fetch('/api/wifi/config').then((x) => x.json())
    return resp as GetWifiNetworkRespBody
  }

  static async saveWifiNetwork(ssid: string, passphrase: string) {
    const resp = await fetch('/api/wifi/config', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ ssid, passphrase }),
    }).then((x) => x.json())
    return resp
  }

  // LTA DataMall APIs
  static async getSavedLtaApiKey() {
    const resp = await fetch('/api/config/lta-api').then((x) => x.json())
    return resp as GetSavedLtaApiKeyBody
  }

  static async saveLtaApiKey(apiKey: string) {
    const resp = await fetch('/api/config/lta-api', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ apiKey }),
    }).then((x) => x.json())
    return resp
  }

  static async getBusStopCode() {
    const resp = await fetch('/api/config/busstop-code').then((x) => x.json())
    return resp as BusStopCodeRespBody
  }

  static async saveBusStopCode(busStopCode: string) {
    const resp = await fetch('/api/config/busstop-code', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ busStopCode }),
    }).then((x) => x.json())
    return resp
  }

  static async getBusServiceOrder() {
    const resp = await fetch('/api/config/bus-service-order').then((x) =>
      x.json(),
    )
    return resp as BusServiceOrderRespBody
  }

  static async saveBusServiceOrder(order: string[]) {
    const resp = await fetch('/api/config/bus-service-order', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ order }),
    }).then((x) => x.json())
    return resp
  }

  // Weather APIs
  static async getWeatherTown() {
    const resp = await fetch('/api/config/weather-town').then((x) => x.json())
    return resp as WeatherTownRespBody
  }

  static async saveWeatherTown(town: string) {
    const resp = await fetch('/api/config/weather-town', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ town }),
    }).then((x) => x.json())
    return resp
  }

  static async getWeatherRegion() {
    const resp = await fetch('/api/config/weather-region').then((x) => x.json())
    return resp as WeatherRegionRespBody
  }

  static async saveWeatherRegion(region: string) {
    const resp = await fetch('/api/config/weather-region', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ region }),
    }).then((x) => x.json())
    return resp
  }

  static async getWeatherTownList() {
    const resp = await fetch('/api/config/weather-town-list').then((x) =>
      x.json(),
    )
    return resp as WeatherTownListRespBody
  }

  static async getWeatherRegionList() {
    const resp = await fetch('/api/config/weather-region-list').then((x) =>
      x.json(),
    )
    return resp as WeatherRegionListRespBody
  }

  // Test APIs
  static async testBusArrival(busStopCode: string) {
    const resp = await fetch('/api/test/bus-arrival', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ busStopCode }),
    }).then((x) => x.json())
    return resp as BusArrivalRespBody
  }

  static async testWeather() {
    const resp = await fetch('/api/test/weather', { method: 'POST' }).then(
      (x) => x.json(),
    )
    return resp as WeatherRespBody
  }
}
