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

export type BusStopCodeRespBody = {
  busStopCode: string | null
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

export class Api {
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
    const resp = await fetch('/api/config/weather-town-list').then((x) => x.json())
    return resp as WeatherTownListRespBody
  }

  static async getWeatherRegionList() {
    const resp = await fetch('/api/config/weather-region-list').then((x) => x.json())
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
    const resp = await fetch('/api/test/weather', { method: 'POST' }).then((x) => x.json())
    return resp as WeatherRespBody
  }
}
