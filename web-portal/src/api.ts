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

export class Api {
  static async scanWifiNetworks() {
    const resp = await fetch('/api/wifi/scan').then((x) => x.json())
    return resp as ScanWifiNetworksRespBody
  }

  static async getConfiguredWifiSsid() {
    const resp = await fetch('/api/wifi/config').then((x) => x.json())
    return resp as GetWifiNetworkRespBody
  }
}
